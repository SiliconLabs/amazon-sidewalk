# Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
#
# AMAZON PROPRIETARY/CONFIDENTIAL
#
# You may not use this file except in compliance with the terms and
# conditions set forth in the accompanying license.txt file.
#
# THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
# DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.

from pathlib import Path
import sys
sys.path.append(str(Path(Path(__file__).parent, "site-packages")))

import traceback
import argparse
import boto3
import json
import logging
import os
import re
import random
import string
import shutil
from enum import IntEnum
from libs.ProvisionWrapper import ProvisionWrapper, InputType, BoardType
from libs.EnvConfig import EnvConfig

logger = logging.getLogger()
logging.basicConfig(level=logging.INFO)

parser = argparse.ArgumentParser(add_help=True)
parser.add_argument('-i', '--instances', help="Number of instances to generate (default: 1)", required=False, default=1)
parser.add_argument('-in',  '--input',  help="Path of the input directory",  required=True)
parser.add_argument('-out', '--output', help="Path of the output directory", required=True)
parser.add_argument('-p', '--aws-profile', help="Name of your AWS profile from .aws/credentials", required=False)
parser.add_argument('-n', '--name', help="Specified name for the newly created device (default: sidewalk_[user]_device)", required=False)
parser.add_argument('-d', '--dst-name', help="Destination name used for uplink traffic routing (default: CFSDestination)", required=False, default="CFSDestination")
parser.add_argument('-t', '--target', help="Target part number for which the MFG page generation has to be done (default: all)", required=False, default="all")
parser.add_argument('-c', '--commander', help="Path to commander executable, not needed if already in system PATH", required=False)
parser.add_argument('-cfg', '--config', help="Configuration file, if provided other arguments are ignored", required=False)
args = parser.parse_args()

# size_nvm3_instance (0x6000) + size_empty_page_at_the_end (0x2000)
MFG_START_ADDR_OFFSET_FROM_END_OF_FLASH = 0x8000

try:

    os.chdir(os.path.dirname(os.path.abspath(__file__)))

    ENDPOINT = 'https://api.iotwireless.us-east-1.amazonaws.com'
    CONFIG_FILE_NAME = 'mfg.runconf'
    RESULT_FILE_NAME = 'mfg_result.json'
    LOCK_FILE_NAME   = '.lock'

    BASE_ADDRESS = {
        0x00000000: ['xg21'],
        0x08000000: ['xg24', 'xg28', 'xg23', 'bg24_1024']
    }

    class ResultLog:
        def __init__(self) -> None:
            self.type        = ''
            self.code        = 0
            self.message     = ''
            self.description = ''
            self.traceback   = ''

    class  ResultDevice:
        def __init__(self) -> None:
            self.deviceName = ''
            self.deviceID = ''
            self.dir      = ''
            self.smsn     = ''
            self.id       = ''
            self.destination = ''
            self.arn      = ''
            self.part     = ''
            self.family   = ''
            self.mfgRange    = ResultMfgRange()

    class ResultMfgRange:
        def __init__(self) -> None:
            self.startAddress = ''
            self.offset = '0x6000'

    class ResultGeneratedProfiles:
        def __init__(self) -> None:
            self.profileID = ''
            self.dir       = ''
            self.devices   = []

    class Result:
        def __init__(self) -> None:
            self.sessionID = ''
            self.generatorResult = 0
            self.generatorResultMessage = ''
            self.logs = []
            self.generatedProfiles = []

        def toJSON(self):
            return json.dumps(self, default=lambda o: o.__dict__, sort_keys=False, indent=4)

    class ResultCodes(IntEnum):
        OK                     = 0
        GENERIC_ERROR          = -1
        COMMANDER_NOT_FOUND    = -2
        AWS_GENERIC_ERROR      = -3
        AWS_ACCESS_DENIED      = -4      # boto3.IoTWireless.Client.exceptions.AccessDeniedException
        AWS_CONFLICT_ERROR     = -5      # boto3.IoTWireless.Client.exceptions.ConflictException
        AWS_THROTTLING         = -6      # boto3.IoTWireless.Client.exceptions.ThrottlingException
        AWS_INTERNAL_SERVER_ERROR = - 7  # boto3.IoTWireless.Client.exceptions.InternalServerException
        AWS_RESOURCE_NOT_FOUND = -8     # boto3.IoTWireless.Client.exceptions.ResourceNotFoundException
        AWS_VALIDATION_EXCEPTION  = - 9 # boto3.IoTWireless.Client.exceptions.ValidationException

    result_content = Result()

    def main():

        if(args.config != None):
            # This is when using the script with a configuration file
            print("Using configuration file for parameters")
            e = EnvConfig(args.config)
            aws = AWSHandler(None, None, None, e.aws_region, e.aws_profile).client
        elif(args.aws_profile != None):
            print("Using input arguments for parameters")
            # This is when using the script with command line arguments
            e = EnvConfig(None, args.instances, args.aws_profile, args.name, args.dst_name, args.target, args.commander)
            aws = AWSHandler(None, None, None, e.aws_region, e.aws_profile).client
        else:
            print("Using script through Simplicity Studio")
            # This is when using the script from within Simplicity Studio
            if getattr(sys, 'frozen', False):
                lock_file_path = os.path.dirname(sys.executable)
                lock_file_path = os.path.join(lock_file_path, LOCK_FILE_NAME)
            elif __file__:
                lock_file_path = os.path.dirname(__file__)
                lock_file_path = os.path.join(lock_file_path, LOCK_FILE_NAME)

            logger.info(lock_file_path)
            lock_file_handler = LockFileHandler(os.path.join(os.path.abspath(args.input)), lock_file_path)
            e = EnvConfig(os.path.join(os.path.abspath(args.input), CONFIG_FILE_NAME))

            if lock_file_handler.IsAlreadyTriggered(e.session_id):
                return
            else:
                lock_file_handler.UpdateLockFile(e.session_id)

            result_content.sessionID = e.session_id
            result_content.generatorResult = ResultCodes.OK

            if os.path.exists(e.commander_path) is False:
                result_content.generatorResult = ResultCodes.COMMANDER_NOT_FOUND

            if e.session_id is None:
                return

            aws = AWSHandler(e.aws_access_key, e.aws_secret_access_key, e.aws_session_token, e.aws_region ).client

        generate_prototype(e, aws)

    def generate_prototype(e, aws):
        is_context_studio = False
        for ix, request in enumerate(e.generation_requests):
            device_profile = ResultGeneratedProfiles()

            if not request.deviceProfileId:
                profile_name = 'prototype_' + ''.join(random.choice(string.ascii_lowercase) for i in range(10))
                logger.info(f"No DeviceProfileID specified. Creating a new DeviceProfile with random name {profile_name}")

                try:
                    response = aws.create_device_profile(Sidewalk={}, Name=profile_name)
                except Exception as e:
                    aws_exception_to_result(aws, e)
                    raise e

                print_response(response)
                print_status_code(response)
                request.deviceProfileId = response["Id"]
                logger.info(f"Profile created, {request.deviceProfileId }")
                e.generation_requests[ix] = request
                e.update_generation_request(request)

            logger.info(f"Getting a DeviceProfile by Id {request.deviceProfileId}")

            try:
                response = aws.get_device_profile(Id=request.deviceProfileId)
            except Exception as e:
                aws_exception_to_result(aws, e)
                raise e

            print_response(response)
            print_status_code(response)
            del response["ResponseMetadata"]

            logger.info("Saving device profile to file")
            make_dir(os.path.join(os.path.abspath(args.output), "mfg_output")) #starts here

            paths = PathsWrapper(request.deviceProfileId, os.path.abspath(args.output))
            paths.save_profile_json(response)
            logger.info(f"Saved DeviceProfile details to {paths.get_profile_json_filepath()}")

            device_profile.profileID = request.deviceProfileId
            device_profile.dir       = os.path.join( "autogen", "mfg_output", "DeviceProfile_" + request.deviceProfileId) #ends here

            for instanceNr in range(0, request.quantity):
                result_device = ResultDevice()

                logger.info(f"Creating a new WirelessDevice (instance nr {instanceNr})")

            # If name is not specified, use the default name :
                if request.deviceName is None or "" == request.deviceName:
                    request.deviceName = f"silabs_sidewalk_device"

                try:
                    response = aws.create_wireless_device(Type='Sidewalk',
                                                        Name=request.deviceName,
                                                        DestinationName=request.destinationName,
                                                        Sidewalk={"DeviceProfileId": request.deviceProfileId})
                except Exception as e:
                    aws_exception_to_result(aws, e)
                    raise e

                print_response(response)
                print_status_code(response)
                wireless_device_id = response["Id"]

                logger.info(f"Getting a WirelessDevice by Id {wireless_device_id}")

                try:
                    response = aws.get_wireless_device(Identifier=wireless_device_id, IdentifierType="WirelessDeviceId")
                except Exception as e:
                    aws_exception_to_result(aws, e)
                    raise e

                print_response(response)
                print_status_code(response)
                del response["ResponseMetadata"]

                logger.info("Saving wireless device to file")
                paths.save_device_json(wireless_device_id, response)

                logger.info("Generating MFG by calling provision.py")

                if getattr(sys, 'frozen', False):
                    # This is for Simplicity Studio only
                    provision_path = os.path.dirname(sys.executable)
                    is_context_studio = True
                elif __file__:
                    # This is when using the script outside of Simplicity Studio
                    provision_path = os.path.dirname(__file__)
                    provision_path = os.path.join(provision_path, "provision")
                    is_context_studio = False

                target_offset_address = None

                if request.targetPart == "all":
                    target_family = None
                elif request.targetPart is not None:
                    result = re.search(r'efr32(?P<family>[a-z]g[0-9]+)[A-Z]+[0-9]+F(?P<memory>[0-9]+)', request.targetPart, re.IGNORECASE)
                    target_family = result.group('family')
                    target_family = target_family.replace(target_family[0], 'x', 1).lower()
                    target_offset_address = get_target_offset_address(target_family, result.group('memory'))
                else:
                    target_family = None

                p = ProvisionWrapper(script_dir=provision_path, silabs_commander_path=e.commander_path, hardware_platform=BoardType.SiLabs, is_context_studio=is_context_studio)

                p.generate_mfg(target_family, wireless_device_path=paths.get_device_json_filepath(wireless_device_id, absPath=True),
                            device_profile_path=paths.get_profile_json_filepath(absPath=True),
                            input_type=InputType.AWS_API_JSONS,
                            output_dir=paths.get_device_dir(wireless_device_id, absPath=True),
                            offset_addr=target_offset_address)


                logger.info("Done!")

                result_device.deviceName = request.deviceName
                result_device.deviceID = wireless_device_id
                result_device.dir      = os.path.join(device_profile.dir, "WirelessDevice_" + wireless_device_id)
                result_device.smsn     = response['Sidewalk']['SidewalkManufacturingSn']
                result_device.id       = response['Id']
                result_device.destination = response['DestinationName']
                result_device.arn         = response['Arn']
                result_device.part     = request.targetPart
                result_device.mfgRange.startAddress = target_offset_address

                device_profile.devices.append(result_device)

            result_content.generatedProfiles.append(device_profile)

    # Reverse a dictionary: keys become values and values become keys
    def index_by_key(dict):
        indexed = {}
        for key, list in dict.items():
            for value in list:
                indexed[value] = key
        return indexed

    def get_target_base_address(device_family):
        base_address_by_family = index_by_key(BASE_ADDRESS)
        if (device_family in base_address_by_family.keys()):
            return base_address_by_family[device_family]
        else: return None

    def get_target_offset_address(device_family, device_memory):
        base_address = get_target_base_address(device_family)
        if(base_address is None):
            logger.error(f"Unknown target, family not recognised: {device_family}")
            return
        end_of_flash = hex(base_address + (int(device_memory)*1024))
        return hex(int(end_of_flash, 16) - MFG_START_ADDR_OFFSET_FROM_END_OF_FLASH)

    def make_dir(path):
        if os.path.exists(path) is False:
            try:
                os.mkdir(path)
            except FileExistsError:
                pass
            except OSError as err:
                logger.error(f"An error has occurred: {err}")
                raise

    class LockFileHandler:
        def __init__(self, project_path, lock_file_path) -> None:
            self.lock_file = lock_file_path
            self.current_sessionId = None
            self.current_path      = project_path

            if os.path.exists(self.lock_file):
                with open(self.lock_file, 'r') as file:
                    lock_json = json.load(file)
                    for item in lock_json['sessions']:
                        if item['path'] == self.current_path:
                            self.current_sessionId = item['sessionID']

        def IsAlreadyTriggered(self, sessionId):
            if self.current_sessionId == sessionId:
                return True
            else:
                return False

        def UpdateLockFile(self, new_sessionId):
            if os.path.exists(self.lock_file):
                lock_file_content = None

                with open(self.lock_file, 'r') as file:
                    lock_file_content = json.load(file)

                for item in lock_file_content['sessions']:
                    if item['path'] == self.current_path:
                        item['sessionID'] = new_sessionId

                with open(self.lock_file, 'w') as file:
                    json.dump(lock_file_content, file, indent=4)
            else:
                file_contect = {'sessions': [{
                    'path' : self.current_path,
                    'sessionID' : new_sessionId
                }]}
                with open(self.lock_file, 'w') as file:
                    json.dump(file_contect, file, indent=4)

    class PathsWrapper:
        def __init__(self, device_profile_id, working_directory = None):
            if working_directory is None:
                if getattr(sys, 'frozen', False):
                    base_path = os.path.dirname(sys.executable)
                elif __file__:
                    base_path = os.path.dirname(__file__)
            else:
                base_path = working_directory

            self.device_profile_dir = os.path.join(base_path, "mfg_output", "DeviceProfile_" + device_profile_id)
            make_dir(self.device_profile_dir)

        def get_profile_dir(self, absPath=False):
            if absPath:
                return os.path.abspath(self.device_profile_dir)
            return self.device_profile_dir

        def get_profile_json_filepath(self, absPath=False):
            p = os.path.join(self.device_profile_dir, "DeviceProfile.json")
            if absPath:
                return os.path.abspath(p)
            return p

        def save_profile_json(self, data):
            with open(self.get_profile_json_filepath(), 'w') as outfile:
                json.dump(data, outfile, indent=4)

        def get_device_dir(self, device_id, absPath=False):
            p = os.path.join(self.get_profile_dir(), "WirelessDevice_" + device_id)
            if absPath:
                return os.path.abspath(p)
            return p

        def get_device_json_filepath(self, device_id, absPath=False):
            p = os.path.join(self.get_device_dir(device_id), "WirelessDevice.json")
            if absPath:
                return os.path.abspath(p)
            return p

        def save_device_json(self, device_id, data):
            make_dir(self.get_device_dir(device_id))
            with open(self.get_device_json_filepath(device_id), 'w') as outfile:
                json.dump(data, outfile, indent=4)


    class AWSHandler:
        def __init__(self, access_key, secret_key, session_token, region="us-east-1", profile_name=None):
            if(profile_name == None):
                self.session = boto3.Session()
                self.client = self.session.client('iotwireless', endpoint_url=ENDPOINT, aws_access_key_id=access_key,
                                                                                        aws_secret_access_key=secret_key,
                                                                                        aws_session_token=session_token,
                                                                                        region_name=region)
            else:
                self.session = boto3.Session(profile_name=profile_name)
                self.client = self.session.client('iotwireless', endpoint_url=ENDPOINT, region_name=region)

    def print_response(api_response):
        logger.info(json.dumps(api_response, indent=2))


    def get_status_code(api_response):
        code = api_response.get("ResponseMetadata").get("HTTPStatusCode")
        return code


    def print_status_code(api_response):
        code = get_status_code(api_response)
        logger.info(f"Status: {code}")

    def aws_exception_to_result(client, exception):
        if client.exceptions.ValidationException is exception:
            result_content.generatorResult = ResultCodes.AWS_VALIDATION_EXCEPTION
        elif client.exceptions.ResourceNotFoundException is exception:
            result_content.generatorResult = ResultCodes.AWS_RESOURCE_NOT_FOUND
        elif client.exceptions.InternalServerException is exception:
            result_content.generatorResult = ResultCodes.AWS_INTERNAL_SERVER_ERROR
        elif client.exceptions.ThrottlingException is exception:
            result_content.generatorResult = ResultCodes.AWS_THROTTLING
        elif client.exceptions.ConflictException is exception:
            result_content.generatorResult = ResultCodes.COMMANDER_NOT_FOUND
        elif client.exceptions.AccessDeniedException is exception:
            result_content.generatorResult = ResultCodes.AWS_ACCESS_DENIED
        else:
            result_content.generatorResult = ResultCodes.AWS_GENERIC_ERROR

        log = ResultLog()
        log.type = 'error'
        log.code = result_content.generatorResult
        log.description = exception
        result_content.logs.append(log)

    def save_project_output():
        # Build output path: args.input/../../mfg_output
        output_path = os.path.join(os.path.abspath(args.input), "../../autogen/mfg_output")

        if not os.path.exists(output_path):
            return

        dst_path = os.path.join( args.output, "mfg_output")
        try:
            os.mkdir(dst_path)
        except FileExistsError:
            pass

        # copy autogen/mfg_output/* to args.output/mfg_output
        for root, dirs, files in os.walk(output_path):
            # For each directory in output_path, create a corresponding directory in dst_path
            for dir in dirs:
                dst_dir = os.path.join(dst_path, os.path.relpath(os.path.join(root, dir), output_path))
                try:
                    os.mkdir(dst_dir)
                except FileExistsError:
                    pass
            # For each file in output_path, copy it to dst_path
            for file in files:
                src_file = os.path.join(root, file)
                # replicate same tree structure in dst_path
                dst_file = os.path.join(dst_path, os.path.relpath(src_file, output_path))
                shutil.copyfile(src_file, dst_file)

    save_project_output()
    main()

except Exception as e:
    if result_content.generatorResult == ResultCodes.OK:
        result_content.generatorResult = ResultCodes.GENERIC_ERROR
        log = ResultLog()
        log.type = 'error'
        log.code = result_content.generatorResult
        log.description = e
        log.traceback = traceback.format_exc()
        result_content.logs.append(log)
        logger.info(e)

finally:
    try:
        os.mkdir(os.path.join( args.output, "mfg_output"))
    except FileExistsError:
        pass

    # New profile/device(s) has been generated
    if len(result_content.generatedProfiles) > 0:
        with open(os.path.join(args.output, "mfg_output", RESULT_FILE_NAME), "w") as file:
            file.write(result_content.toJSON())

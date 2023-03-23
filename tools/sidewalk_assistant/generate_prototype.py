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


try:
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
    from enum import IntEnum
    from libs.ProvisionWrapper import ProvisionWrapper, InputType, BoardType
    from libs.EnvConfig import EnvConfig

    logger = logging.getLogger()
    logging.basicConfig(level=logging.INFO)

    os.chdir(os.path.dirname(os.path.abspath(__file__)))

    ENDPOINT = 'https://api.iotwireless.us-east-1.amazonaws.com'
    CONFIG_FILE_NAME = 'mfg.runconf'
    RESULT_FILE_NAME = 'mfg_result.json'
    LOCK_FILE_NAME   = '.lock'

    class ResultLog:
        def __init__(self) -> None:
            self.type        = ''
            self.code        = 0
            self.message     = ''
            self.description = ''
            self.traceback   = ''

    class  ResultDevice:
        def __init__(self) -> None:
            self.deviceID = ''
            self.dir      = ''
            self.smsn     = ''
            self.id       = ''
            self.destination = ''
            self.arn      = ''
            self.part     = ''
            self.family   = ''

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


    parser = argparse.ArgumentParser(add_help=True)
    parser.add_argument('-i', '--instances', help="Number of instances to generate (default: 1)", required=False,
                        default=1)
    parser.add_argument('-in',  '--input',  help="Path of the input directory",  required=True)
    parser.add_argument('-out', '--output', help="Path of the output directory", required=True)
    args = parser.parse_args()

    result_content = Result()

    def main():

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
            make_dir(os.path.join(os.path.abspath(args.output), "mfg_output"))

            paths = PathsWrapper(request.deviceProfileId, os.path.abspath(args.output))
            paths.save_profile_json(response)
            logger.info(f"Saved DeviceProfile details to {paths.get_profile_json_filepath()}")

            device_profile.profileID = request.deviceProfileId
            device_profile.dir       = os.path.join( "autogen", "mfg_output", "DeviceProfile_" + request.deviceProfileId)

            for instanceNr in range(0, request.quantity):
                result_device = ResultDevice()

                logger.info(f"Creating a new WirelessDevice (instance nr {instanceNr})")

                try:
                    response = aws.create_wireless_device(Type='Sidewalk',
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
                    provision_path = os.path.dirname(sys.executable)
                elif __file__:
                    provision_path = os.path.dirname(__file__)
                    provision_path = os.path.join(provision_path, "provision")

                if request.targetPart is not None:
                    target_family = re.search(r'efr32(?P<family>[mb]g[0-9]+)', request.targetPart, re.IGNORECASE).group('family')
                    target_family = target_family.replace('m', 'x').replace('b', 'x').replace('M', 'x').replace('B', 'x').replace('G', 'g')
                else:
                    target_family = None

                p = ProvisionWrapper(script_dir=provision_path, silabs_commander_path=e.commander_path, hardware_platform=BoardType.SiLabs)

                p.generate_mfg(target_family, wireless_device_path=paths.get_device_json_filepath(wireless_device_id, absPath=True),
                            device_profile_path=paths.get_profile_json_filepath(absPath=True),
                            input_type=InputType.AWS_API_JSONS,
                            output_dir=paths.get_device_dir(wireless_device_id, absPath=True))

                logger.info("Done!")

                result_device.deviceID = wireless_device_id
                result_device.dir      = os.path.join(device_profile.dir, "WirelessDevice_" + wireless_device_id)
                result_device.smsn     = response['Sidewalk']['SidewalkManufacturingSn']
                result_device.id       = response['Id']
                result_device.destination = response['DestinationName']
                result_device.arn         = response['Arn']
                result_device.part     = request.targetPart

                device_profile.devices.append(result_device)
    
            result_content.generatedProfiles.append(device_profile)

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
        def __init__(self, access_key, secret_key, session_token, region ):
            self.session = boto3.Session()
            self.client = self.session.client('iotwireless', endpoint_url=ENDPOINT, aws_access_key_id=access_key,
                                                                                    aws_secret_access_key=secret_key,
                                                                                    aws_session_token=session_token,
                                                                                    region_name=region)

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

    with open(os.path.join(args.output, "mfg_output", RESULT_FILE_NAME), "w") as file:
         file.write(result_content.toJSON())

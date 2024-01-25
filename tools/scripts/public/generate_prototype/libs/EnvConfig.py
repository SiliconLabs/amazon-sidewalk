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


import json
import logging
import os

from .ProvisionWrapper import BoardType


logger = logging.getLogger()
logging.basicConfig(level=logging.INFO)

AWS_REGION = "us-east-1"

class GenerationRequest:
    def __init__(self) -> None:
        self.deviceProfileId   = ''
        self.deviceName        = ''
        self.destinationName   = ''
        self.targetPart        = ''
        self.quantity          = 0

class EnvConfig:
    def __init__(self, config_file_name, instances=1, aws_profile="default", name=None, dst_name="CFSDestination", target=None, commander=None):

        # This is when using the script with a configuration file (or within Simplicity Studio)
        if(config_file_name != None):
            self.config_file = config_file_name

            with open(self.config_file, 'r') as file:
                config_json = json.load(file)

            # This is when using the script from within Simplicity Studio
            if(config_json.get("sessionID") == None):
                self.aws_region  = config_json["awsAccount"].get("awsRegion", AWS_REGION)
                self.aws_profile = config_json["awsAccount"].get("awsProfile", aws_profile)
                if(config_json["commanderPath"] != None):
                    self.commander_path = config_json["commanderPath"]

                self.generation_requests = []
                for request in config_json["generationRequests"]:
                    generation_request = GenerationRequest()
                    generation_request.deviceName      = request['deviceName']
                    generation_request.destinationName = request['destinationName']
                    generation_request.quantity        = request['quantity']
                    generation_request.targetPart      = request['targetPart']
                    generation_request.deviceProfileId = request['deviceProfileId']

                    self.generation_requests.append(generation_request)

            # This is when using the script with a configuration file
            else:
                self.session_id     = config_json["sessionID"]
                self.aws_profile    = None
                self.commander_path = config_json["commanderPath"]

                self.aws_access_key        = config_json["awsAccount"].get("awsAccessKeyId")
                self.aws_secret_access_key = config_json["awsAccount"].get("awsSecretAccessKey")
                self.aws_session_token     = config_json["awsAccount"].get("awsSessionToken")
                self.aws_region            = config_json["awsAccount"].get("awsRegion")

                self.generation_requests = []

                for request in config_json["generationRequests"]:
                    generation_request = GenerationRequest()
                    generation_request.deviceName      = request['deviceName']
                    generation_request.deviceProfileId = request['deviceProfileId']
                    generation_request.destinationName = request['destinationName']
                    generation_request.targetPart      = request['targetPart']
                    generation_request.quantity        = request['quantity']

                    self.generation_requests.append(generation_request)
        # This is when using the script with command line arguments
        else:
            self.config_file = None
            self.aws_region        = AWS_REGION
            self.aws_profile       = aws_profile
            if(commander != None):
                self.commander_path = commander

            self.generation_requests = []
            generation_request = GenerationRequest()
            generation_request.deviceName      = name
            generation_request.destinationName = dst_name
            generation_request.quantity        = instances
            generation_request.targetPart      = target
            generation_request.deviceProfileId = None

            self.generation_requests.append(generation_request)

        logger.info(f"Loaded configuration: DESTINATION: {generation_request.destinationName} , "
                f"DEVICE_PROFILE: {generation_request.deviceProfileId}")

        self.hardware_platform = BoardType.SiLabs
        self.my_cwd = os.path.dirname(os.path.realpath(__file__))

    def update_generation_request(self, request):
        if(self.config_file is not None):
            with open(self.config_file, 'r') as file:
                config_json = json.load(file)

            config_json["generationRequests"] =  []
            for item in self.generation_requests:
                config_json["generationRequests"].append(item.__dict__)

            with open(self.config_file, "w") as file:
                json.dump(config_json, file, indent=4)
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

class GenerationRequest:
    def __init__(self) -> None:
        self.deviceProfileId   = ''
        self.destinationName   = ''
        self.targetPart        = ''
        self.quantity          = 0

class EnvConfig:
    def __init__(self, config_file_name):
        self.config_file = config_file_name

        with open(self.config_file, 'r') as file:
            config_json = json.load(file)

        self.session_id     = config_json["sessionID"]
        self.commander_path = config_json["commanderPath"]

        self.aws_access_key        = config_json["awsAccount"].get("awsAccessKeyId")
        self.aws_secret_access_key = config_json["awsAccount"].get("awsSecretAccessKey")
        self.aws_session_token     = config_json["awsAccount"].get("awsSessionToken")
        self.aws_region            = config_json["awsAccount"].get("awsRegion")

        self.generation_requests = []

        for request in config_json["generationRequests"]:
            generation_request = GenerationRequest()
            generation_request.deviceProfileId = request['deviceProfileId']
            generation_request.destinationName = request['destinationName']
            generation_request.targetPart      = request['targetPart']
            generation_request.quantity        = request['quantity']

            self.generation_requests.append(generation_request)

            logger.info(f"Loaded configuration: DESTINATION: {generation_request.destinationName} , "
                    f"DEVICE_PROFILE: {generation_request.deviceProfileId}")

        self.hardware_platform = BoardType.SiLabs
        self.my_cwd = os.path.dirname(os.path.realpath(__file__))

    def update_generation_request(self, request):
        with open(self.config_file, 'r') as file:
            config_json = json.load(file)

        config_json["generationRequests"] =  []
        for item in self.generation_requests:
            config_json["generationRequests"].append(item.__dict__)

        with open(self.config_file, "w") as file:
            json.dump(config_json, file, indent=4)
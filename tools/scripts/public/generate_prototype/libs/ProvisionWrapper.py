# Copyright 2023 Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: MIT-0

import logging
import os
import subprocess
import sys
import platform
from enum import Enum

logger = logging.getLogger()
logging.basicConfig(level=logging.INFO)

class BoardType(Enum):
    SiLabs = 1,
    All = 99


class InputType(Enum):
    CERTIFICATE_JSON = 0,
    AWS_API_JSONS = 1


class ProvisionWrapperException(Exception):
    pass

class ProvisionWrapper:
    def __init__(self, script_dir, silabs_commander_path=None, hardware_platform=BoardType.All, is_context_studio=True):
        self.main_path = os.path.abspath(script_dir)

        if getattr(sys, 'frozen', False):
            current_path = os.path.dirname(sys.executable)
            self.config_file= os.path.join(current_path , 'config.yaml')
        elif __file__:
            self.config_file = os.path.join('config', 'silabs', 'efr32', 'config.yaml')

        self.hardware_platform = hardware_platform

        current_platform = platform.system()
        provision_executable = 'provision'
        commander_executable = 'commander'

        self.is_context_studio = is_context_studio
        if is_context_studio:
            if current_platform == 'Windows':
                provision_executable = provision_executable + '.exe'
                commander_executable = commander_executable + '.exe'
            if current_platform == 'Linux':
                provision_executable = provision_executable
            if current_platform == 'Darwin':
                provision_executable = provision_executable
        else:
            provision_executable = provision_executable + '.py'

        self.provision_binary_path = os.path.join(self.main_path, provision_executable)

        if silabs_commander_path:
            self.commander = silabs_commander_path
        else:
            self.commander = commander_executable

        self.DEFAULT_BOARDS = ['xg21', 'xg24', 'xg28', 'xg23']

    def generate_mfg(self, target_family, output_dir, input_type, wireless_device_path=None, device_profile_path=None,
                     certificate_json=None, offset_addr=None):
        if input_type == InputType.AWS_API_JSONS:
            assert wireless_device_path, "For selected provisioning method, wireless device json is mandatory"
            assert device_profile_path, "For selected provisioning method, device_profile is mandatory"
            assert os.path.exists(wireless_device_path), f"File not found: {wireless_device_path}"
            assert os.path.exists(device_profile_path), f"File not found: {device_profile_path}"
        else:
            assert False, "Unknown provisioning method"

        board = self.hardware_platform

        if board == BoardType.SiLabs or board == BoardType.All:
            logger.info("  Generating MFG.S37 For SiLabs xG21, xG23, xG24 and xG28")
            sl_mfg_nvm3 = os.path.join(output_dir, "SiLabs_MFG.nvm3")
            sl_generic_mfg_s37 = os.path.join(output_dir, 'mfg.s37')

            # Generate a manufacturing page for the given OPN (family and flash address)
            if target_family != None:
                self.generate_nvm3_and_s37_from_aws_jsons(device_json=wireless_device_path,
                                    profile_json=device_profile_path,
                                    board=BoardType.SiLabs,
                                    out_nvm3=sl_mfg_nvm3,
                                    chip=target_family,
                                    offset_addr=offset_addr,
                                    outfile_s37=sl_generic_mfg_s37)
            # Generate manufacturing pages for all supported radio boards (not all parts)
            else:
                for board in self.DEFAULT_BOARDS:
                    sl_mfg_s37 = os.path.join(output_dir, f'MFG_{board.upper()}.s37')
                    self.generate_nvm3_and_s37_from_aws_jsons(device_json=wireless_device_path,
                                                            profile_json=device_profile_path,
                                                            board=BoardType.SiLabs,
                                                            out_nvm3=sl_mfg_nvm3,
                                                            chip=board,
                                                            outfile_s37=sl_mfg_s37)

    def generate_nvm3_and_s37_from_aws_jsons(self, device_json, profile_json, board, out_nvm3, chip, outfile_s37, offset_addr=None):
        assert board == BoardType.SiLabs, "Operation supported only for SiLabs"
        logger.info(self.provision_binary_path)

        args=[self.provision_binary_path, 'silabs', 'aws', '--wireless_device_json', device_json,
                                      '--commander-bin', self.commander,
                                      '--device_profile_json', profile_json,
                                      '--output_nvm3', out_nvm3,
                                      '--chip', chip, '--output_s37', outfile_s37]

        if(not self.is_context_studio):
            args.insert(0, "python3")

        if(offset_addr is not None):
            args.append('--addr')
            args.append(offset_addr)
        
        result = subprocess.run(args=args,
                               cwd=self.main_path, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print_subprocess_results(result, subprocess_name="provision.py")

def print_subprocess_results(result, subprocess_name="", withAssert=True):
    if result.returncode != 0:
        message = " ".join(result.args)
        message += " \n"
        message += " " + result.stdout.decode()
        message += " " + result.stderr.decode()
        raise ProvisionWrapperException(message)

    for line in result.stdout.decode().splitlines():
        logger.info(line)
        if withAssert:
            assert 'error' not in line, f"Something went wrong after calling subprocess {subprocess_name}"

    for line in result.stderr.decode().splitlines():
        logger.info(line)
        if withAssert:
            assert 'error' not in line, f"Something went wrong after calling subprocess {subprocess_name}"

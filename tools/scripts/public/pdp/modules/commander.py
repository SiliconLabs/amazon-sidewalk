# !/usr/bin/env python3

import subprocess
import re

# commander output handling
COMMANDER_EXP_MSG_OK_PATTERN = r"DONE"
COMMANDER_EXP_MSG_ERR_PATTERN = r"ERROR"                                          # printed on stderr
COMMANDER_EXP_MSG_ERR_NOT_FOUND_PATTERN = r"commander: not found"                 # printed on stderr
COMMANDER_EXP_MSG_ERR_VERIFICATION_ERROR_PATTERN = r"ERROR: Verification failed!" # printed on stdout

# NVM3 content file line structure
NVM3_CONTENT_LINE_STRUCT = "{0}:OBJ:{1}"

# parameterizable commander commands
COMMANDER_ARG_SERIAL_NO = "--serialno {0}"
COMMANDER_ARG_DEVICE = "--device {0}"
COMMANDER_CMD_NVM3_INITFILE = "commander nvm3 initfile --address {0} --size {1} --device {2} --outfile {3}"
COMMANDER_CMD_NVM3_SET = "commander nvm3 set {0} --nvm3file {1} --outfile {2}"
COMMANDER_CMD_CONVERT = "commander convert {0} {1} --outfile {2}"
COMMANDER_CMD_MASSERASE = "commander device masserase {0} {1}"
COMMANDER_CMD_FLASH = "commander flash {0} {1} {2}"
COMMANDER_CMD_RESET = "commander device reset {0}"
COMMANDER_CMD_VERIFY_BLANK = "commander verify --blank --region @mainflash {0}"

class Commander:
  def __init__(self, jlink_ser=None):
    self.jlink_ser = jlink_ser

  def _process(self, cmd):
    result = subprocess.run(cmd, capture_output=True, text=True, shell=True)
    if re.search(COMMANDER_EXP_MSG_ERR_PATTERN, result.stderr) or\
       re.search(COMMANDER_EXP_MSG_ERR_NOT_FOUND_PATTERN, result.stderr) or\
       re.search(COMMANDER_EXP_MSG_ERR_VERIFICATION_ERROR_PATTERN, result.stdout):
      raise SystemError(result.stderr)

  def masserase(self, device):
    cmd = COMMANDER_CMD_MASSERASE.format(COMMANDER_ARG_DEVICE.format(device), "")
    if self.jlink_ser:
      cmd = COMMANDER_CMD_MASSERASE.format(COMMANDER_ARG_DEVICE.format(device), COMMANDER_ARG_SERIAL_NO.format(self.jlink_ser))
    self._process(cmd)

  def verify_blank(self, device):
    cmd = COMMANDER_CMD_VERIFY_BLANK.format(COMMANDER_ARG_DEVICE.format(device), "")
    if self.jlink_ser:
      cmd = COMMANDER_CMD_VERIFY_BLANK.format(COMMANDER_ARG_DEVICE.format(device), COMMANDER_ARG_SERIAL_NO.format(self.jlink_ser))
    self._process(cmd)

  def reset(self, device):
    cmd = COMMANDER_CMD_RESET.format(COMMANDER_ARG_DEVICE.format(device), "")
    if self.jlink_ser:
      cmd = COMMANDER_CMD_RESET.format(COMMANDER_ARG_DEVICE.format(device), COMMANDER_ARG_SERIAL_NO.format(self.jlink_ser))
    self._process(cmd)

  def flash(self, filename, device):
    cmd = COMMANDER_CMD_FLASH.format(filename, COMMANDER_ARG_DEVICE.format(device), "")
    if self.jlink_ser:
      cmd = COMMANDER_CMD_FLASH.format(filename, COMMANDER_ARG_DEVICE.format(device), COMMANDER_ARG_SERIAL_NO.format(self.jlink_ser))
    self._process(cmd)

  def create_nvm3_initfile(self, nvm3inststartaddress, nvm3instsize, device, outfile):
    cmd = COMMANDER_CMD_NVM3_INITFILE.format(nvm3inststartaddress, nvm3instsize, device, outfile)
    self._process(cmd)

  def set_nvm3(self, initfile, nvm3file, outfile):
    cmd = COMMANDER_CMD_NVM3_SET.format(initfile, nvm3file, outfile)
    self._process(cmd)

  def convert(self, file1, file2, outfile):
    arg1 = "" if file2 == None else file2 # omit file2 if not provided
    cmd = COMMANDER_CMD_CONVERT.format(file1, arg1, outfile)
    self._process(cmd)

  def generate_nvm3_content(self, kv_set):
    objs = list()
    for k, v in kv_set.items():
      objs.append(NVM3_CONTENT_LINE_STRUCT.format("0x%04x" % k, v))
      objs.append("\n")
    objs.pop()
    return "".join(objs)
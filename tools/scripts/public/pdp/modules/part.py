# !/usr/bin/env python3

import re

FLASH_BASE_ADDRESS = {
  "xg21": 0x00000000,
  "xg23": 0x08000000,
  "xg24": 0x08000000,
  "xg25": 0x08000000,
  "xg28": 0x08000000,
}

# size_nvm3_instance (0x6000) + size_empty_page_at_the_end (0x2000)
MFG_START_ADDR_OFFSET_FROM_END_OF_FLASH = 0x8000

class Part:
  def __init__(self, part):
    part = part.lower()
    result = re.search(r'(?i)efr32(?P<family>[a-z]g[0-9]+)(?P<option>[a-z][0-9]+)f(?P<memory>[0-9]+)', part)
    self._family = result.group('family')
    self._family_general = 'x' + result.group('family')[1:]
    self._memory = result.group('memory')
    self._option = result.group('option')

  def get_mfg_page_start_addr_str(self) -> str:
    return f"0x{self.get_mfg_page_start_addr():08x}"

  def get_mfg_page_start_addr(self) -> str:
    return FLASH_BASE_ADDRESS[self._family_general] + (int(self._memory) * 1024) - MFG_START_ADDR_OFFSET_FROM_END_OF_FLASH

  def get_jlink_device(self) -> str:
    return "".join(["EFR32", self._family.upper(), self._option[0].upper(), "xxx", "F", self._memory])

  def get_mfg_page_size(self) -> int:
    return 0x6000

  def get_stack_size(self) -> int:
    return 0x1000

  def get_ram_start_addr(self) -> int:
    return 0x20000000
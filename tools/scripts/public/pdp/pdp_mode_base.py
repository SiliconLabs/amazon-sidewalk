# !/usr/bin/env python3

from modules.pdp import PDP

class PDPModeBase:
  def __init__(self, logger, part, jlink_ser, pdp_img):
    _soc_ram_st_addr = part.get_ram_start_addr()
    _soc_stack_size = part.get_stack_size()
    self._logger = logger
    self._pdp = PDP(self._logger, part.get_jlink_device(), jlink_ser, _soc_ram_st_addr, _soc_stack_size, pdp_img)

  def _provision_dd(self, **kwargs):
    raise NotImplementedError("_provision_dd not implemented")

  def _arg_check(self, **kwargs):
    for arg in self._required_kwarg_list:
      if not kwargs[arg]:
        raise ValueError("arg {0} does not exist".format(arg))

  def _flash_pdp_bin(self):
    self._pdp.comm_open()
    self._pdp.comm_reset_and_halt()
    self._pdp.burn_ram_img()
    self._pdp.comm_close()

  def _provision(self, **kwargs):
    self._pdp.comm_open(start_rtt=True)
    self._provision_dd(**kwargs)
    self._pdp.comm_close(stop_rtt=True)

  def execute(self, **kwargs):
    self._arg_check(**kwargs)
    self._flash_pdp_bin()
    self._provision(**kwargs)
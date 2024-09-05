# !/usr/bin/env python3

from modules.pdp import PDP

class PDPModeBase:
  def __init__(self, logger, commander, pdp_img, iostream):
    self._logger = logger
    self._commander = commander
    self._pdp = PDP(self._logger, commander, pdp_img, iostream)

  def _provision_dd(self, **kwargs):
    raise NotImplementedError("_provision_dd not implemented")

  def _arg_check(self, **kwargs):
    for arg in self._required_kwarg_list:
      if not kwargs[arg]:
        raise ValueError("arg {0} does not exist".format(arg))

  def _flash_pdp(self):
    self._pdp.flash_pdp()

  def _provision(self, **kwargs):
    self._pdp.comm_open()
    self._provision_dd(**kwargs)
    self._pdp.comm_close()
    self._commander.reset()

  def execute(self, **kwargs):
    self._arg_check(**kwargs)
    self._flash_pdp()
    self._provision(**kwargs)
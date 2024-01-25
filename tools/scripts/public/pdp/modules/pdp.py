# !/usr/bin/env python3

import binascii
from enum import Enum
from .serial_wire import *
from .pdp_api import *

class PDPMode(str, Enum):
  PRIV_KEY_PROV = 'priv_key_prov'
  ON_DEV_CERT_GEN = 'on_dev_cert_gen'

class PDP:
  def __init__(self, logger, jlink_dev, jlink_ser, soc_ram_st_addr, soc_stack_size, pdp_img):
    self._logger = logger
    self._soc_ram_st_addr = soc_ram_st_addr
    self._soc_stack_st_addr = self._soc_ram_st_addr + soc_stack_size
    self._pdp_img = pdp_img
    self._sw = SerialWire(jlink_dev, jlink_ser)

  def comm_open(self, start_rtt=False):
    self._logger.info("Opening serial wire connection (start_rtt: {0})".format(start_rtt))
    self._sw.connect()
    if start_rtt:
      self._sw.rtt_start()

  def comm_reset_and_halt(self):
    self._logger.info("Resetting and halting serial wire connection")
    self._sw.reset_and_halt()

  def comm_close(self, stop_rtt=False):
    self._logger.info("Closing serial wire connection (stop_rtt: {0})".format(stop_rtt))
    if stop_rtt:
      self._sw.rtt_stop()
      self._sw.reset()
    self._sw.close()

  def comm_send_receive(self, tx_pkt):
    self._logger.debug("tx: {0}".format(binascii.hexlify(tx_pkt)))
    if not self._check_tx(tx_pkt):
      return None
    self._sw.rtt_send(tx_pkt)
    rx_pkt = self._sw.rtt_receive()
    self._logger.debug("rx: {0}".format(binascii.hexlify(bytes(rx_pkt))))
    if not self._check_rx(rx_pkt):
      return None
    return rx_pkt[PROTOCOL_DATA_START_IDX:]

  def burn_ram_img(self):
    self._logger.info("Burning PDP application image at {0} (size: {1} bytes)".format(hex(self._soc_ram_st_addr), len(self._pdp_img)))
    reset_ok, written = self._sw.burn_ram_img(self._soc_ram_st_addr, self._soc_stack_st_addr, self._pdp_img)
    if reset_ok:
      self._logger.debug("{0} bytes written".format(written))

  def _check_tx(self, tx_pkt):
    if len(tx_pkt) >= SERIAL_WIRE_TX_BUF_SIZE:
      self._logger.error('tx packet too big {0}'.format(SERIAL_WIRE_TX_BUF_SIZE))
      return False
    return True

  def _check_rx(self, rx_pkt):
    status = int.from_bytes(rx_pkt[PROTOCOL_STATUS_RANGE], "little")
    if status != PROTOCOL_STATUS_NO_ERR:
      if rx_pkt[PROTOCOL_INT_ERR_RANGE] != None and int.from_bytes(rx_pkt[PROTOCOL_INT_ERR_RANGE], "little") != PROTOCOL_STATUS_NO_ERR:
        int_err = rx_pkt[PROTOCOL_INT_ERR_RANGE]
        self._logger.error("Wrong expected status: {0} (internal_error: {1})".format(status, int.from_bytes(int_err)))
      else:
        self._logger.error("Wrong expected status: {0}".format(status))
      return False
    self._logger.info("Status: OK({0})".format(status))
    return True
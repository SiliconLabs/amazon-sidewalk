# !/usr/bin/env python3

import binascii
from enum import Enum
from .iostream import *
from .iostream_rtt import *
from .pdp_api import *
from .commander import *

class PDPMode(str, Enum):
  PRIV_KEY_PROV = 'priv_key_prov'
  ON_DEV_CERT_GEN = 'on_dev_cert_gen'

class PDP:
  def __init__(self, logger, commander, pdp_img, iostream):
    self._logger = logger
    self._pdp_img = pdp_img
    self._commander = commander
    self._iostream = iostream

  def comm_open(self):
    self._logger.info("Opening IOStream")
    self._iostream.connect()
    self._iostream.start()

  def comm_close(self):
    self._logger.info("Closing IOStream")
    self._iostream.stop()
    self._iostream.reset()
    self._iostream.close()

  def comm_send_receive(self, tx_pkt):
    self._logger.debug("tx: {0}".format(binascii.hexlify(tx_pkt)))
    self._sanity_check_tx(tx_pkt)
    self._iostream.send(tx_pkt)
    rx_pkt = self._iostream.receive()
    self._logger.debug("rx: {0}".format(binascii.hexlify(bytes(rx_pkt))))
    self._sanity_check_rx(rx_pkt)
    return rx_pkt[PROTOCOL_DATA_START_IDX:]

  def flash_pdp(self):
    self._logger.info("Flashing PDP application image")
    self._commander.flash(self._pdp_img)

  def _sanity_check_tx(self, tx_pkt):
    if len(tx_pkt) >= IOSTREAM_TX_BUF_SIZE:
      self._logger.error('tx packet too big {0}'.format(len(tx_pkt)))
      raise Exception()

  def _sanity_check_rx(self, rx_pkt):
    status = int.from_bytes(rx_pkt[PROTOCOL_STATUS_RANGE], "little")
    if status != PROTOCOL_STATUS_NO_ERR:
      if rx_pkt[PROTOCOL_INT_ERR_RANGE] != None and int.from_bytes(rx_pkt[PROTOCOL_INT_ERR_RANGE], "little") != PROTOCOL_STATUS_NO_ERR:
        int_err = rx_pkt[PROTOCOL_INT_ERR_RANGE]
        self._logger.error("Wrong expected status: {0} (internal_error: {1})".format(status, binascii.hexlify(int_err)))
      else:
        self._logger.error("Wrong expected status: {0}".format(status))
      raise Exception()
    self._logger.info("Status: OK({0})".format(status))
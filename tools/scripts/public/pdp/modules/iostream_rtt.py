# !/usr/bin/env python3

import pylink
from .iostream import IOSTREAM_RX_BUF_SIZE

class IOStream_RTT:
  def __init__(self, chip_name, jlink_ser):
    self._jlink = pylink.JLink()
    self._chip_name = chip_name
    self._jlink_ser = jlink_ser

  def connect(self):
    self._jlink.open(serial_no=self._jlink_ser)
    self._jlink.set_tif(interface=pylink.JLinkInterfaces.SWD)
    self._jlink.connect(chip_name=self._chip_name, speed="auto", verbose=True)

  def reset(self):
    self._jlink.reset(halt=False)

  def start(self):
    self._jlink.rtt_start()

  def stop(self):
    self._jlink.rtt_stop()

  def send(self, data):
    nb_sent = 0
    while nb_sent == 0:
      nb_sent = self._jlink.rtt_write(0, data)
    return nb_sent

  def receive(self):
    data = bytes()
    while len(data) == 0:
      data = self._jlink.rtt_read(0, IOSTREAM_RX_BUF_SIZE)
    return data

  def close(self):
    self._jlink.close()
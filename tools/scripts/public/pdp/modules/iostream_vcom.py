# !/usr/bin/env python3

import serial
import binascii
import time
from .iostream import IOSTREAM_RX_BUF_SIZE

READ_TIMEOUT = 0.2
DEVICE_BOOT_DELAY = 0.5 # wait for the VCOM port to be ready

class IOStream_VCOM:
  def __init__(self, chip_name, jlink_ser, vcom_port):
    self._ser = serial.Serial(port=vcom_port, baudrate=115200, timeout=READ_TIMEOUT)

  def connect(self):
    time.sleep(DEVICE_BOOT_DELAY)
    if self._ser.is_open:
      self._ser.close()
    self._ser.open()

  def reset(self):
    pass

  def start(self):
    pass

  def stop(self):
    pass

  def send(self, data):
    return self._ser.write(data)

  def receive(self):
    data = bytes()
    while len(data) == 0:
      data = self._ser.read(IOSTREAM_RX_BUF_SIZE)
    return data

  def close(self):
    if self._ser.is_open:
      self._ser.close()
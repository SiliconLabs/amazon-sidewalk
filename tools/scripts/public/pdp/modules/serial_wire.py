# !/usr/bin/env python3

import pylink

SERIAL_WIRE_TX_BUF_SIZE = 1024
SERIAL_WIRE_RX_BUF_SIZE = 1024

class SerialWire:
  def __init__(self, chip_name, jlink_ser):
    self.jlink = pylink.JLink()
    self.chip_name = chip_name
    self.jlink_ser = jlink_ser

  @property
  def is_connected(self):
    return self.jlink.connected()

  def connect(self):
    self.jlink.open(serial_no=self.jlink_ser)
    self.jlink.set_tif(interface=pylink.JLinkInterfaces.SWD)
    self.jlink.connect(chip_name=self.chip_name, speed="auto", verbose=True)

  def reset_and_halt(self):
    self.jlink.reset(halt=True)

  def reset(self):
    self.jlink.reset(halt=False)

  def rtt_start(self):
    self.jlink.rtt_start()

  def rtt_stop(self):
    self.jlink.rtt_stop()

  def rtt_send(self, data):
    nb_sent = 0
    while nb_sent == 0:
      nb_sent = self.jlink.rtt_write(0, data)
    return nb_sent

  def rtt_receive(self):
    data = bytes()
    while len(data) == 0:
      data = self.jlink.rtt_read(0, SERIAL_WIRE_RX_BUF_SIZE)
    return data

  def close(self):
    self.jlink.close()

  def burn_ram_img(self, ram_addr, stack_addr, img):
    if img:
      written = self.jlink.memory_write8(addr=ram_addr, data=list(img))
      self.jlink.register_write(reg_index=13, value=stack_addr) # SP
      self.jlink.register_write(reg_index=15, value=stack_addr) # PC
      reset_ok = self.jlink.restart(num_instructions=0, skip_breakpoints=False)
      return reset_ok, written
    else:
      return False, 0
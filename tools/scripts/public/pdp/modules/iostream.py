# !/usr/bin/env python3

from enum import Enum

IOSTREAM_TX_BUF_SIZE = 1024
IOSTREAM_RX_BUF_SIZE = 1024

class IOStream(str, Enum):
  IOSTREAM_RTT = 'rtt'
  IOSTREAM_VCOM = 'vcom'
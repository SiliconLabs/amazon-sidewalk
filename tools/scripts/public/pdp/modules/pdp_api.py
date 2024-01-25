# !/usr/bin/env python3

from enum import Enum

BYTE_ORDER_LE = 'little'

# after 4-byte status either internal error or data
PROTOCOL_STATUS_START_IDX = 0
PROTOCOL_STATUS_END_IDX = 4
PROTOCOL_INT_ERR_START_IDX = 4
PROTOCOL_INT_ERR_END_IDX = 8
PROTOCOL_DATA_START_IDX = 4
PROTOCOL_STATUS_RANGE = slice(PROTOCOL_STATUS_START_IDX, PROTOCOL_STATUS_END_IDX)
PROTOCOL_INT_ERR_RANGE = slice(PROTOCOL_INT_ERR_START_IDX, PROTOCOL_INT_ERR_END_IDX)

# SMSN generation
PROTOCOL_DATA_SMSN_LEN_START_IDX = 0
PROTOCOL_DATA_SMSN_LEN_END_IDX = 4
PROTOCOL_DATA_SMSN_LEN_RANGE = slice(PROTOCOL_DATA_SMSN_LEN_START_IDX, PROTOCOL_DATA_SMSN_LEN_END_IDX)
PROTOCOL_DATA_SMSN_VALUE_START_IDX = 4

# CSR generation
PROTOCOL_DATA_CSR_LEN_START_IDX = 0
PROTOCOL_DATA_CSR_LEN_END_IDX = 4
PROTOCOL_DATA_CSR_LEN_RANGE = slice(PROTOCOL_DATA_CSR_LEN_START_IDX, PROTOCOL_DATA_CSR_LEN_END_IDX)
PROTOCOL_DATA_CSR_VALUE_START_IDX = 4

# protocol error status
PROTOCOL_STATUS_NO_ERR = 0

# used in on-device certificate generation
class CryptoCurve(int, Enum):
  ED25519 = 1
  P256R1 = 2

class CommandList:
  # Private key provisioning
  PRIV_KEY_PROV_WRITE_NVM3 = 0
  PRIV_KEY_PROV_INJECT_KEY = 1
  # On-device certificate generation
  ON_DEV_CERT_GEN_INIT = 2
  ON_DEV_CERT_GEN_GEN_SMSN = 3
  ON_DEV_CERT_GEN_GEN_CSR = 4
  ON_DEV_CERT_GEN_WRITE_CERT_CHAIN = 5
  ON_DEV_CERT_GEN_WRITE_APP_SRV_PUB_KEY = 6
  ON_DEV_CERT_GEN_STORE = 7

class Base(object):
  def serialize(self):
    p = bytearray()
    p.extend(int(self.cmd).to_bytes(2, BYTE_ORDER_LE))
    p.extend(len(self.body).to_bytes(2, BYTE_ORDER_LE))
    p.extend(self.body)
    return p

class PrivKeyProv_WriteNVM3(Base):
  def __init__(self, obj_key, obj_data):
    self.cmd = CommandList.PRIV_KEY_PROV_WRITE_NVM3
    self.body = bytearray()
    self.body.extend(obj_key.to_bytes(4, BYTE_ORDER_LE))
    self.body.extend(len(obj_data).to_bytes(2, BYTE_ORDER_LE))
    self.body.extend(obj_data)

class PrivKeyProv_InjectKey(Base):
  def __init__(self, ka_lifetime, ka_location, ka_usage_flags, ka_bits, ka_algo, ka_type, key_id, key):
    self.cmd = CommandList.PRIV_KEY_PROV_INJECT_KEY
    self.body = bytearray()
    self.body.extend(ka_lifetime.to_bytes(4, BYTE_ORDER_LE))
    self.body.extend(ka_location.to_bytes(4, BYTE_ORDER_LE))
    self.body.extend(ka_usage_flags.to_bytes(4, BYTE_ORDER_LE))
    self.body.extend(ka_bits.to_bytes(4, BYTE_ORDER_LE))
    self.body.extend(ka_algo.to_bytes(4, BYTE_ORDER_LE))
    self.body.extend(ka_type.to_bytes(1, BYTE_ORDER_LE))
    self.body.extend(key_id.to_bytes(4, BYTE_ORDER_LE))
    self.body.extend(len(key).to_bytes(4, BYTE_ORDER_LE))
    self.body.extend(key)

class OnDevCertGen_Init(Base):
  def __init__(self):
    self.cmd = CommandList.ON_DEV_CERT_GEN_INIT
    self.body = bytearray()

class OnDevCertGen_GenSMSN(Base):
  def __init__(self, dev_type, dsn, apid, board_id):
    self.cmd = CommandList.ON_DEV_CERT_GEN_GEN_SMSN
    self.body = bytearray()
    self.body.extend(len(dev_type).to_bytes(2, BYTE_ORDER_LE))
    self.body.extend(len(dsn).to_bytes(2, BYTE_ORDER_LE))
    self.body.extend(len(apid).to_bytes(2, BYTE_ORDER_LE))
    self.body.extend(len(board_id).to_bytes(2, BYTE_ORDER_LE))
    self.body.extend(dev_type.encode())
    self.body.extend(dsn.encode())
    self.body.extend(apid.encode())
    if len(board_id):
      self.body.extend(board_id.encode())

class OnDevCertGen_GenCSR(Base):
  def __init__(self, crypto_curve):
    self.cmd = CommandList.ON_DEV_CERT_GEN_GEN_CSR
    self.body = bytearray()
    self.body.extend(crypto_curve.value.to_bytes(1, BYTE_ORDER_LE))

class OnDevCertGen_WriteCertChain(Base):
  def __init__(self, crypto_curve, cert):
    self.cmd = CommandList.ON_DEV_CERT_GEN_WRITE_CERT_CHAIN
    self.body = bytearray()
    self.body.extend(crypto_curve.value.to_bytes(1, BYTE_ORDER_LE))
    self.body.extend(len(cert).to_bytes(2, BYTE_ORDER_LE))
    self.body.extend(cert)

class OnDevCertGen_WriteAppSrvPubKey(Base):
  def __init__(self, app_srv_pub_key):
    self.cmd = CommandList.ON_DEV_CERT_GEN_WRITE_APP_SRV_PUB_KEY
    self.body = bytearray()
    self.body.extend(len(app_srv_pub_key).to_bytes(2, BYTE_ORDER_LE))
    self.body.extend(app_srv_pub_key)

class OnDevCertGen_Store(Base):
  def __init__(self):
    self.cmd = CommandList.ON_DEV_CERT_GEN_STORE
    self.body = bytearray()
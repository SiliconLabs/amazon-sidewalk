# !/usr/bin/env python3

import json
from enum import Enum
from ctypes import Structure, c_ubyte
from prodict import Prodict
from base64 import b64decode

# default PSA key IDs
PSA_KEY_ID_ED25519_DEFAULT = 1
PSA_KEY_ID_P256R1_DEFAULT = 2

# certificate field lengths
SMSN_SIZE = 32
SERIAL_SIZE = 4
PRK_SIZE = 32
ED25519_PUB_SIZE = 32
P256R1_PUB_SIZE = 64
SIG_SIZE = 64

class SidCertType(str, Enum):
  PROTOTYPE = 'proto'
  PRODUCTION = 'prod'

MFG_NVM3_KEY_BASE = 0xA9000 # see component/includes/projects/sid/sal/silabs/sid_pal/include/nvm3_manager.h

# this enumeration has to be in sync with
# r-gerrit/projects/sid/sal/common/public/sid_pal_ifc/mfg_store/sid_pal_mfg_store_ifc.h
# it stores common manufacturing object IDs for all the silicon vendors
class MfgObj(Enum):
  SMSN = MFG_NVM3_KEY_BASE + 0x4
  APP_PUB_ED25519 = MFG_NVM3_KEY_BASE + 0x5
  DEVICE_PRIV_ED25519 = MFG_NVM3_KEY_BASE + 0x6
  DEVICE_PUB_ED25519 = MFG_NVM3_KEY_BASE + 0x7
  DEVICE_PUB_ED25519_SIGNATURE = MFG_NVM3_KEY_BASE + 0x8
  DEVICE_PRIV_P256R1 = MFG_NVM3_KEY_BASE + 0x9
  DEVICE_PUB_P256R1 = MFG_NVM3_KEY_BASE + 0xA
  DEVICE_PUB_P256R1_SIGNATURE = MFG_NVM3_KEY_BASE + 0xB
  DAK_PUB_ED25519 = MFG_NVM3_KEY_BASE + 0xC
  DAK_PUB_ED25519_SIGNATURE = MFG_NVM3_KEY_BASE + 0xD
  DAK_ED25519_SERIAL = MFG_NVM3_KEY_BASE + 0xE
  DAK_PUB_P256R1 = MFG_NVM3_KEY_BASE + 0xF
  DAK_PUB_P256R1_SIGNATURE = MFG_NVM3_KEY_BASE + 0x10
  DAK_P256R1_SERIAL = MFG_NVM3_KEY_BASE + 0x11
  PRODUCT_PUB_ED25519 = MFG_NVM3_KEY_BASE + 0x12
  PRODUCT_PUB_ED25519_SIGNATURE = MFG_NVM3_KEY_BASE + 0x13
  PRODUCT_ED25519_SERIAL = MFG_NVM3_KEY_BASE + 0x14
  PRODUCT_PUB_P256R1 = MFG_NVM3_KEY_BASE + 0x15
  PRODUCT_PUB_P256R1_SIGNATURE = MFG_NVM3_KEY_BASE + 0x16
  PRODUCT_P256R1_SERIAL = MFG_NVM3_KEY_BASE + 0x17
  MAN_PUB_ED25519 = MFG_NVM3_KEY_BASE + 0x18
  MAN_PUB_ED25519_SIGNATURE = MFG_NVM3_KEY_BASE + 0x19
  MAN_ED25519_SERIAL = MFG_NVM3_KEY_BASE + 0x1A
  MAN_PUB_P256R1 = MFG_NVM3_KEY_BASE + 0x1B
  MAN_PUB_P256R1_SIGNATURE = MFG_NVM3_KEY_BASE + 0x1C
  MAN_P256R1_SERIAL = MFG_NVM3_KEY_BASE + 0x1D
  SW_PUB_ED25519 = MFG_NVM3_KEY_BASE + 0x1E
  SW_PUB_ED25519_SIGNATURE = MFG_NVM3_KEY_BASE + 0x1F
  SW_ED25519_SERIAL = MFG_NVM3_KEY_BASE + 0x20
  SW_PUB_P256R1 = MFG_NVM3_KEY_BASE + 0x21
  SW_PUB_P256R1_SIGNATURE = MFG_NVM3_KEY_BASE + 0x22
  SW_P256R1_SERIAL = MFG_NVM3_KEY_BASE + 0x23
  AMZN_PUB_ED25519 = MFG_NVM3_KEY_BASE + 0x24
  AMZN_PUB_P256R1 = MFG_NVM3_KEY_BASE + 0x25
  APID = MFG_NVM3_KEY_BASE + 0x26

# __packed__ C stucture of ED25519 certificate after decoded from base64
class Cert_ED25519(Structure):
  _fields_ = [
    (MfgObj.SMSN.name, c_ubyte * SMSN_SIZE),
    (MfgObj.DEVICE_PUB_ED25519.name, c_ubyte * ED25519_PUB_SIZE),
    (MfgObj.DEVICE_PUB_ED25519_SIGNATURE.name, c_ubyte * SIG_SIZE),
    (MfgObj.DAK_ED25519_SERIAL.name, c_ubyte * SERIAL_SIZE),
    (MfgObj.DAK_PUB_ED25519.name, c_ubyte * ED25519_PUB_SIZE),
    (MfgObj.DAK_PUB_ED25519_SIGNATURE.name, c_ubyte * SIG_SIZE),
    (MfgObj.PRODUCT_ED25519_SERIAL.name, c_ubyte * SERIAL_SIZE),
    (MfgObj.PRODUCT_PUB_ED25519.name, c_ubyte * ED25519_PUB_SIZE),
    (MfgObj.PRODUCT_PUB_ED25519_SIGNATURE.name, c_ubyte * SIG_SIZE),
    (MfgObj.MAN_ED25519_SERIAL.name, c_ubyte * SERIAL_SIZE),
    (MfgObj.MAN_PUB_ED25519.name, c_ubyte * ED25519_PUB_SIZE),
    (MfgObj.MAN_PUB_ED25519_SIGNATURE.name, c_ubyte * SIG_SIZE),
    (MfgObj.SW_ED25519_SERIAL.name, c_ubyte * SERIAL_SIZE),
    (MfgObj.SW_PUB_ED25519.name, c_ubyte * ED25519_PUB_SIZE),
    (MfgObj.SW_PUB_ED25519_SIGNATURE.name, c_ubyte * SIG_SIZE),
    ("", c_ubyte * SERIAL_SIZE), # not used
    (MfgObj.AMZN_PUB_ED25519.name, c_ubyte * ED25519_PUB_SIZE),
    ("", c_ubyte * SIG_SIZE), # not used
  ]

# __packed__ C stucture of P256R1 certificate after decoded from base64
class Cert_P256R1(Structure):
  _fields_ = [
    (MfgObj.SMSN.name, c_ubyte * SMSN_SIZE),
    (MfgObj.DEVICE_PUB_P256R1.name, c_ubyte * P256R1_PUB_SIZE),
    (MfgObj.DEVICE_PUB_P256R1_SIGNATURE.name, c_ubyte * SIG_SIZE),
    (MfgObj.DAK_P256R1_SERIAL.name, c_ubyte * SERIAL_SIZE),
    (MfgObj.DAK_PUB_P256R1.name, c_ubyte * P256R1_PUB_SIZE),
    (MfgObj.DAK_PUB_P256R1_SIGNATURE.name, c_ubyte * SIG_SIZE),
    (MfgObj.PRODUCT_P256R1_SERIAL.name, c_ubyte * SERIAL_SIZE),
    (MfgObj.PRODUCT_PUB_P256R1.name, c_ubyte * P256R1_PUB_SIZE),
    (MfgObj.PRODUCT_PUB_P256R1_SIGNATURE.name, c_ubyte * SIG_SIZE),
    (MfgObj.MAN_P256R1_SERIAL.name, c_ubyte * SERIAL_SIZE),
    (MfgObj.MAN_PUB_P256R1.name, c_ubyte * P256R1_PUB_SIZE),
    (MfgObj.MAN_PUB_P256R1_SIGNATURE.name, c_ubyte * SIG_SIZE),
    (MfgObj.SW_P256R1_SERIAL.name, c_ubyte * SERIAL_SIZE),
    (MfgObj.SW_PUB_P256R1.name, c_ubyte * P256R1_PUB_SIZE),
    (MfgObj.SW_PUB_P256R1_SIGNATURE.name, c_ubyte * SIG_SIZE),
    ("", c_ubyte * SERIAL_SIZE), # not used
    (MfgObj.AMZN_PUB_P256R1.name, c_ubyte * P256R1_PUB_SIZE),
    ("", c_ubyte * SIG_SIZE), # not used
  ]

class PrivKey:
  def __init__(self, ka_lifetime, ka_location, ka_usage_flags, ka_bits, ka_algo, ka_type, key_id):
    self._lifetime = ka_lifetime
    self._location = ka_location
    self._usage_flags = ka_usage_flags
    self._bits = ka_bits
    self._algo = ka_algo
    self._type = ka_type
    self._kid = key_id

  @property
  def ka_lifetime(self):
    return self._lifetime

  @property
  def ka_location(self):
    return self._location

  @property
  def ka_usage_flags(self):
    return self._usage_flags

  @property
  def ka_bits(self):
    return self._bits

  @property
  def ka_algo(self):
    return self._algo

  @property
  def ka_type(self):
    return self._type

  @property
  def key_id(self):
    return self._kid

# lifetime, location, usage flags, bits, algo and type values below are derived from PSA crypto module
# definitions in GSDK
sid_priv_keys = {
  MfgObj.DEVICE_PRIV_ED25519.value: PrivKey(0x00000001, 0x00000001, 0x00000400, 0x000000FF, 0x06000800, 0x42, PSA_KEY_ID_ED25519_DEFAULT),
  MfgObj.DEVICE_PRIV_P256R1.value: PrivKey(0x00000001, 0x00000001, 0x00000400, 0x00000100, 0x06000609, 0x12, PSA_KEY_ID_P256R1_DEFAULT)
}

class SidCertBase:
  def __init__(self, cert, cert_is_file):
    if cert_is_file:
      with open(cert, "r") as cert_file:
        self._content = Prodict.from_dict(json.load(cert_file))
    else:
      self._content = Prodict.from_dict(json.loads(cert))

  def apid(self):
    return self._apid

  def get_dynamic_data(self):
    d = dict()
    d[MfgObj.SMSN.value] = self.smsn().lower()
    d[MfgObj.DEVICE_PRIV_ED25519.value] = self.priv_key_ed25519
    d[MfgObj.DEVICE_PUB_ED25519.value] = bytearray(self.ed25519.DEVICE_PUB_ED25519).hex()
    d[MfgObj.DEVICE_PUB_ED25519_SIGNATURE.value] = bytearray(self.ed25519.DEVICE_PUB_ED25519_SIGNATURE).hex()
    d[MfgObj.DEVICE_PRIV_P256R1.value] = self.priv_key_p256r1
    d[MfgObj.DEVICE_PUB_P256R1.value] = bytearray(self.p256r1.DEVICE_PUB_P256R1).hex()
    d[MfgObj.DEVICE_PUB_P256R1_SIGNATURE.value] = bytearray(self.p256r1.DEVICE_PUB_P256R1_SIGNATURE).hex()
    d[MfgObj.DAK_PUB_ED25519.value] = bytearray(self.ed25519.DAK_PUB_ED25519).hex()
    d[MfgObj.DAK_PUB_ED25519_SIGNATURE.value] = bytearray(self.ed25519.DAK_PUB_ED25519_SIGNATURE).hex()
    d[MfgObj.DAK_ED25519_SERIAL.value] = bytearray(self.ed25519.DAK_ED25519_SERIAL).hex()
    d[MfgObj.DAK_PUB_P256R1.value] = bytearray(self.p256r1.DAK_PUB_P256R1).hex()
    d[MfgObj.DAK_PUB_P256R1_SIGNATURE.value] = bytearray(self.p256r1.DAK_PUB_P256R1_SIGNATURE).hex()
    d[MfgObj.DAK_P256R1_SERIAL.value] = bytearray(self.p256r1.DAK_P256R1_SERIAL).hex()
    return d

  def get_static_data(self):
    d = dict()
    d[MfgObj.APP_PUB_ED25519.value] = self._app_srv_pub
    d[MfgObj.DEVICE_PRIV_ED25519.value] = bytearray(PSA_KEY_ID_ED25519_DEFAULT.to_bytes(32, "little")).hex()
    d[MfgObj.DEVICE_PRIV_P256R1.value] = bytearray(PSA_KEY_ID_P256R1_DEFAULT.to_bytes(32, "little")).hex()
    d[MfgObj.PRODUCT_PUB_ED25519.value] = bytearray(self.ed25519.PRODUCT_PUB_ED25519).hex()
    d[MfgObj.PRODUCT_PUB_ED25519_SIGNATURE.value] = bytearray(self.ed25519.PRODUCT_PUB_ED25519_SIGNATURE).hex()
    d[MfgObj.PRODUCT_ED25519_SERIAL.value] = bytearray(self.ed25519.PRODUCT_ED25519_SERIAL).hex()
    d[MfgObj.PRODUCT_PUB_P256R1.value] = bytearray(self.p256r1.PRODUCT_PUB_P256R1).hex()
    d[MfgObj.PRODUCT_PUB_P256R1_SIGNATURE.value] = bytearray(self.p256r1.PRODUCT_PUB_P256R1_SIGNATURE).hex()
    d[MfgObj.PRODUCT_P256R1_SERIAL.value] = bytearray(self.p256r1.PRODUCT_P256R1_SERIAL).hex()
    d[MfgObj.MAN_PUB_ED25519.value] = bytearray(self.ed25519.MAN_PUB_ED25519).hex()
    d[MfgObj.MAN_PUB_ED25519_SIGNATURE.value] = bytearray(self.ed25519.MAN_PUB_ED25519_SIGNATURE).hex()
    d[MfgObj.MAN_ED25519_SERIAL.value] = bytearray(self.ed25519.MAN_ED25519_SERIAL).hex()
    d[MfgObj.MAN_PUB_P256R1.value] = bytearray(self.p256r1.MAN_PUB_P256R1).hex()
    d[MfgObj.MAN_PUB_P256R1_SIGNATURE.value] = bytearray(self.p256r1.MAN_PUB_P256R1_SIGNATURE).hex()
    d[MfgObj.MAN_P256R1_SERIAL.value] = bytearray(self.p256r1.MAN_P256R1_SERIAL).hex()
    d[MfgObj.SW_PUB_ED25519.value] = bytearray(self.ed25519.SW_PUB_ED25519).hex()
    d[MfgObj.SW_PUB_ED25519_SIGNATURE.value] = bytearray(self.ed25519.SW_PUB_ED25519_SIGNATURE).hex()
    d[MfgObj.SW_ED25519_SERIAL.value] = bytearray(self.ed25519.SW_ED25519_SERIAL).hex()
    d[MfgObj.SW_PUB_P256R1.value] = bytearray(self.p256r1.SW_PUB_P256R1).hex()
    d[MfgObj.SW_PUB_P256R1_SIGNATURE.value] = bytearray(self.p256r1.SW_PUB_P256R1_SIGNATURE).hex()
    d[MfgObj.SW_P256R1_SERIAL.value] = bytearray(self.p256r1.SW_P256R1_SERIAL).hex()
    d[MfgObj.AMZN_PUB_ED25519.value] = bytearray(self.ed25519.AMZN_PUB_ED25519).hex()
    d[MfgObj.AMZN_PUB_P256R1.value] = bytearray(self.p256r1.AMZN_PUB_P256R1).hex()
    d[MfgObj.APID.value] = "".join("{:02x}".format(ord(c)) for c in self._apid)
    return d

class SidCertProto(SidCertBase):
  def __init__(self, cert, apid, app_srv_pub, cert_is_file=True):
    super().__init__(cert, cert_is_file)
    self._app_srv_pub = app_srv_pub
    self._apid = apid

  @property
  def ed25519(self):
    for cert in self._content.Sidewalk.DeviceCertificates:
      if cert['SigningAlg'] == 'Ed25519':
        return Cert_ED25519.from_buffer_copy(b64decode(cert['Value']))
    return None

  @property
  def p256r1(self):
    for cert in self._content.Sidewalk.DeviceCertificates:
      if cert['SigningAlg'] == 'P256r1':
        return Cert_P256R1.from_buffer_copy(b64decode(cert['Value']))
    return None

  def smsn(self):
    return self._content.Sidewalk.SidewalkManufacturingSn

  @property
  def priv_key_ed25519(self):
    for key in self._content.Sidewalk.PrivateKeys:
      if key['SigningAlg'] == 'Ed25519':
        return key['Value']
    return None

  @property
  def priv_key_p256r1(self):
    for key in self._content.Sidewalk.PrivateKeys:
      if key['SigningAlg'] == 'P256r1':
        return key['Value']
    return None

class SidCertProdOpenSSL(SidCertBase):
  def __init__(self, cert, cert_is_file=True):
    super().__init__(cert, cert_is_file)
    self._app_srv_pub = self._content.applicationServerPublicKey
    self._apid = self._content.metadata.apid

  @property
  def ed25519(self):
    return Cert_ED25519.from_buffer_copy(b64decode(self._content.eD25519))

  @property
  def p256r1(self):
    return Cert_P256R1.from_buffer_copy(b64decode(self._content.p256R1))

  def smsn(self):
    return self._content.metadata.smsn

  @property
  def priv_key_ed25519(self):
    return self._content.metadata.devicePrivKeyEd25519

  @property
  def priv_key_p256r1(self):
    return self._content.metadata.devicePrivKeyP256R1

class SidCertProdOnDevCertGen(SidCertBase):
  def __init__(self, cert, cert_is_file=True):
    super().__init__(cert, cert_is_file)

  @property
  def ed25519(self):
    return self._content.eD25519

  @property
  def p256r1(self):
    return self._content.p256R1
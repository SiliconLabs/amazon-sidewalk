# !/usr/bin/env python3

from modules.sid_cert import sid_priv_keys, MfgObj
from modules.pdp_api import *
from pdp_mode_base import PDPModeBase

required_args = {
  "dynamic_data"
}

class PrivKeyProv(PDPModeBase):
  _required_kwarg_list = {
    "dynamic_data"
  }

  def _nvm3_store(self, mfg_obj_id, mfg_obj_data):
    self._logger.debug("Storing {0} onto NVM3".format(MfgObj(mfg_obj_id).name))
    return PrivKeyProv_WriteNVM3(
      mfg_obj_id,
      bytearray.fromhex(mfg_obj_data)).serialize()

  def _inject(self, mfg_obj_id, mfg_obj_data):
    self._logger.debug("Injecting {0} into SecureVault".format(MfgObj(mfg_obj_id).name))
    return PrivKeyProv_InjectKey(
      sid_priv_keys[mfg_obj_id].ka_lifetime,
      sid_priv_keys[mfg_obj_id].ka_location,
      sid_priv_keys[mfg_obj_id].ka_usage_flags,
      sid_priv_keys[mfg_obj_id].ka_bits,
      sid_priv_keys[mfg_obj_id].ka_algo,
      sid_priv_keys[mfg_obj_id].ka_type,
      sid_priv_keys[mfg_obj_id].key_id,
      bytearray.fromhex(mfg_obj_data)).serialize()

  _mfg_obj_dispatch = {
    MfgObj.DEVICE_PRIV_ED25519.value:           _inject,
    MfgObj.DEVICE_PRIV_P256R1.value:            _inject,
    MfgObj.SMSN.value:                          _nvm3_store,
    MfgObj.DEVICE_PUB_ED25519.value:            _nvm3_store,
    MfgObj.DEVICE_PUB_ED25519_SIGNATURE.value:  _nvm3_store,
    MfgObj.DEVICE_PUB_P256R1.value:             _nvm3_store,
    MfgObj.DEVICE_PUB_P256R1_SIGNATURE.value:   _nvm3_store,
    MfgObj.DAK_PUB_ED25519.value:               _nvm3_store,
    MfgObj.DAK_PUB_ED25519_SIGNATURE.value:     _nvm3_store,
    MfgObj.DAK_ED25519_SERIAL.value:            _nvm3_store,
    MfgObj.DAK_PUB_P256R1.value:                _nvm3_store,
    MfgObj.DAK_PUB_P256R1_SIGNATURE.value:      _nvm3_store,
    MfgObj.DAK_P256R1_SERIAL.value:             _nvm3_store
  }

  def _provision_dd(self, **kwargs):
    for mfg_obj_id, mfg_obj_data in kwargs['dynamic_data'].items():
      tx_pkt = PrivKeyProv._mfg_obj_dispatch[mfg_obj_id](self, mfg_obj_id, mfg_obj_data)
      self._pdp.comm_send_receive(tx_pkt)
# !/usr/bin/env python3

import base64
import binascii
import subprocess
import shlex
import sys
from modules.sid_cert import SidCertProdOnDevCertGen
from modules.pdp_api import *
from pdp_mode_base import PDPModeBase

SST_CMD = "python3 sidewalk_signing_tool.py -p={0} -c={1} --pin={2} --eddsa_csr={3} --ecdsa_csr={4} --apid={5} --control_log_dir=out"

class OnDevCertGen(PDPModeBase):
  _required_kwarg_list = {
    "dev_type",
    "dsn",
    "apid",
    "app_srv_pub_key",
    "sst_prod_tag",
    "sst_hsm_conn_addr",
    "sst_hsm_pin",
  }

  def _init(self):
    self._logger.debug("Initializing on-device certificate generation")
    tx_pkt = OnDevCertGen_Init().serialize()
    self._pdp.comm_send_receive(tx_pkt)
    self._logger.debug("Done")

  def _generate_smsn(self, dev_type, dsn, apid, board_id):
    self._logger.debug("Generating SMSN")
    tx_pkt = OnDevCertGen_GenSMSN(dev_type, dsn, apid, board_id).serialize()
    rx_pld = self._pdp.comm_send_receive(tx_pkt)
    smsn_len = int.from_bytes(rx_pld[PROTOCOL_DATA_SMSN_LEN_RANGE], "little")
    smsn = bytes(rx_pld[PROTOCOL_DATA_SMSN_VALUE_START_IDX:])
    self._logger.debug("SMSN({0}): {1}".format(smsn_len, binascii.hexlify(smsn)))
    return smsn

  def _generate_csr(self, crypto_curve):
    self._logger.debug("Generating CSR {0}".format(crypto_curve))
    tx_pkt = OnDevCertGen_GenCSR(crypto_curve).serialize()
    rx_pld = self._pdp.comm_send_receive(tx_pkt)
    csr_len = int.from_bytes(rx_pld[PROTOCOL_DATA_CSR_LEN_RANGE], "little")
    csr = bytes(rx_pld[PROTOCOL_DATA_CSR_VALUE_START_IDX:])
    self._logger.debug("CSR({0}): {1}".format(csr_len, binascii.hexlify(csr)))
    return csr

  def _sign_csr(self, sst_prod_tag, sst_hsm_conn_addr, sst_hsm_pin, csr_ed25519, csr_p256r1, apid):
    self._logger.debug("Signing CSRs with sidewalk signing tool")
    csr_ed25519_b64 = base64.b64encode(csr_ed25519).decode("utf-8")
    csr_p256r1_b64 = base64.b64encode(csr_p256r1).decode("utf-8")
    cmd = SST_CMD.format(sst_prod_tag, sst_hsm_conn_addr, sst_hsm_pin, csr_ed25519_b64, csr_p256r1_b64, apid)
    self._logger.debug(cmd)
    try:
      p = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE)
      out_json, err = p.communicate()
      if not err:
        cert = SidCertProdOnDevCertGen(out_json.decode("utf-8"), cert_is_file=False)
        self._logger.debug("ED25519 cert: {0}".format(cert.ed25519))
        self._logger.debug("P256R1 cert: {0}".format(cert.p256r1))
        return base64.b64decode(cert.ed25519), base64.b64decode(cert.p256r1)
      else:
        self._logger.error("Failed to sign the CSRs")
        raise SystemError(err)
    except Exception as e:
      self._logger.error("Sidewalk signing tool execution internal error: {0}".format(e))
      self._logger.error("Possible problems: YubiHSM is not plugged or sidewalk signing tool configuration is not correct or Yubi connector is not running")
      sys.exit(-1)
    return None, None

  def _write_cert_chain(self, crypto_curve, cert):
    self._logger.debug("Writing {0} certificate (len: {1})".format(crypto_curve, len(cert)))
    tx_pkt = OnDevCertGen_WriteCertChain(crypto_curve, cert).serialize()
    self._pdp.comm_send_receive(tx_pkt)
    self._logger.debug("Done")

  def _write_app_srv_pub_key(self, app_srv_pub_key_str):
    app_srv_pub_key = bytearray.fromhex(app_srv_pub_key_str)
    self._logger.debug("Writing application server public key (len: {0}): {1}".format(len(app_srv_pub_key), app_srv_pub_key_str))
    tx_pkt = OnDevCertGen_WriteAppSrvPubKey(app_srv_pub_key).serialize()
    self._pdp.comm_send_receive(tx_pkt)
    self._logger.debug("Done")

  def _store(self):
    self._logger.debug("Finalizing on-device certificate generation")
    tx_pkt = OnDevCertGen_Store().serialize()
    self._pdp.comm_send_receive(tx_pkt)
    self._logger.debug("Done")

  def _provision_dd(self, **kwargs):
    self._init()
    smsn = self._generate_smsn(kwargs['dev_type'], kwargs['dsn'], kwargs['apid'], "")
    csr_ed25519 = self._generate_csr(CryptoCurve.ED25519)
    csr_p256r1 = self._generate_csr(CryptoCurve.P256R1)
    cert_chain_ed25519, cert_chain_p256r1 = self._sign_csr(kwargs['sst_prod_tag'], kwargs['sst_hsm_conn_addr'], kwargs['sst_hsm_pin'], csr_ed25519, csr_p256r1, kwargs['apid'])
    if cert_chain_ed25519 and cert_chain_p256r1:
      self._write_cert_chain(CryptoCurve.ED25519, cert_chain_ed25519)
      self._write_cert_chain(CryptoCurve.P256R1, cert_chain_p256r1)
    self._write_app_srv_pub_key(kwargs['app_srv_pub_key'])
    self._store()
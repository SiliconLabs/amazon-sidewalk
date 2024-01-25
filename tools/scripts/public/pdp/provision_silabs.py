# !/usr/bin/env python3

import argparse
import logging
import json
from enum import Enum
from modules.pdp import PDPMode
from modules.sid_cert import SidCertProto, SidCertProdOpenSSL, SidCertType
from modules.commander import Commander
from modules.part import Part
from priv_key_prov import PrivKeyProv
from on_dev_cert_gen import OnDevCertGen

ERR_MSG_PROVIDED_ARGS_NOT_CONSISTENT = "Provided args are not consistent"

logger = logging.getLogger('provision')
logger.setLevel(logging.DEBUG)
ch = logging.StreamHandler()
ch.setFormatter(logging.Formatter('%(asctime)s %(levelname)s %(message)s'))
logger.addHandler(ch)

argparser = argparse.ArgumentParser(description="Sidewalk PDP provisioning script")
argparser.add_argument("--sid-cert", help="Sidewalk device certificate", type=str)
argparser.add_argument("--sid-cert-type", help="Sidewalk device certificate type", type=str, choices=[SidCertType.PROTOTYPE, SidCertType.PRODUCTION])
argparser.add_argument("--part", help="Part (ie: efr32mg24b220f1536im48)", type=str)
argparser.add_argument("--sid-init-img", help="Sidewalk initialization image", type=str)
argparser.add_argument("--pdp-img", help="Sidewalk PDP application image", type=str)
argparser.add_argument("--pdp-mode", help="PDP mode", type=str, choices=[PDPMode.PRIV_KEY_PROV, PDPMode.ON_DEV_CERT_GEN], required=True)
argparser.add_argument("--dev-type", help="Device type issued by DMS", type=str)
argparser.add_argument("--dsn", help="Device serial number", type=str)
argparser.add_argument("--apid", help="Advertised product ID (base64 encoded string)", type=str, default=None)
argparser.add_argument("--app-srv-pub-key", help="Sidewalk application server public key (hex string)", type=str, default=None)
argparser.add_argument("--sst-prod-tag", help="Sidewalk signing tool production tag", type=str, default=None)
argparser.add_argument("--sst-hsm-conn-addr", help="Sidewalk signing tool yubihsm connector address", type=str, default=None)
argparser.add_argument("--sst-hsm-pin", help="Sidewalk signing tool yubihsm pin", type=str, default=None)
argparser.add_argument("--jlink-ser", help="JLink serial number", type=str)
argparser.add_argument("--prod-config", help="Configuration file for production", type=str)
args = argparser.parse_args()

def load_pdp_img(pdp_img):
  pdp_img_bin = None
  if pdp_img:
    with open(pdp_img, 'rb') as f:
      pdp_img_bin = f.read()
  return pdp_img_bin

def flash_sid_init_img(part, sid_init_img, jlink_ser):
  commander = Commander(jlink_ser)
  logger.info("Wiping device flash memory")
  commander.masserase(part.get_jlink_device())
  logger.info("Resetting device")
  commander.reset(part.get_jlink_device())
  logger.info("Verifying blank")
  commander.verify_blank(part.get_jlink_device())
  logger.info("Flashing sidewalk initialization image")
  commander.flash(sid_init_img, part.get_jlink_device())

def sanity_check_args():
    if args.pdp_mode == PDPMode.PRIV_KEY_PROV:
      if not args.sid_cert_type:
        raise ValueError("Sidewalk certificate type is not provided")
      if not args.part or not args.sid_cert or not args.pdp_img or\
        args.dev_type or args.dsn or args.apid or args.app_srv_pub_key or\
        args.sst_prod_tag or args.sst_hsm_conn_addr or args.sst_hsm_pin or\
        args.prod_config:
        raise ValueError(ERR_MSG_PROVIDED_ARGS_NOT_CONSISTENT)
    else: # on-device cert gen
      if not args.dsn:
        raise ValueError("DSN is not provided")
      if args.prod_config:
        if args.sid_cert or args.sid_cert_type or args.part or\
          args.pdp_img or args.dev_type or\
          args.apid or args.app_srv_pub_key or args.sst_prod_tag or\
          args.sst_hsm_conn_addr or args.sst_hsm_pin or\
          args.pdp_mode == PDPMode.PRIV_KEY_PROV:
          raise ValueError(ERR_MSG_PROVIDED_ARGS_NOT_CONSISTENT)
      else:
        if args.sid_cert or args.sid_cert_type or not args.part or\
           not args.pdp_img or\
           not args.dev_type or not args.apid or not args.app_srv_pub_key or\
           not args.sst_prod_tag or not args.sst_hsm_conn_addr or\
           not args.sst_hsm_pin:
          raise ValueError(ERR_MSG_PROVIDED_ARGS_NOT_CONSISTENT)

def parse_prod_config():
  logger.info("Parsing configuration file for production")
  with open(args.prod_config, "r") as f:
    prod_config = json.load(f)
    args.part               = prod_config["part"]
    args.sid_init_img       = prod_config["sid_init_img"]
    args.pdp_img            = prod_config["pdp_img"]
    args.dev_type           = prod_config["dev_type"]
    args.apid               = prod_config["apid"]
    args.app_srv_pub_key    = prod_config["app_srv_pub_key"]
    args.sst_prod_tag       = prod_config["sst_prod_tag"]
    args.sst_hsm_conn_addr  = prod_config["sst_hsm_conn_addr"]
    args.sst_hsm_pin        = prod_config["sst_hsm_pin"]

if __name__ == "__main__":
  sanity_check_args()
  if args.prod_config:
    parse_prod_config()
  part = Part(args.part)
  if args.sid_init_img:
    flash_sid_init_img(part, args.sid_init_img, args.jlink_ser)
  if args.pdp_mode == PDPMode.PRIV_KEY_PROV:
    sid_cert = None
    logger.info("Parsing sidewalk certificate json file")
    if args.sid_cert_type == SidCertType.PROTOTYPE:
      sid_cert = SidCertProto(args.sid_cert, None, None)
    elif args.sid_cert_type == SidCertType.PRODUCTION:
      sid_cert = SidCertProdOpenSSL(args.sid_cert)
    priv_key_prov = PrivKeyProv(logger, part, args.jlink_ser, load_pdp_img(args.pdp_img))
    priv_key_prov.execute(dynamic_data=sid_cert.get_dynamic_data())
  elif args.pdp_mode == PDPMode.ON_DEV_CERT_GEN:
    on_dev_cert_gen = OnDevCertGen(logger, part, args.jlink_ser, load_pdp_img(args.pdp_img))
    on_dev_cert_gen.execute(dev_type=args.dev_type, dsn=args.dsn, apid=args.apid, app_srv_pub_key=args.app_srv_pub_key, sst_prod_tag=args.sst_prod_tag, sst_hsm_conn_addr=args.sst_hsm_conn_addr, sst_hsm_pin=args.sst_hsm_pin)
  logger.info("Done")
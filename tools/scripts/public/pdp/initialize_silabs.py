# !/usr/bin/env python3

import argparse
import os
import shutil
import logging
from modules.sid_cert import SidCertProto, SidCertProdOpenSSL, SidCertType
from modules.sid_dev_prof import SidDevProf
from modules.commander import Commander
from modules.part import Part
from modules.pdp import PDPMode

TEMP_FOLDER_NAME = "temp/"
OUT_FOLDER_NAME = "out/"
OUTFILE_NAME = OUT_FOLDER_NAME + "sid_init_img.s37"
INITFILE_NAME = TEMP_FOLDER_NAME + "initfile.s37"
MFG_IMAGE_NAME = TEMP_FOLDER_NAME + "mfg.s37"
NVM3_CONTENT_FILENAME = TEMP_FOLDER_NAME + "nvm3content.txt"

logger = logging.getLogger('initialize')
logger.setLevel(logging.DEBUG)
ch = logging.StreamHandler()
ch.setFormatter(logging.Formatter('%(asctime)s %(levelname)s %(message)s'))
logger.addHandler(ch)

argparser = argparse.ArgumentParser(description="Sidewalk PDP initialization script")
argparser.add_argument("--sid-cert", help="Sidewalk device certificate", type=str)
argparser.add_argument("--sid-cert-type", help="Sidewalk device certificate type", type=str, choices=[SidCertType.PROTOTYPE, SidCertType.PRODUCTION])
argparser.add_argument("--sid-dev-prof", help="Sidewalk device profile", type=str)
argparser.add_argument("--part", help="Part (ie: efr32mg24b220f1536im48)", type=str, required=True)
argparser.add_argument("--sid-usr-app-img", help="Sidewalk user application image", type=str, default=None)
argparser.add_argument("--pdp-mode", help="PDP mode", type=str, choices=[PDPMode.PRIV_KEY_PROV, PDPMode.ON_DEV_CERT_GEN], required=True)
args = argparser.parse_args()

def generate_init_img(part, static_data, sid_usr_app_img):
  commander = Commander()
  logger.info("Creating NVM3 init file")
  commander.create_nvm3_initfile(part.get_mfg_page_start_addr(), part.get_mfg_page_size(), part.get_jlink_device(), INITFILE_NAME)
  logger.info("Generating NVM3 content")
  nvm3_content = commander.generate_nvm3_content(static_data)
  logger.info("Saving NVM3 content file")
  with open(NVM3_CONTENT_FILENAME, "w") as f:
    f.write(nvm3_content)
  logger.info("Generating sidewalk static data image")
  commander.set_nvm3(INITFILE_NAME, NVM3_CONTENT_FILENAME, MFG_IMAGE_NAME)
  if sid_usr_app_img:
    logger.info("Merging sidewalk static data image and sidewalk user application image")
  else:
    logger.info("Copying sidewalk static data image into sidewalk initialization image")
  commander.convert(MFG_IMAGE_NAME, sid_usr_app_img, OUTFILE_NAME)

def create_working_folders():
  logger.info("Creating working folders")
  shutil.rmtree(OUT_FOLDER_NAME, ignore_errors=True)
  if not os.path.exists(TEMP_FOLDER_NAME):
    os.mkdir(TEMP_FOLDER_NAME)
  if not os.path.exists(OUT_FOLDER_NAME):
    os.mkdir(OUT_FOLDER_NAME)

def delete_working_folders():
  logger.info("Deleting temporary folders")
  shutil.rmtree(TEMP_FOLDER_NAME)

if __name__ == "__main__":
  logger.info("Starting sidewalk initialization image generation")
  create_working_folders()
  part = Part(args.part)
  sid_usr_app_img = args.sid_usr_app_img
  if args.pdp_mode == PDPMode.PRIV_KEY_PROV:
    sid_cert = None
    logger.info("Parsing sidewalk certificate json file")
    if args.sid_cert_type == SidCertType.PROTOTYPE:
      sid_dev_prof = SidDevProf(args.sid_dev_prof)
      sid_cert = SidCertProto(args.sid_cert, sid_dev_prof.get_apid(), sid_dev_prof.get_app_srv_pub_key())
    elif args.sid_cert_type == SidCertType.PRODUCTION:
      sid_cert = SidCertProdOpenSSL(args.sid_cert)
    generate_init_img(part, sid_cert.get_static_data(), sid_usr_app_img)
  elif args.pdp_mode == PDPMode.ON_DEV_CERT_GEN:
    if sid_usr_app_img:
      logger.info("Copying sidewalk user application into sidewalk initialization image")
      shutil.copyfile(sid_usr_app_img, OUTFILE_NAME)
  delete_working_folders()
  logger.info("Done")

# !/usr/bin/env python3

import json

class SidDevProf:
  def __init__(self, dev_profile):
    self._dev_profile = None
    with open(dev_profile, "r") as dev_profile_file:
        self._dev_profile = json.load(dev_profile_file)

  def get_apid(self):
    return self._dev_profile['Sidewalk']['DakCertificateMetadata'][0]['DeviceTypeId'][-4:]

  def get_app_srv_pub_key(self):
    return self._dev_profile['Sidewalk']['ApplicationServerPublicKey']
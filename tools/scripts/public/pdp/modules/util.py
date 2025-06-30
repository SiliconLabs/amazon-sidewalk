# !/usr/bin/env python3

import os

def sanity_check_file_extension(path, ext_expected):
  _, ext_actual = os.path.splitext(path)
  if ext_actual != ext_expected:
    raise ValueError("{} is not in {} format".format(path, ext_expected))

def sanity_check_file_exists(path):
  if not os.path.isfile(path):
    raise ValueError("{} file does not exist".format(path))
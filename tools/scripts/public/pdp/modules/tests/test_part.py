# !/usr/bin/env python3

import unittest
from part import Part

def assert_str_eq(actual, expected):
  assert actual == expected, f"actual: {actual}, expected: {expected}"

class TestModule(unittest.TestCase):
  def test_parse_part_with_upper_case(self):
    p = Part("EFR32MG24B020F1536IM48")
    assert_str_eq(p.get_mfg_page_start_addr_str(), "0x08178000")
    assert_str_eq(p.get_jlink_device(), "EFR32MG24BxxxF1536")

  def test_parse_part_with_lower_case(self):
    p = Part("efr32mg24b020f1536im48")
    assert_str_eq(p.get_mfg_page_start_addr_str(), "0x08178000")
    assert_str_eq(p.get_jlink_device(), "EFR32MG24BxxxF1536")

  def test_parse_part_with_alternate_caps(self):
    p = Part("eFr32Mg24b020F1536iM48")
    assert_str_eq(p.get_mfg_page_start_addr_str(), "0x08178000")
    assert_str_eq(p.get_jlink_device(), "EFR32MG24BxxxF1536")

  def test_parse_part_without_part_after_capacity(self):
    p = Part("eFr32Mg24b020F1536")
    assert_str_eq(p.get_mfg_page_start_addr_str(), "0x08178000")
    assert_str_eq(p.get_jlink_device(), "EFR32MG24BxxxF1536")

  # following parts may or may not exist

  def test_parse_zg23_512(self):
    p = Part("EFR32ZG23B010F512")
    assert_str_eq(p.get_mfg_page_start_addr_str(), "0x08078000")
    assert_str_eq(p.get_jlink_device(), "EFR32ZG23BxxxF512")

  def test_parse_mg21_1024(self):
    p = Part("EFR32MG21B010F1024")
    assert_str_eq(p.get_mfg_page_start_addr_str(), "0x000f8000")
    assert_str_eq(p.get_jlink_device(), "EFR32MG21BxxxF1024")

  def test_parse_bg21_1024(self):
    p = Part("EFR32BG21B010F1024")
    assert_str_eq(p.get_mfg_page_start_addr_str(), "0x000f8000")
    assert_str_eq(p.get_jlink_device(), "EFR32BG21BxxxF1024")

  def test_parse_mg24_1536(self):
    p = Part("EFR32MG24B020F1536")
    assert_str_eq(p.get_mfg_page_start_addr_str(), "0x08178000")
    assert_str_eq(p.get_jlink_device(), "EFR32MG24BxxxF1536")

  def test_parse_fg25_1920(self):
    p = Part("EFR32FG25B010F1920")
    assert_str_eq(p.get_mfg_page_start_addr_str(), "0x081d8000")
    assert_str_eq(p.get_jlink_device(), "EFR32FG25BxxxF1920")

  def test_parse_zg28_1024(self):
    p = Part("EFR32ZG28B010F1024")
    assert_str_eq(p.get_mfg_page_start_addr_str(), "0x080f8000")
    assert_str_eq(p.get_jlink_device(), "EFR32ZG28BxxxF1024")

  def test_parse_wrong_part(self):
    with self.assertRaises(AttributeError) as exc:
      p = Part("EFM32ZG28B010F1024")

  def test_parse_wrong_family(self):
    with self.assertRaises(KeyError) as exc:
      p = Part("EFR32RG26B010F1024")
      assert_str_eq(p.get_mfg_page_start_addr_str(), "0x080f8000")

if __name__ == '__main__':
  unittest.main()
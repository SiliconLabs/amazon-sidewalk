# Readme

## Common libraries

| Library | BLE | FSK | CSS | Auto Connect | Multilink |
|---------|:---:|:---:|:---:|:------------:|:--------:|
| `sl_lib_ble_subghz_fsk_css.a` | ✓ | ✓ | ✓ | ✓ | ✓ |
| `sl_lib_ble.a` | ✓ | | | ✓ | |
| `sl_lib_subghz_fsk.a` | | ✓ | | | |
| `sl_lib_subghz_fsk_css.a` | | ✓ | ✓ | ✓ | ✓ |
| `sl_lib_qualification_subghz_fsk.a` | | ✓ | | | |
| `sl_lib_qualification_subghz_fsk_css.a` | | ✓ | ✓ | ✓ | ✓ |

*Note: All libraries contain only sidewalk stack without PAL*
* `sl_lib_radio_sx126x.a`: only sx126x radio PAL implementation (includes only lowest-level drivers)

## PDP libraries

* Folder `pdp_sidlib`: Production Device Provisioner library and its dependencies

/***************************************************************************//**
 * @file
 * @brief efr32xgxx_config.h
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 * Your use of this software is governed by the terms of
 * Silicon Labs Master Software License Agreement (MSLA)available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 * This software contains Third Party Software licensed by Silicon Labs from
 * Amazon.com Services LLC and its affiliates and is governed by the sections
 * of the MSLA applicable to Third Party Software and the additional terms set
 * forth in amazon_sidewalk_license.txt.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *  claim that you wrote the original software. If you use this software
 *  in a product, an acknowledgment in the product documentation would be
 *  appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *  misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef EFR32XGXX_CONFIG_H
#define EFR32XGXX_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <sid_pal_radio_ifc.h>
#include "silabs/efr32xgxx.h"
#include <stdint.h>

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define RADIO_REGION_NA SID_PAL_RADIO_RC_NA
#define RADIO_REGION_EU SID_PAL_RADIO_RC_EU
#define RADIO_REGION_NONE SID_PAL_RADIO_RC_NONE

typedef struct radio_efr32xgxx_regional_param {
  uint8_t param_region;
  int8_t max_tx_power[SID_PAL_RADIO_DATA_RATE_MAX_NUM];
  int8_t cca_level_adjust[SID_PAL_RADIO_DATA_RATE_MAX_NUM];
  int16_t ant_dbi;
} radio_efr32xgxx_regional_param_t;

typedef struct radio_efr32xgxx_regional_config {
  uint8_t radio_region;
  uint8_t reg_param_table_size;
  const radio_efr32xgxx_regional_param_t *reg_param_table;
} radio_efr32xgxx_regional_config_t;

typedef struct {
  radio_efr32xgxx_regional_config_t regional_config;
  sid_pal_radio_state_transition_timings_t state_timings;
  RAIL_TxPowerConfig_t tx_power_cfg;
} radio_efr32xgxx_device_config_t;

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
void set_radio_efr32xgxx_device_config(const radio_efr32xgxx_device_config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif /* EFR32XGXX_CONFIG_H */

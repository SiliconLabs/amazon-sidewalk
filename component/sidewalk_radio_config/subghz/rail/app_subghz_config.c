/***************************************************************************//**
 * @file
 * @brief app_subghz_config.c
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
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <efr32xgxx_config.h>
#include <gpio.h>
#include "app_subghz_config.h"
#include <mfg_store_app_values.h>
#ifdef SL_SIDEWALK_ASSISTANT
#include "sl_sidewalk_properties.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/* same tx power applies for all data rates*/
#define RADIO_MAX_TX_POWER_NA           20
#define RADIO_MAX_TX_POWER_EU           14

#ifndef REGION_US915
    #define REGION_US915
#endif

#if defined (REGION_ALL)
#define RADIO_REGION                    RADIO_REGION_NONE
#elif defined (REGION_US915)
#define RADIO_REGION                    RADIO_REGION_NA
#elif defined (REGION_EU868)
#define RADIO_REGION                    RADIO_REGION_EU
#endif

#define RADIO_RX_LNA_GAIN               0
#define RADIO_MAX_CAD_SYMBOL            SID_PAL_RADIO_LORA_CAD_04_SYMBOL
#define RADIO_ANT_GAIN(X)               ((X) * 100)

#define SAR_DCR_CONFIG                  (100)

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static const radio_efr32xgxx_regional_param_t radio_efr32xgxx_regional_param[] =
{
  {
    .param_region = RADIO_REGION_NA,
    #ifdef SL_SIDEWALK_CONFIGURATOR_PROPERTIES_SUBGHZ_OUTPUT_POWER
    #define MAX_TX_POWER SL_SIDEWALK_CONFIGURATOR_PROPERTIES_SUBGHZ_OUTPUT_POWER
    #else
    #define MAX_TX_POWER RADIO_MAX_TX_POWER_NA
    #endif
    .max_tx_power = { MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,
                      MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER },
    .cca_level_adjust = { 0, 0, 0, 0, 0, 0 },
    .ant_dbi = (int16_t)RADIO_ANT_GAIN(2.15)
  },
};

const radio_efr32xgxx_device_config_t radio_efr32xgxx_cfg =
{
  .regional_config = {
    .radio_region         = RADIO_REGION,
    .reg_param_table_size = sizeof(radio_efr32xgxx_regional_param) / sizeof(radio_efr32xgxx_regional_param[0]),
    .reg_param_table      = radio_efr32xgxx_regional_param,
  },
  .state_timings = {
    .sleep_to_full_power_us = 406,
    .full_power_to_sleep_us = 0,
    .rx_to_tx_us = 0,
    .tx_to_rx_us = 0,
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
    .tx_delay_us = 5000,
#endif
  },
};

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

const radio_efr32xgxx_device_config_t * get_radio_cfg(void)
{
  return &radio_efr32xgxx_cfg;
}

struct sid_sub_ghz_links_config sub_ghz_link_config = {
  .enable_link_metrics = true,
  .sar_dcr = SAR_DCR_CONFIG,
  .registration_config = {
    .enable = true,
    .periodicity_s = UINT32_MAX,
  },
#ifdef SL_SIDEWALK_CONFIGURATOR_PROPERTIES_SUBGHZ_OUTPUT_POWER
  .link2_max_tx_power_in_dbm = SL_SIDEWALK_CONFIGURATOR_PROPERTIES_SUBGHZ_OUTPUT_POWER,
  .link3_max_tx_power_in_dbm = SL_SIDEWALK_CONFIGURATOR_PROPERTIES_SUBGHZ_OUTPUT_POWER,
#else
  .link2_max_tx_power_in_dbm = RADIO_MAX_TX_POWER_NA,
  .link3_max_tx_power_in_dbm = RADIO_MAX_TX_POWER_NA,
#endif
};

struct sid_sub_ghz_links_config* app_get_sub_ghz_config(void)
{
  return &sub_ghz_link_config;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

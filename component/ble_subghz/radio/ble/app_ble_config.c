/***************************************************************************//**
 * @file
 * @brief app_ble_config.c
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

#include "app_ble_config.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Array counter
#define countof(array_)                                                                              \
  (1                                                                                                 \
   ? sizeof(array_) / sizeof((array_)[0])                                                            \
   : sizeof(struct { int do_not_use_countof_for_pointers : ((void *)(array_) == (void *)&array_); }) \
  )

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// BLE service config
static const sid_ble_cfg_service_t ble_service =
{
  .type     = AMA_SERVICE,
  .id       =
  {
    .type = UUID_TYPE_16,
    .uu   = { 0xFE, 0x03 },
  },
};

/// BLE descriptor config
static const sid_ble_cfg_descriptor_t ble_desc[] =
{
  {
    .id =
    {
      .type = UUID_TYPE_16,
      .uu   = { 0x29, 0x02 },
    },
    .perm =
    {
      .is_write = true,
    },
  }
};

/// BLE characteristics
static const sid_ble_cfg_characteristics_t ble_characteristics[] =
{
  {
    .id =
    {
      .type = UUID_TYPE_128,
      .uu   = { 0x3C, 0xC5, 0x61, 0xAB, 0x27, 0x04, 0x32, 0x92,
                0x58, 0x4D, 0x6C, 0x7D, 0xC9, 0x96, 0xF9, 0x74 },
    },
    .properties =
    {
      .is_write_no_resp = true,
    },
    .perm =
    {
      .is_write = true,
    },
  },
  {
    .id =
    {
      .type = UUID_TYPE_128,
      .uu   = { 0xFE, 0xD2, 0xF0, 0xE7, 0xB7, 0x53, 0x15, 0x90,
                0xC1, 0x47, 0xCE, 0xFE, 0xC0, 0x83, 0x2E, 0xB3 },
    },
    .properties =
    {
      .is_notify = true,
    },
    .perm =
    {
      .is_none = true,
    },
  },
};

/// BLE advertisement parameters
static const sid_ble_cfg_adv_param_t adv_param =
{
  .type          = AMA_SERVICE,
  .fast_enabled  = true,
  .slow_enabled  = true,
  .fast_interval = 256,
  .fast_timeout  = 3000,
  .slow_interval = 1600,
  .slow_timeout  = 0,
};

/// BLE connection parameters
static const sid_ble_cfg_conn_param_t conn_param =
{
  .min_conn_interval = 16,
  .max_conn_interval = 60,
  .slave_latency     = 0,
  .conn_sup_timeout  = 400,
};

/// BLE GATT profile
static const sid_ble_cfg_gatt_profile_t ble_profile[] =
{
  {
    .service = ble_service,
    .char_count = countof(ble_characteristics),
    .characteristic = ble_characteristics,
    .desc_count = countof(ble_desc),
    .desc = ble_desc,
  },
};

/// BLE configuration
static const sid_ble_config_t ble_cfg =
{
  .name              = "Sl",
  .mtu               = 247,
  .is_adv_available  = true,
  .mac_addr_type     = SID_BLE_CFG_MAC_ADDRESS_TYPE_RANDOM_PRIVATE_RESOLVABLE,
  .adv_param         = adv_param,
  .is_conn_available = true,
  .conn_param        = conn_param,
  .num_profile       = countof(ble_profile),
  .profile           = ble_profile,
};

/// BLE link configuration
static const sid_ble_link_config_t ble_config =
{
  .create_ble_adapter = sid_pal_ble_adapter_create,
  .config             = &ble_cfg,
};

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/******************************************************************************
 * Function to return BLE config.
 *****************************************************************************/
const sid_ble_link_config_t *app_get_ble_config(void)
{
  return &ble_config;
}

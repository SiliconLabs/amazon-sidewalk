/***************************************************************************//**
 * @file
 * @brief ble_adapter.h
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef BLE_ADAPTER_H
#define BLE_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_bt_api.h"
#include "sid_error.h"

#if defined(SL_SIDEWALK_UNIT_TEST)
#include "sid_pal_ble_adapter_ifc_mock.h"
#else
#include "sid_pal_ble_adapter_ifc.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
typedef struct {
  uint16_t current_service_handle;          // The service declaration attribute handle
  uint16_t *current_characteristic_handle;  // The characteristic value attribute handle
  uint16_t *current_descriptor_handle;      // The descriptor attribute handle
} sid_pal_ble_profile_config_t;

typedef struct {
  const sid_ble_config_t *cfg;
  const sid_pal_ble_adapter_callbacks_t *callback;
  uint16_t mtu_size;
  bool is_connected;
  uint16_t conn_id;
  uint8_t bt_addr[BLE_ADDR_MAX_LEN];
} sid_pal_ble_adapter_ctx_t;
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
sid_error_t sid_pal_ble_adapter_create(sid_pal_ble_adapter_interface_t *handle);
void sl_ble_adapter_on_event(sl_bt_msg_t *evt);
void sl_ble_adapter_on_kernel_start(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_ADAPTER_H

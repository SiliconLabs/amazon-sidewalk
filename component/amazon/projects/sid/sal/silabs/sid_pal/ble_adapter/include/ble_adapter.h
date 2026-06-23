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

/**
 * \addtogroup sid_pal
 * @{
 */
/**
 * \addtogroup sidewalk_sdk_ble_adapter
 * @{
 */
/**
 * \addtogroup sidewalk_sdk_ble_adapter_support
 * @{
 */

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @addtogroup sidewalk_sdk_ble_adapter_support_types Type definitions
 * @ingroup sidewalk_sdk_ble_adapter_support
 * @{
 *****************************************************************************/
/**
 * @brief Configuration structure for BLE profile in SID PAL.
 *
 * This structure holds the handles for the current BLE service, characteristic, and descriptor.
 */
typedef struct {
  uint16_t current_service_handle;          /*!< The service declaration attribute handle */
  uint16_t *current_characteristic_handle;  /*!< The characteristic value attribute handle */
  uint16_t *current_descriptor_handle;      /*!< The descriptor attribute handle */
} sid_pal_ble_profile_config_t;

/** @} (end sidewalk_sdk_ble_adapter_support_types) */

/**************************************************************************//**
 * @addtogroup sidewalk_sdk_ble_adapter_support_types Type definitions
 * @ingroup sidewalk_sdk_ble_adapter_support
 * @{
 *****************************************************************************/
/**
 * @brief BLE Adapter Context Structure
 *
 * This structure holds the context information for the BLE adapter.
 */
typedef struct {
  const sid_ble_config_t *cfg;                      /*!< Configuration parameters for the BLE adapter. */
  const sid_pal_ble_adapter_callbacks_t *callback;  /*!< Callback functions for the BLE adapter. */
  sid_ble_cfg_adv_param_t current_adv_config;       /*!< Current advertising configuration parameters. */
  sid_ble_cfg_conn_param_t current_conn_config;     /*!< Current connection configuration parameters. */
  sid_ble_cfg_conn_param_t last_conn_config;        /*!< Last connection configuration parameters. */
  uint8_t bt_addr[BLE_ADDR_MAX_LEN];                /*!< Bluetooth address. */
  uint16_t mtu_size;                                /*!< Maximum Transmission Unit (MTU) size. */
  uint16_t conn_id;                                 /*!< Connection identifier. */
  bool is_connected;                                /*!< Connection status flag. */
} sid_pal_ble_adapter_ctx_t;

/** @} (end sidewalk_sdk_ble_adapter_support_types) */
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/**
 * @brief Creates a BLE adapter interface.
 *
 * This function initializes the BLE adapter interface and assigns it to the provided handle.
 *
 * @param[in,out] handle Pointer to the BLE adapter interface handle.
 * @return sid_error_t Error code indicating the result of the operation.
 */
sid_error_t sid_pal_ble_adapter_create(sid_pal_ble_adapter_interface_t *handle);

/**
 * @brief Handles BLE events.
 *
 * This function processes BLE events received from the Bluetooth stack.
 *
 * @param[in] evt Pointer to the Bluetooth event message.
 */
void sl_ble_adapter_on_event(sl_bt_msg_t *evt);

/**
 * @brief Callback for kernel start event.
 *
 * This function is called when the kernel starts.
 */
void sl_ble_adapter_on_kernel_start(void);

/**
 * @brief This function returns the MAC address of the BLE advertiser.
 *
 * This MAC address is used during BLE advertising.
 *
 * @param[out] addr Pointer to the buffer where the MAC address will be stored.
 * @return sid_error_t Error code indicating the result of the operation.
 */
sid_error_t ble_adapter_get_advertiser_address(uint8_t *addr);

/**
 * @brief This function returns the MAC address of the BLE connection.
 *
 * This MAC address is used during BLE connection.
 *
 * @param[out] addr Pointer to the buffer where the MAC address will be stored.
 * @return sid_error_t Error code indicating the result of the operation.
 */
sid_error_t ble_adapter_get_connection_address(uint8_t *addr);

#ifdef __cplusplus
}
#endif

#endif // BLE_ADAPTER_H
/** @} */ // end of sid_pal group
/** @} */ // end of sidewalk_sdk_ble_adapter group
/** @} */ // end of sidewalk_sdk_ble_adapter_support group

/***************************************************************************//**
 * @file
 * @brief ble_adapter.c
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

//========================================================
//===== BLE adapter only supports legacy advertising =====
//========================================================

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include <sid_pal_ble_adapter_ifc.h>
#include <sid_ble_config_ifc.h>
#include <sid_pal_log_ifc.h>
#include "ble_adapter.h"
#include "sl_bt_api.h"
#include "sl_bluetooth_config.h"
#include "sl_malloc.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define BLE_NOTIFY_LENGTH                               (2)
#define BLE_NOTIFICATION_ENABLED                        (1)
#define BLE_COMPANY_ID_BYTE_LENGTH                      (2)
#define BLE_COMPANY_ID                                  (0x0171)            // Amazon.com Services LLC
#define SL_BT_DEFAULT_SM_CONFIGURE_FLAGS                ((uint8_t) 0x0E)
#define SL_BT_HAL_SM_MAX_BONDING_COUNT                  ((uint8_t) 0x08)
#define SL_BT_HAL_SM_POLICY_FLAGS                       ((uint8_t) 0x02)
#define UUID_LEN_16BIT                                  (2)
#define UUID_LEN_32BIT                                  (4)
#define UUID_LEN_128BIT                                 (16)
#define SL_BT_PERM_NONE                                 (0)
#define SL_BT_PERM_READ                                 (1)
#define SL_BT_PERM_WRITE                                (16)
#define SL_BT_PERM_READ_ENCRYPTED                       (2)
#define SL_BT_PERM_READ_ENCRYPTED_MITM                  (4)
#define SL_BT_PERM_WRITE_ENCRYPTED                      (32)
#define SL_BT_PERM_WRITE_ENCRYPTED_MITM                 (64)
#define SL_BT_ADDR_TYPE_BYTE_INDEX                      (5)
#define SL_BT_MAX_LEGACY_ADV_DATA_LEN                   (31)
#define SL_BT_ADDR_TYPE_MASK                            ((uint8_t) 0xC0)
#define SL_BT_ADDR_TYPE_NON_RESOLVABLE_PRIVATE          ((uint8_t) 0x00)
#define SL_BT_ADDR_TYPE_STATIC_RANDOM                   ((uint8_t) 0xC0)
#define SL_BT_CHANNEL_MAP                               ((uint8_t) 7)
#define SL_BT_ADV_FLAG_GENERAL_DISCOVERABLE             ((uint8_t) 0x02)
#define SL_BT_ADV_FLAG_BR_EDR_NOT_SUPPORTED             ((uint8_t) 0x04)
#define SL_BT_ADV_DATA_TYPE_FLAGS                       ((uint8_t) 0x01)
#define SL_BT_ADV_DATA_TYPE_COMPLETE_16BIT_UUIDS        ((uint8_t) 0x03)
#define SL_BT_ADV_DATA_TYPE_COMPLETE_LOCAL_NAME         ((uint8_t) 0x09)
#define SL_BT_ADV_DATA_TYPE_MANUFACTURER_DATA           ((uint8_t) 0xFF)
#define SL_BT_GATTS_TRAN_TYPE_INVALID                   ((uint32_t) 0x00)
#define SL_BT_GATTS_TRAN_TYPE_READ                      ((uint32_t) 0x01)
#define SL_BT_GATTS_TRAN_TYPE_WRITE                     ((uint32_t) 0x02)
#define SL_BT_GATTS_TRAN_TYPE_PREP_WRITE                ((uint32_t) 0x03)

// Macro to set a uint16_t data item to advertisement data
#define SL_BT_PRV_SET_ADV_DATA_UINT16(ptr, value) \
  do {                                            \
    uint16_t _value = (value);                    \
    (ptr)[0] = (uint8_t) (_value & 0xFF);         \
    (ptr)[1] = (uint8_t) (_value >> 8);           \
  } while (0)

typedef struct {
  const sid_ble_config_t *cfg;
  const sid_pal_ble_adapter_callbacks_t *callback;
  uint16_t mtu_size;
  bool is_connected;
  uint16_t conn_id;
  uint8_t bt_addr[BLE_ADDR_MAX_LEN];
} sid_pal_ble_adapter_ctx_t;

typedef struct {
  uint16_t current_service_handle;          // The service declaration attribute handle
  uint16_t *current_characteristic_handle;  // The characteristic value attribute handle
  uint16_t *current_descriptor_handle;      // The descriptor attribute handle
} sid_pal_ble_profile_config_t;

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
// Callback functions
static void ble_connection_cb_fnc(uint16_t conn_id,
                                  bool connected,
                                  bd_addr *bt_addr);
static void ble_request_write_cb_fnc(uint16_t conn_id,
                                     uint32_t trans_id,
                                     bd_addr *bt_addr,
                                     uint16_t attr_handle,
                                     uint16_t offset,
                                     size_t length,
                                     bool need_resp,
                                     bool is_prep,
                                     uint8_t *data);
// BLE adapter interface functions
static sid_error_t ble_adapter_init(const sid_ble_config_t *cfg);
static sid_error_t ble_adapter_start_service(void);
static sid_error_t ble_adapter_set_adv_data(uint8_t *data, uint8_t length);
static sid_error_t ble_adapter_start_advertisement(void);
static sid_error_t ble_adapter_stop_advertisement(void);
static sid_error_t ble_adapter_send_data(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length);
static sid_error_t ble_adapter_set_callback(const sid_pal_ble_adapter_callbacks_t *cb);
static sid_error_t ble_adapter_disconnect(void);
static sid_error_t ble_adapter_deinit(void);
// BLE event handlers
static void sl_ble_adapter_on_system_boot(sl_bt_evt_system_boot_t *event);
static void sl_ble_adapter_on_advertiser_timeout(sl_bt_evt_advertiser_timeout_t *event);
static void sl_ble_adapter_on_connection_opened(sl_bt_evt_connection_opened_t *event);
static void sl_ble_adapter_on_connection_closed(sl_bt_evt_connection_closed_t *event);
static void sl_ble_adapter_on_gatt_server_characteristic_status_id(sl_bt_evt_gatt_server_characteristic_status_t *event);
static void sl_ble_adapter_on_gatt_server_indication_timeout_id(sl_bt_evt_gatt_server_indication_timeout_t *event);
static void sl_ble_adapter_on_gatt_server_user_write_request_id(sl_bt_evt_gatt_server_user_write_request_t *event);
static void sl_ble_adapter_on_gatt_mtu_exchanged_id(sl_bt_evt_gatt_mtu_exchanged_t *event);
// Helper functions
static void sl_ble_init_failed(const char *msg);
static void sl_ble_free_resources();
static void sl_ble_abort_session(const char *msg, uint16_t session);
static uint16_t sl_ble_evaluate_permissions(uint16_t xPermissions);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
static struct sid_pal_ble_adapter_interface ble_ifc =
{
  .init          = ble_adapter_init,
  .start_service = ble_adapter_start_service,
  .set_adv_data  = ble_adapter_set_adv_data,
  .start_adv     = ble_adapter_start_advertisement,
  .stop_adv      = ble_adapter_stop_advertisement,
  .send          = ble_adapter_send_data,
  .set_callback  = ble_adapter_set_callback,
  .disconnect    = ble_adapter_disconnect,
  .deinit        = ble_adapter_deinit,
};

// Current adapter context
static sid_pal_ble_adapter_ctx_t ctx;
// BLE profile
static sid_pal_ble_profile_config_t *ble_profile = NULL;
// Advertising parameters
static sid_ble_cfg_adv_param_t adv_timing_params;

// Indicate whether BLE stack is started
static bool is_bluetooth_started = false;
// Indicate whether kernel is started
static bool is_kernel_started = false;
// Indicate whether BLE advertising is active
static bool is_adv_active = false;
// Indicate whether BLE advertising is slow or fast
static bool is_fast_adv_active = true;
// The advertising set handle allocated from Bluetooth stack
static uint8_t advertising_set_handle = SL_BT_INVALID_ADVERTISING_SET_HANDLE;

// Static random address used for advertisers
// This static random Bluetooth address is used by all advertisers that specify the
// 'BTAddrTypeStaticRandom' address type in their configuration. The address is generated when it is
// first needed and remains unchanged until device reboot.
static bd_addr adv_static_random_addr = { 0 };
static bool have_adv_static_random_addr = false;

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
sid_error_t sid_pal_ble_adapter_create(sid_pal_ble_adapter_interface_t *handle);

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void sl_ble_adapter_on_event(sl_bt_msg_t *evt)
{
  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_system_boot_id:
      sl_ble_adapter_on_system_boot(&evt->data.evt_system_boot);
      break;

    case sl_bt_evt_advertiser_timeout_id:
      sl_ble_adapter_on_advertiser_timeout(&evt->data.evt_advertiser_timeout);

      // Switch to slow advertisement
      is_fast_adv_active = false;
      adv_timing_params.fast_interval = ctx.cfg->adv_param.slow_interval;
      adv_timing_params.fast_timeout = ctx.cfg->adv_param.slow_timeout;
      ctx.callback->adv_start_callback();
      break;

    case sl_bt_evt_connection_opened_id:
      sl_ble_adapter_on_connection_opened(&evt->data.evt_connection_opened);
      break;

    case sl_bt_evt_connection_closed_id:
      sl_ble_adapter_on_connection_closed(&evt->data.evt_connection_closed);

      // Switch back to fast advertisement
      is_fast_adv_active = true;
      adv_timing_params.fast_interval = ctx.cfg->adv_param.fast_interval;
      adv_timing_params.fast_timeout = ctx.cfg->adv_param.fast_timeout;
      break;

    case sl_bt_evt_gatt_server_characteristic_status_id:
      sl_ble_adapter_on_gatt_server_characteristic_status_id(&evt->data.evt_gatt_server_characteristic_status);
      break;

    case sl_bt_evt_gatt_server_indication_timeout_id:
      sl_ble_adapter_on_gatt_server_indication_timeout_id(&evt->data.evt_gatt_server_indication_timeout);
      break;

    case sl_bt_evt_gatt_server_user_write_request_id:
      sl_ble_adapter_on_gatt_server_user_write_request_id(&evt->data.evt_gatt_server_user_write_request);
      break;

    case sl_bt_evt_gatt_mtu_exchanged_id:
      sl_ble_adapter_on_gatt_mtu_exchanged_id(&evt->data.evt_gatt_mtu_exchanged);
      break;

    default:
      // Other events are ignored
      break;
  }
}

// Function called by platform init when the RTOS kernel is started
void sl_ble_adapter_on_kernel_start(void)
{
  is_kernel_started = true;
}

sid_error_t sid_pal_ble_adapter_create(sid_pal_ble_adapter_interface_t *handle)
{
  if (!handle) {
    return SID_ERROR_INVALID_ARGS;
  }

  *handle = &ble_ifc;

  return SID_ERROR_NONE;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static void ble_connection_cb_fnc(uint16_t conn_id,
                                  bool connected,
                                  bd_addr *bt_addr)
{
  if (bt_addr != NULL) {
    SID_PAL_LOG_INFO("Sidewalk BLE State: %sCONNECTED\n", connected ? "" : "DIS");
    ctx.conn_id = conn_id;
    ctx.is_connected = connected;
    memcpy(ctx.bt_addr, bt_addr->addr, BLE_ADDR_MAX_LEN);
    ctx.callback->conn_callback(ctx.is_connected, ctx.bt_addr);
  }
}

static void ble_request_write_cb_fnc(uint16_t conn_id,
                                     uint32_t trans_id,
                                     bd_addr *bt_addr,
                                     uint16_t attr_handle,
                                     uint16_t offset,
                                     size_t length,
                                     bool need_resp,
                                     bool is_prep,
                                     uint8_t *data)
{
  // Unused parameter
  (void)bt_addr;
  (void)is_prep;

  if (data != NULL) {
    sid_ble_cfg_service_identifier_t id;
    for (uint8_t i = 0; i < ctx.cfg->num_profile; i++) {
      id = ctx.cfg->profile[i].service.type;
      for (uint8_t j = 0; j < ctx.cfg->profile[i].char_count; j++) {
        if (attr_handle == ble_profile[i].current_characteristic_handle[j]) {
          ctx.callback->data_callback(id, data, length);
        }
      }
      for (uint8_t j = 0; j < ctx.cfg->profile[i].desc_count; j++) {
        if (attr_handle == ble_profile[i].current_descriptor_handle[j]) {
          if (length == BLE_NOTIFY_LENGTH) {
            uint16_t notif_data;
            memcpy(&notif_data, data, sizeof(notif_data));
            ctx.callback->notify_callback(id, (notif_data == BLE_NOTIFICATION_ENABLED));
          }
        }
      }

      if (need_resp && conn_id) {
        // Send a response to a read/write operation
        switch (trans_id) {
          case SL_BT_GATTS_TRAN_TYPE_WRITE:
          {
            // Send response to remote
            (void)sl_bt_gatt_server_send_user_write_response(conn_id, attr_handle, 0);
            break;
          }

          case SL_BT_GATTS_TRAN_TYPE_PREP_WRITE:
          {
            // Send response to remote
            sl_bt_gatt_server_send_user_prepare_write_response(conn_id, attr_handle, 0, offset, length, data);
            break;
          }

          case SL_BT_GATTS_TRAN_TYPE_READ:
          {
            uint16_t sent_len;

            // Check MTU size
            uint16_t rsp_val_len;
            if (sl_bt_gatt_server_get_mtu(conn_id, &rsp_val_len) != SL_STATUS_OK) {
              break;
            }
            // Compare MTU and the length of the unsent Attribute value
            if (rsp_val_len > length) {
              rsp_val_len = length;
            }
            // Send response to remote
            (void)sl_bt_gatt_server_send_user_read_response(conn_id, attr_handle, 0, rsp_val_len, data, &sent_len);
            break;
          }

          default:
            // Nothing to do
            break;
        }
      }
      return;
    }
  }
}

static void sl_ble_init_failed(const char *msg)
{
  // When failed, stop the Bluetooth stack to avoid getting into a partially initialized state
  (void)sl_bt_system_stop_bluetooth();
  is_bluetooth_started = false;
  if (msg != NULL) {
    SID_PAL_LOG_ERROR(msg);
  }
}

static void sl_ble_free_resources()
{
  for (uint8_t i = 0; i < ctx.cfg->num_profile; i++) {
    if ((ble_profile != NULL) && (ble_profile[i].current_descriptor_handle != NULL)) {
      sl_free(ble_profile[i].current_descriptor_handle);
      ble_profile[i].current_descriptor_handle = NULL;
    }
    if ((ble_profile != NULL) && (ble_profile[i].current_characteristic_handle != NULL)) {
      sl_free(ble_profile[i].current_characteristic_handle);
      ble_profile[i].current_characteristic_handle = NULL;
    }

    ble_profile[i].current_service_handle = 0;
  }

  if (ble_profile != NULL) {
    sl_free(ble_profile);
    ble_profile = NULL;
  }
}

static void sl_ble_abort_session(const char *msg, uint16_t session)
{
  // Cancel all changes performed in current session and close the session
  (void)sl_bt_gattdb_abort(session);
  SID_PAL_LOG_ERROR(msg);
}

static uint16_t sl_ble_evaluate_permissions(uint16_t xPermissions)
{
  uint16_t retVal = 0;

  if (xPermissions & SL_BT_PERM_READ_ENCRYPTED) {
    retVal |= SL_BT_GATTDB_ENCRYPTED_READ;
  }
  if (xPermissions & SL_BT_PERM_READ_ENCRYPTED_MITM) {
    retVal |= SL_BT_GATTDB_AUTHENTICATED_READ;
  }
  if (xPermissions & SL_BT_PERM_WRITE_ENCRYPTED) {
    retVal |= SL_BT_GATTDB_ENCRYPTED_WRITE;
  }
  if (xPermissions & SL_BT_PERM_WRITE_ENCRYPTED_MITM) {
    retVal |= SL_BT_GATTDB_AUTHENTICATED_WRITE;
  }

  return retVal;
}

static sid_error_t ble_adapter_init(const sid_ble_config_t *cfg)
{
  if (!cfg) {
    SID_PAL_LOG_ERROR("Missing Sidewalk BLE configuration");
    is_bluetooth_started = false;
    return SID_ERROR_INVALID_ARGS;
  }

  SID_PAL_LOG_INFO("Sidewalk BLE initialization is in progress");

  // Save BLE configuration
  ctx.cfg = cfg;
  ctx.mtu_size = cfg->mtu;

  // Allocate memory dinamically for BLE profile
  ble_profile = (sid_pal_ble_profile_config_t *)sl_malloc(ctx.cfg->num_profile * sizeof(sid_pal_ble_profile_config_t));
  if (!ble_profile) {
    SID_PAL_LOG_ERROR("Memory allocation failed for Sidewalk BLE profile");
    return SID_ERROR_GENERIC;
  } else {
    // Allocate memory dinamically for BLE characteristics and descriptors based on config file
    for (uint8_t i = 0; i < ctx.cfg->num_profile; i++) {
      ble_profile[i].current_service_handle = 0;
      ble_profile[i].current_characteristic_handle = (uint16_t *)sl_malloc(ctx.cfg->profile[i].char_count * sizeof(uint16_t));
      if (!ble_profile[i].current_characteristic_handle) {
        sl_free(ble_profile);
        ble_profile = NULL;
        SID_PAL_LOG_ERROR("Memory allocation failed for Sidewalk BLE characteristic");
        return SID_ERROR_GENERIC;
      } else {
        ble_profile[i].current_descriptor_handle = (uint16_t *)sl_malloc(ctx.cfg->profile[i].desc_count * sizeof(uint16_t));
        if (!ble_profile[i].current_descriptor_handle) {
          sl_free(ble_profile[i].current_characteristic_handle);
          ble_profile[i].current_characteristic_handle = NULL;
          sl_free(ble_profile);
          ble_profile = NULL;
          SID_PAL_LOG_ERROR("Memory allocation failed for Sidewalk BLE descriptor");
          return SID_ERROR_GENERIC;
        }
      }
    }
  }

  if (is_fast_adv_active) {
    memcpy(&adv_timing_params, &cfg->adv_param, sizeof(sid_ble_cfg_adv_param_t));
  }

  sl_status_t sl_status = SL_STATUS_FAIL;

  // Request the stack to start
  if (!is_bluetooth_started && is_kernel_started) {
    sl_status = sl_bt_system_start_bluetooth();
  } else {
    SID_PAL_LOG_ERROR("Sidewalk BLE start request is failed due to:%s%s",
                      is_bluetooth_started ? " Sidewalk BLE stack has already started." : "",
                      !is_kernel_started ? " Kernel has not started yet." : "");
    is_bluetooth_started = false;
    return SID_ERROR_GENERIC;
  }

  // If the start request was successful, we use sl_bt_system_get_random_data() to check for readiness
  if (sl_status == SL_STATUS_OK) {
    uint8_t DummyData = 0;
    size_t OutputDataLen = 0;
    sl_status = sl_bt_system_get_random_data(sizeof(DummyData), sizeof(DummyData), &OutputDataLen, &DummyData);
  } else {
    SID_PAL_LOG_ERROR("Sidewalk BLE stack is not ready");
    is_bluetooth_started = false;
    return SID_ERROR_GENERIC;
  }

  // If successful, the Bluetooth stack has started up
  if (sl_status == SL_STATUS_OK) {
    // Set default max MTU
    if (sl_bt_gatt_set_max_mtu(ctx.mtu_size, &ctx.mtu_size) != SL_STATUS_OK) {
      sl_ble_free_resources();
      sl_ble_init_failed("Set default max MTU failed");
      return SID_ERROR_GENERIC;
    }

    // Set default bondable mode (bonding is disabled)
    if (sl_bt_sm_set_bondable_mode(0) != SL_STATUS_OK) {
      sl_ble_free_resources();
      sl_ble_init_failed("Set default bondable mode failed");
      return SID_ERROR_GENERIC;
    }

    // Set security manager configuration
    if (sl_bt_sm_configure(SL_BT_DEFAULT_SM_CONFIGURE_FLAGS, sm_io_capability_noinputnooutput) != SL_STATUS_OK) {
      sl_ble_free_resources();
      sl_ble_init_failed("Set security manager configuration failed");
      return SID_ERROR_GENERIC;
    }

    // Store bonding configuration
    if (sl_bt_sm_store_bonding_configuration(SL_BT_HAL_SM_MAX_BONDING_COUNT, SL_BT_HAL_SM_POLICY_FLAGS) != SL_STATUS_OK) {
      sl_ble_free_resources();
      sl_ble_init_failed("Store bonding configuration failed");
      return SID_ERROR_GENERIC;
    }

    // Set the global maximum TX power to the highest power that advertising can use
    int16_t tx_power_min = 0;
    int16_t tx_power_max = 0;
    if (sl_bt_system_set_tx_power(SL_BT_CONFIG_MIN_TX_POWER, SL_BT_CONFIG_MAX_TX_POWER,
                                  &tx_power_min, &tx_power_max) != SL_STATUS_OK) {
      sl_ble_free_resources();
      sl_ble_init_failed("Set the global maximum TX power failed");
      return SID_ERROR_GENERIC;
    }

    is_bluetooth_started = true;
  } else {
    sl_ble_free_resources();
    sl_ble_init_failed("Sidewalk BLE stack has failed to start");
    return SID_ERROR_GENERIC;
  }

  SID_PAL_LOG_INFO("Sidewalk BLE initialization is successfully completed");

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_start_service(void)
{
  if (!ctx.cfg->num_profile) {
    sl_ble_free_resources();
    return SID_ERROR_INVALID_ARGS;
  }

  // Session handle
  uint16_t gattdb_session_id = 0;

  // Return status value
  sl_status_t sl_status = SL_STATUS_OK;

  // *********** Create a new service ***********
  for (uint8_t i = 0; i < ctx.cfg->num_profile; i++) {
    // Start a new GATT database update session
    sl_status = sl_bt_gattdb_new_session(&gattdb_session_id);

    if (sl_status == SL_STATUS_OK) {
      size_t uuid_len = 0;
      switch (ctx.cfg->profile[i].service.id.type) {
        case UUID_TYPE_16:
        {
          uuid_len = UUID_LEN_16BIT;
          uint8_t uuid_little_endian[UUID_LEN_16BIT];
          memset(uuid_little_endian, 0, UUID_LEN_16BIT);
          // Little endian conversion
          for (uint8_t idx = 0; idx < UUID_LEN_16BIT; idx++) {
            uuid_little_endian[idx] = ctx.cfg->profile[i].service.id.uu[(UUID_LEN_16BIT - idx) - 1];
          }
          // Add a service into the local GATT database
          sl_status = sl_bt_gattdb_add_service(gattdb_session_id, sl_bt_gattdb_primary_service, 0,
                                               uuid_len, uuid_little_endian, &ble_profile[i].current_service_handle);
          break;
        }

        case UUID_TYPE_32:
        {
          uuid_len = UUID_LEN_32BIT;
          uint8_t uuid_little_endian[UUID_LEN_32BIT];
          memset(uuid_little_endian, 0, UUID_LEN_32BIT);
          // Little endian conversion
          for (uint8_t idx = 0; idx < UUID_LEN_32BIT; idx++) {
            uuid_little_endian[idx] = ctx.cfg->profile[i].service.id.uu[(UUID_LEN_32BIT - idx) - 1];
          }
          // Add a service into the local GATT database
          sl_status = sl_bt_gattdb_add_service(gattdb_session_id, sl_bt_gattdb_primary_service, 0,
                                               uuid_len, uuid_little_endian, &ble_profile[i].current_service_handle);
          break;
        }

        case UUID_TYPE_128:
        {
          uuid_len = UUID_LEN_128BIT;
          uint8_t uuid_little_endian[UUID_LEN_128BIT];
          memset(uuid_little_endian, 0, UUID_LEN_128BIT);
          // Little endian conversion
          for (uint8_t idx = 0; idx < UUID_LEN_128BIT; idx++) {
            uuid_little_endian[idx] = ctx.cfg->profile[i].service.id.uu[(UUID_LEN_128BIT - idx) - 1];
          }
          // Add a service into the local GATT database
          sl_status = sl_bt_gattdb_add_service(gattdb_session_id, sl_bt_gattdb_primary_service, 0,
                                               uuid_len, uuid_little_endian, &ble_profile[i].current_service_handle);
          break;
        }

        default:
        {
          sl_ble_free_resources();
          sl_ble_abort_session("Invalid service UUID type", gattdb_session_id);
          return SID_ERROR_GENERIC;
          break;
        }
      }
    } else {
      sl_ble_free_resources();
      sl_ble_abort_session("Start a new GATT database update session for service failed", gattdb_session_id);
      return SID_ERROR_GENERIC;
    }

    if (sl_status == SL_STATUS_OK) {
      // Save all changes performed in current session and close the session
      if (sl_bt_gattdb_commit(gattdb_session_id) != SL_STATUS_OK) {
        sl_ble_free_resources();
        SID_PAL_LOG_ERROR("Save all changes performed in current session and close the session failed");
        return SID_ERROR_GENERIC;
      }
    } else {
      sl_ble_free_resources();
      sl_ble_abort_session("Add a service into the local GATT database failed", gattdb_session_id);
      return SID_ERROR_GENERIC;
    }

    // *********** Fill up Characteristic properties ***********
    for (uint8_t j = 0; j < ctx.cfg->profile[i].char_count; j++) {
      // Start a new GATT database update session
      sl_status = sl_bt_gattdb_new_session(&gattdb_session_id);

      if (sl_status == SL_STATUS_OK) {
        uint16_t properties = 0;
        if (ctx.cfg->profile[i].characteristic[j].properties.is_notify) {
          properties |= SL_BT_GATTDB_CHARACTERISTIC_NOTIFY;
        }
        if (ctx.cfg->profile[i].characteristic[j].properties.is_read) {
          properties |= SL_BT_GATTDB_CHARACTERISTIC_READ;
        }
        if (ctx.cfg->profile[i].characteristic[j].properties.is_write) {
          properties |= SL_BT_GATTDB_CHARACTERISTIC_WRITE;
        }
        if (ctx.cfg->profile[i].characteristic[j].properties.is_write_no_resp) {
          properties |= SL_BT_GATTDB_CHARACTERISTIC_WRITE_NO_RESPONSE;
        }

        uint16_t xPermissions = 0;
        if (ctx.cfg->profile[i].characteristic[j].perm.is_none) {
          xPermissions |= SL_BT_PERM_NONE;
        }
        if (ctx.cfg->profile[i].characteristic[j].perm.is_read) {
          xPermissions |= SL_BT_PERM_READ;
        }
        if (ctx.cfg->profile[i].characteristic[j].perm.is_write) {
          xPermissions |= SL_BT_PERM_WRITE;
        }

        uint16_t permissions = sl_ble_evaluate_permissions(xPermissions);

        if (ctx.cfg->profile[i].characteristic[j].id.type == UUID_TYPE_16) {
          // 16-bit uuid
          sl_bt_uuid_16_t uuid_little_endian;
          // Little endian conversion
          for (uint8_t idx = 0; idx < UUID_LEN_16BIT; idx++) {
            uuid_little_endian.data[idx] = ctx.cfg->profile[i].characteristic[j].id.uu[(UUID_LEN_16BIT - idx) - 1];
          }
          // Add a 16-bits UUID characteristic to a service
          sl_status = sl_bt_gattdb_add_uuid16_characteristic(gattdb_session_id,
                                                             ble_profile[i].current_service_handle,
                                                             properties,
                                                             permissions,
                                                             SL_BT_GATTDB_NO_AUTO_CCCD, // Do not create client-config automatically
                                                             uuid_little_endian,
                                                             sl_bt_gattdb_user_managed_value,
                                                             0, 0, NULL,  // Ignored parameters when value type is user_managed
                                                             &ble_profile[i].current_characteristic_handle[j]);
        } else if (ctx.cfg->profile[i].characteristic[j].id.type == UUID_TYPE_128) {
          // 128-bit uuid
          uuid_128 uuid_little_endian;
          // Little endian conversion (config is already in little endian format)
          for (uint8_t idx = 0; idx < UUID_LEN_128BIT; idx++) {
            uuid_little_endian.data[idx] = ctx.cfg->profile[i].characteristic[j].id.uu[idx];
          }
          // Add a 128-bits UUID characteristic to a service
          sl_status = sl_bt_gattdb_add_uuid128_characteristic(gattdb_session_id,
                                                              ble_profile[i].current_service_handle,
                                                              properties,
                                                              permissions,
                                                              SL_BT_GATTDB_NO_AUTO_CCCD,  // Do not create client-config automatically
                                                              uuid_little_endian,
                                                              sl_bt_gattdb_user_managed_value,
                                                              0, 0, NULL, // Ignored parameters when value type is user_managed
                                                              &ble_profile[i].current_characteristic_handle[j]);
        } else {
          sl_ble_free_resources();
          sl_ble_abort_session("Invalid characteristic UUID type", gattdb_session_id);
          return SID_ERROR_GENERIC;
        }

        if (sl_status == SL_STATUS_OK) {
          // Save all changes performed in current session and close the session
          if (sl_bt_gattdb_commit(gattdb_session_id) != SL_STATUS_OK) {
            sl_ble_free_resources();
            SID_PAL_LOG_ERROR("Save all changes performed in current session and close the session failed");
            return SID_ERROR_GENERIC;
          }
        } else {
          sl_ble_free_resources();
          sl_ble_abort_session("Add UUID characteristic to a service failed", gattdb_session_id);
          return SID_ERROR_GENERIC;
        }
      } else {
        sl_ble_free_resources();
        sl_ble_abort_session("Start a new GATT database update session for characteristic failed", gattdb_session_id);
        return SID_ERROR_GENERIC;
      }
    }

    // *********** Fill up Descriptor properties ***********
    for (uint8_t j = 0; j < ctx.cfg->profile[i].desc_count; j++) {
      // Start a new GATT database update session
      sl_status = sl_bt_gattdb_new_session(&gattdb_session_id);

      if (sl_status == SL_STATUS_OK) {
        // The characteristic value attribute handle of the characteristic the descriptor is added to
        uint16_t last_characteristic_handle = 0;

        // Some commonly used UUIDs
        const uint8_t uuid_primary_service[2] = { 0x00, 0x28 };
        const uint8_t uuid_secondary_service[2] = { 0x01, 0x28 };
        const uint8_t uuid_chr[2] = { 0x03, 0x28 };

        uint16_t next_pri_srv = 0;
        uint16_t next_sec_srv = 0;
        // Find attributes of a certain type from a local GATT database
        (void)sl_bt_gatt_server_find_attribute(ble_profile[i].current_service_handle,
                                               UUID_LEN_16BIT,
                                               uuid_primary_service,
                                               &next_pri_srv);
        // Find attributes of a certain type from a local GATT database
        (void)sl_bt_gatt_server_find_attribute(ble_profile[i].current_service_handle,
                                               UUID_LEN_16BIT,
                                               uuid_secondary_service,
                                               &next_pri_srv);

        // Return whichever the smaller, 0 means not found
        uint16_t service_end = (next_pri_srv > next_sec_srv) ? next_sec_srv : next_pri_srv;
        if (service_end > 0) {
          // Found service, minus 1 to get last handle of previous service
          service_end--;
        }

        if (service_end == 0) {
          // If there is no following service, set ending to 0xFFFF
          service_end = 0xFFFF;
        }

        uint16_t start = ble_profile[i].current_service_handle;
        uint16_t next_char = 0;
        do {
          // Find attributes of a certain type from a local GATT database
          sl_status = sl_bt_gatt_server_find_attribute(start,
                                                       UUID_LEN_16BIT,
                                                       uuid_chr,
                                                       &next_char);
          if (sl_status != SL_STATUS_OK || next_char == 0 || next_char > service_end) {
            // If no characteristic is found or found characteristic beyond the service, exit
            break;
          }
          ++next_char;  // Convert to characteristic value handle
          start = next_char;
          last_characteristic_handle = next_char;
        } while (sl_status == SL_STATUS_OK);

        if (last_characteristic_handle == 0) {
          sl_ble_free_resources();
          sl_ble_abort_session("Invalid characteristic value", gattdb_session_id);
          return SID_ERROR_GENERIC;
        }

        uint16_t xPermissions = 0;
        if (ctx.cfg->profile[i].desc[j].perm.is_none) {
          xPermissions |= SL_BT_PERM_NONE;
        }
        if (ctx.cfg->profile[i].desc[j].perm.is_read) {
          xPermissions |= SL_BT_PERM_READ;
        }
        if (ctx.cfg->profile[i].desc[j].perm.is_write) {
          xPermissions |= SL_BT_PERM_WRITE;
        }

        uint16_t permissions = sl_ble_evaluate_permissions(xPermissions);

        uint16_t properties = 0;
        if ((xPermissions & SL_BT_PERM_READ)
            || (xPermissions & SL_BT_PERM_READ_ENCRYPTED)
            || (xPermissions & SL_BT_PERM_READ_ENCRYPTED_MITM)) {
          properties |= SL_BT_GATTDB_DESCRIPTOR_READ;
        }
        if ((xPermissions & SL_BT_PERM_WRITE)
            || (xPermissions & SL_BT_PERM_WRITE_ENCRYPTED)
            || (xPermissions & SL_BT_PERM_WRITE_ENCRYPTED_MITM)) {
          properties |= SL_BT_GATTDB_DESCRIPTOR_WRITE;
        }

        if (ctx.cfg->profile[i].desc[j].id.type == UUID_TYPE_16) {
          // 16-bit UUID
          sl_bt_uuid_16_t uuid_little_endian;
          // Little endian conversion
          for (uint8_t idx = 0; idx < UUID_LEN_16BIT; idx++) {
            uuid_little_endian.data[idx] = ctx.cfg->profile[i].desc[j].id.uu[(UUID_LEN_16BIT - idx) - 1];
          }
          // Add a 16-bits UUID descriptor to a characteristic
          sl_status = sl_bt_gattdb_add_uuid16_descriptor(gattdb_session_id,
                                                         last_characteristic_handle,
                                                         properties,
                                                         permissions,
                                                         uuid_little_endian,
                                                         sl_bt_gattdb_user_managed_value,
                                                         0, 0, NULL,  // Ignored parameters when value type is user_managed
                                                         &ble_profile[i].current_descriptor_handle[j]);
        } else if (ctx.cfg->profile[i].desc[j].id.type == UUID_TYPE_128) {
          // 128-bit uuid
          uuid_128 uuid_little_endian;
          // Little endian conversion
          for (uint8_t idx = 0; idx < UUID_LEN_128BIT; idx++) {
            uuid_little_endian.data[idx] = ctx.cfg->profile[i].desc[j].id.uu[(UUID_LEN_128BIT - idx) - 1];
          }
          // Add a 128-bits UUID descriptor to a characteristic
          sl_status = sl_bt_gattdb_add_uuid128_descriptor(gattdb_session_id,
                                                          last_characteristic_handle,
                                                          properties,
                                                          permissions,
                                                          uuid_little_endian,
                                                          sl_bt_gattdb_user_managed_value,
                                                          0, 0, NULL, // Ignored parameters when value type is user_managed
                                                          &ble_profile[i].current_descriptor_handle[j]);
        } else {
          sl_ble_free_resources();
          sl_ble_abort_session("Invalid descriptor UUID type", gattdb_session_id);
          return SID_ERROR_GENERIC;
        }

        if (sl_status == SL_STATUS_OK) {
          // Save all changes performed in current session and close the session
          if (sl_bt_gattdb_commit(gattdb_session_id) != SL_STATUS_OK) {
            sl_ble_free_resources();
            SID_PAL_LOG_ERROR("Save all changes performed in current session and close the session failed");
            return SID_ERROR_GENERIC;
          }
        } else {
          sl_ble_free_resources();
          sl_ble_abort_session("Add UUID descriptor to a characteristic failed", gattdb_session_id);
          return SID_ERROR_GENERIC;
        }
      } else {
        sl_ble_free_resources();
        sl_ble_abort_session("Start a new GATT database update session for descriptor failed", gattdb_session_id);
        return SID_ERROR_GENERIC;
      }
    }
  }

  // *********** Start Service ***********
  for (uint8_t i = 0; i < ctx.cfg->num_profile; i++) {
    // Start a new GATT database update session
    if (sl_bt_gattdb_new_session(&gattdb_session_id) == SL_STATUS_OK) {
      // Start a service
      if (sl_bt_gattdb_start_service(gattdb_session_id, ble_profile[i].current_service_handle) == SL_STATUS_OK) {
        // Save all changes performed in current session and close the session
        if (sl_bt_gattdb_commit(gattdb_session_id) != SL_STATUS_OK) {
          sl_ble_free_resources();
          SID_PAL_LOG_ERROR("Save all changes performed in current session and close the session failed");
          return SID_ERROR_GENERIC;
        }
      } else {
        sl_ble_free_resources();
        sl_ble_abort_session("Start service failed", gattdb_session_id);
        return SID_ERROR_GENERIC;
      }
    } else {
      sl_ble_free_resources();
      sl_ble_abort_session("Start a new GATT database update session for start service failed", gattdb_session_id);
      return SID_ERROR_GENERIC;
    }
  }

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_set_adv_data(uint8_t *data, uint8_t length)
{
  if (!data
      || !length
      || (length > ctx.mtu_size)
      || !ctx.cfg->is_adv_available
      || !ctx.cfg->adv_param.fast_enabled
      || !ctx.cfg->adv_param.slow_enabled) {
    SID_PAL_LOG_ERROR("Invalid advertisement parameters");
    return SID_ERROR_INVALID_ARGS;
  }

  bool found = false;

  for (uint8_t i = 0; i < ctx.cfg->num_profile; i++) {
    if (ctx.cfg->adv_param.type == ctx.cfg->profile[i].service.type) {
      found = true;
      break;
    }
  }

  if (!found) {
    SID_PAL_LOG_ERROR("Invalid service type");
    return SID_ERROR_INCOMPATIBLE_PARAMS;
  }

  // If we don't yet have an advertiser set, create one now
  if (advertising_set_handle == SL_BT_INVALID_ADVERTISING_SET_HANDLE) {
    if (sl_bt_advertiser_create_set(&advertising_set_handle) != SL_STATUS_OK) {
      SID_PAL_LOG_ERROR("Create advertising set failed");
      return SID_ERROR_GENERIC;
    }
  }

  // ********** Set the advertising parameters **********

  // Convert the address type
  uint8_t address_type;
  switch (ctx.cfg->mac_addr_type) {
    case SID_BLE_CFG_MAC_ADDRESS_TYPE_PUBLIC:
      address_type = sl_bt_gap_public_address;
      break;

    case SID_BLE_CFG_MAC_ADDRESS_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE:
      address_type = sl_bt_gap_random_nonresolvable_address;
      break;

    case SID_BLE_CFG_MAC_ADDRESS_TYPE_STATIC_RANDOM:
      address_type = sl_bt_gap_static_address;
      break;

    case SID_BLE_CFG_MAC_ADDRESS_TYPE_RANDOM_PRIVATE_RESOLVABLE:
      address_type = sl_bt_gap_random_resolvable_address;
      break;

    default:
      SID_PAL_LOG_ERROR("Invalid address type");
      return SID_ERROR_GENERIC;
      break;
  }

  // Return status value
  sl_status_t sl_status = SL_STATUS_OK;

  // Set the address type
  if (address_type == sl_bt_gap_public_address) {
    // Clear the random address in order to use the default advertiser address
    // which is either the public device address programmed at production or the
    // address written into persistent storage using @ref sl_bt_system_set_identity_address command.
    if (sl_bt_advertiser_clear_random_address(advertising_set_handle) != SL_STATUS_OK) {
      SID_PAL_LOG_ERROR("Clear random address failed");
      return SID_ERROR_GENERIC;
    }
  } else {
    // Random address
    bd_addr address = { 0 };
    bd_addr addressOut = { 0 };

    // The address is one of the random address types. See which one
    if (address_type == sl_bt_gap_static_address) {
      // Advertisers with static random address use the same shared address.
      // Generate it now if we don't have it already.
      if (!have_adv_static_random_addr) {
        // Get random bytes to construct a random address
        size_t data_len = 0;
        sl_status = sl_bt_system_get_random_data(sizeof(adv_static_random_addr.addr),
                                                 sizeof(adv_static_random_addr.addr),
                                                 &data_len,
                                                 adv_static_random_addr.addr);
        if (sl_status != SL_STATUS_OK) {
          SID_PAL_LOG_ERROR("Failed to get random data");
          return SID_ERROR_GENERIC;
        }

        // Make sure we got all the bytes we requested
        if (data_len < sizeof(adv_static_random_addr.addr)) {
          SID_PAL_LOG_ERROR("Failed to get enough random data");
          return SID_ERROR_GENERIC;
        }

        // Set the type bits to indicate the correct type
        adv_static_random_addr.addr[SL_BT_ADDR_TYPE_BYTE_INDEX] &= ~SL_BT_ADDR_TYPE_MASK;
        adv_static_random_addr.addr[SL_BT_ADDR_TYPE_BYTE_INDEX] |= SL_BT_ADDR_TYPE_STATIC_RANDOM;

        have_adv_static_random_addr = true;
      }
      // Copy the shared address
      memcpy(address.addr, adv_static_random_addr.addr, sizeof(address.addr));
    } else if (address_type == sl_bt_gap_random_nonresolvable_address) {
      // Advertisers that use a random non-resolvable address get a fresh random address
      size_t data_len = 0;
      sl_status = sl_bt_system_get_random_data(sizeof(address.addr),
                                               sizeof(address.addr),
                                               &data_len,
                                               address.addr);
      if (sl_status != SL_STATUS_OK) {
        SID_PAL_LOG_ERROR("Failed to get random data");
        return SID_ERROR_GENERIC;
      }

      // Make sure we got all the bytes we requested
      if (data_len < sizeof(address.addr)) {
        SID_PAL_LOG_ERROR("Failed to get enough random data");
        return SID_ERROR_GENERIC;
      }

      // Set the type bits to indicate the correct type
      address.addr[SL_BT_ADDR_TYPE_BYTE_INDEX] &= ~SL_BT_ADDR_TYPE_MASK;
      address.addr[SL_BT_ADDR_TYPE_BYTE_INDEX] |= SL_BT_ADDR_TYPE_NON_RESOLVABLE_PRIVATE;
    } else {
      // The type is a private resolvable random address.
      // The Bluetooth stack will generate the address internally and ignores the passed address.
    }

    // Set random address for this advertiser
    if (sl_bt_advertiser_set_random_address(advertising_set_handle, address_type, address, &addressOut) != SL_STATUS_OK) {
      SID_PAL_LOG_ERROR("Set random address for this advertiser failed");
      return SID_ERROR_GENERIC;
    }
  }

  // Set timing parameters
  sl_status = sl_bt_advertiser_set_timing(advertising_set_handle,
                                          adv_timing_params.fast_interval,
                                          adv_timing_params.fast_interval,
                                          adv_timing_params.fast_timeout, 0);
  if (sl_status != SL_STATUS_OK) {
    SID_PAL_LOG_ERROR("Set timing parameters failed");
    return SID_ERROR_GENERIC;
  }

  // Set the channel map
  sl_status = sl_bt_advertiser_set_channel_map(advertising_set_handle,
                                               SL_BT_CHANNEL_MAP);
  if (sl_status != SL_STATUS_OK) {
    SID_PAL_LOG_ERROR("Set the channel map failed");
    return SID_ERROR_GENERIC;
  }

  // Set the power level
  int16_t set_tx_power = 0;
  sl_status = sl_bt_advertiser_set_tx_power(advertising_set_handle,
                                            SL_BT_CONFIG_MAX_TX_POWER,
                                            &set_tx_power);
  if (sl_status != SL_STATUS_OK) {
    SID_PAL_LOG_ERROR("Set the power level failed");
    return SID_ERROR_GENERIC;
  }

  // ********** Generate the advertisement data **********

  // Legacy advertisement
  uint8_t adv_buf[SL_BT_MAX_LEGACY_ADV_DATA_LEN];
  memset(adv_buf, 0, sizeof(adv_buf));
  size_t adv_data_len = 0;
  uint32_t size_remaining = sizeof(adv_buf);
  uint32_t entry_size = 0;
  uint8_t adv_buf_idx = 0;

  // ====== Optionally append advertisement flags ======
  uint8_t flags = SL_BT_ADV_FLAG_GENERAL_DISCOVERABLE | SL_BT_ADV_FLAG_BR_EDR_NOT_SUPPORTED;
  // Make sure the data fits. We need one extra byte for type and another for length
  entry_size = sizeof(flags) + 1 + 1;
  if (size_remaining < entry_size) {
    SID_PAL_LOG_WARNING("Advertisement data does not fit");
  }
  // Set the length, type, and data
  adv_buf[adv_buf_idx] = sizeof(flags) + 1; // + 1 byte for the type
  adv_buf[adv_buf_idx + 1] = SL_BT_ADV_DATA_TYPE_FLAGS;
  if (sizeof(flags) > 0) {
    memcpy(&adv_buf[adv_buf_idx + 2], &flags, sizeof(flags));
  }
  size_remaining -= entry_size;
  adv_buf_idx += entry_size;

  // ====== Optionally append service UUIDs ======
  if (ctx.cfg->profile[0].service.id.uu != NULL) {
    // Make sure the data fits. We need one extra byte for type and another for length
    size_t uuid_len = UUID_LEN_16BIT;
    entry_size = uuid_len + 1 + 1;
    if (size_remaining < entry_size) {
      SID_PAL_LOG_WARNING("Advertisement data does not fit");
    }
    // Set the length, type, and data
    adv_buf[adv_buf_idx] = uuid_len + 1;  // + 1 byte for the type
    adv_buf[adv_buf_idx + 1] = SL_BT_ADV_DATA_TYPE_COMPLETE_16BIT_UUIDS;
    // Little endian conversion (16-bit UUID)
    uint8_t little_endian_uuid[2] = { 0, 0 };
    little_endian_uuid[0] = ctx.cfg->profile[0].service.id.uu[1];
    little_endian_uuid[1] = ctx.cfg->profile[0].service.id.uu[0];
    if (uuid_len > 0) {
      memcpy(&adv_buf[adv_buf_idx + 2], little_endian_uuid, uuid_len);
    }
    size_remaining -= entry_size;
    adv_buf_idx += entry_size;
  }

  // ====== Optionally append manufacturer data ======
  uint8_t manufacturer_len = length + BLE_COMPANY_ID_BYTE_LENGTH;
  uint8_t manuf_data[manufacturer_len];
  memset(manuf_data, 0, manufacturer_len);
  manuf_data[0] = (uint8_t)(BLE_COMPANY_ID & 0xFF);
  manuf_data[1] = (uint8_t)(BLE_COMPANY_ID >> 0x08);
  memcpy(&manuf_data[2], data, length);

  if ((manufacturer_len > 0) && (manuf_data != NULL)) {
    // Make sure the data fits. We need one extra byte for type and another for length
    entry_size = manufacturer_len + 1 + 1;
    if (size_remaining < entry_size) {
      SID_PAL_LOG_WARNING("Advertisement data does not fit");
    }
    // Set the length, type, and data
    adv_buf[adv_buf_idx] = manufacturer_len + 1;  // + 1 byte for the type
    adv_buf[adv_buf_idx + 1] = SL_BT_ADV_DATA_TYPE_MANUFACTURER_DATA;

    memcpy(&adv_buf[adv_buf_idx + 2], manuf_data, manufacturer_len);

    size_remaining -= entry_size;
    adv_buf_idx += entry_size;
  }

  // Set the final size
  adv_data_len = sizeof(adv_buf) - size_remaining;

  // Set the user data to the Bluetooth stack
  if (sl_bt_legacy_advertiser_set_data(advertising_set_handle, sl_bt_advertiser_advertising_data_packet, adv_data_len, adv_buf) != SL_STATUS_OK) {
    SID_PAL_LOG_ERROR("Set advertisement data failed");
    return SID_ERROR_GENERIC;
  }

  // ********** Generate the scan response data **********

  uint8_t scan_rsp_buf[SL_BT_MAX_LEGACY_ADV_DATA_LEN];
  memset(scan_rsp_buf, 0, sizeof(scan_rsp_buf));
  size_t scan_rsp_data_len = 0;
  uint32_t size_remaining_scan_rsp = sizeof(scan_rsp_buf);
  uint32_t entry_size_scan_rsp = 0;
  uint8_t scan_rsp_buf_idx = 0;

  // ====== Optionally append device name ======
  if ((ctx.cfg->name != NULL) && (strlen(ctx.cfg->name) > 0)) {
    // Make sure the data fits. We need one extra byte for type and another for length
    entry_size_scan_rsp = strlen(ctx.cfg->name) + 1 + 1;
    if (size_remaining_scan_rsp < entry_size_scan_rsp) {
      SID_PAL_LOG_WARNING("Advertisement data does not fit");
    }
    // Set the length, type, and data
    scan_rsp_buf[scan_rsp_buf_idx] = strlen(ctx.cfg->name) + 1; // + 1 byte for the type
    scan_rsp_buf[scan_rsp_buf_idx + 1] = SL_BT_ADV_DATA_TYPE_COMPLETE_LOCAL_NAME;
    memcpy(&scan_rsp_buf[scan_rsp_buf_idx + 2], ctx.cfg->name, strlen(ctx.cfg->name));

    size_remaining_scan_rsp -= entry_size_scan_rsp;
    scan_rsp_buf_idx += entry_size_scan_rsp;
  }

  // Set the final size
  scan_rsp_data_len = sizeof(scan_rsp_buf) - size_remaining_scan_rsp;

  // Set the user data to the Bluetooth stack
  if (sl_bt_legacy_advertiser_set_data(advertising_set_handle, sl_bt_advertiser_scan_response_packet, scan_rsp_data_len, scan_rsp_buf) != SL_STATUS_OK) {
    SID_PAL_LOG_ERROR("Set scan response data failed");
    return SID_ERROR_GENERIC;
  }

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_start_advertisement(void)
{
  // If we don't yet have an advertiser set, create one now
  if (advertising_set_handle == SL_BT_INVALID_ADVERTISING_SET_HANDLE) {
    if (sl_bt_advertiser_create_set(&advertising_set_handle) != SL_STATUS_OK) {
      SID_PAL_LOG_ERROR("Create advertising set failed");
      return SID_ERROR_GENERIC;
    }
  }

  // Start advertising with user-defined data to listen for incoming connections
  if (sl_bt_legacy_advertiser_start(advertising_set_handle, sl_bt_legacy_advertiser_connectable) != SL_STATUS_OK) {
    SID_PAL_LOG_ERROR("Start advertising failed");
    return SID_ERROR_GENERIC;
  }

  is_adv_active = true;

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_stop_advertisement(void)
{
  // Stop advertising if we have a handle and are currently active
  if ((advertising_set_handle != SL_BT_INVALID_ADVERTISING_SET_HANDLE) && (is_adv_active)) {
    if (sl_bt_advertiser_stop(advertising_set_handle) != SL_STATUS_OK) {
      SID_PAL_LOG_ERROR("Stop advertising failed");
      return SID_ERROR_GENERIC;
    }

    is_adv_active = false;
  }

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_send_data(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length)
{
  if (!ctx.is_connected) {
    SID_PAL_LOG_ERROR("Sidewalk BLE is not connected");
    return SID_ERROR_PORT_NOT_OPEN;
  }

  if (!data || !length || (length > ctx.mtu_size)) {
    SID_PAL_LOG_ERROR("Invalid arguments");
    return SID_ERROR_INVALID_ARGS;
  }

  uint16_t handle = 0;
  bool found = false;

  for (uint8_t i = 0; i < ctx.cfg->num_profile; i++) {
    if (id == ctx.cfg->profile[i].service.type) {
      for (uint8_t j = 0; j < ctx.cfg->profile[i].char_count; j++) {
        if (ctx.cfg->profile[i].characteristic[j].properties.is_notify) {
          handle = ble_profile[i].current_characteristic_handle[j];
          found = true;
          break;
        }
      }
    }
    if (found) {
      break;
    }
  }

  if (found) {
    // Notification does not need confirmation
    if (sl_bt_gatt_server_send_notification(ctx.conn_id, handle, length, data) != SL_STATUS_OK) {
      // Call the application (failure)
      ctx.callback->ind_callback(false);

      SID_PAL_LOG_ERROR("Send notification failed");
      return SID_ERROR_GENERIC;
    }
    // Call the application (success)
    ctx.callback->ind_callback(true);
  } else {
    SID_PAL_LOG_ERROR("Invalid argument to send notification");
    return SID_ERROR_INVALID_ARGS;
  }

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_set_callback(const sid_pal_ble_adapter_callbacks_t *cb)
{
  if (!cb) {
    return SID_ERROR_NULL_POINTER;
  }

  if (!cb->data_callback
      || !cb->notify_callback
      || !cb->conn_callback
      || !cb->ind_callback
      || !cb->mtu_callback
      || !cb->adv_start_callback) {
    return SID_ERROR_INVALID_ARGS;
  }

  ctx.callback = cb;
  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_disconnect(void)
{
  if (!ctx.is_connected) {
    SID_PAL_LOG_INFO("Sidewalk BLE is not connected");
    return SID_ERROR_NONE;
  }

  // Disconnect a remote device or cancel a pending connection
  if (sl_bt_connection_close(ctx.conn_id) != SL_STATUS_OK) {
    SID_PAL_LOG_ERROR("Failed to close the connection");
    return SID_ERROR_GENERIC;
  }

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_deinit(void)
{
  // If we have an advertiser set, clean that up
  if (advertising_set_handle != SL_BT_INVALID_ADVERTISING_SET_HANDLE) {
    sl_bt_advertiser_delete_set(advertising_set_handle);
    advertising_set_handle = SL_BT_INVALID_ADVERTISING_SET_HANDLE;
  }

  // Cleanup the resources
  sl_ble_free_resources();

  // Stop the Bluetooth stack
  // We ignore any errors, as there's nothing we could or should do if an error
  // is returned. The command commits to shutting down as much as it can, and the
  // way to continue is to start the stack again, regardless of what happened
  // at the time of stopping.
  (void)sl_bt_system_stop_bluetooth();
  is_bluetooth_started = false;

  return SID_ERROR_NONE;
}

// Triggered on system boot event
static void sl_ble_adapter_on_system_boot(sl_bt_evt_system_boot_t *event)
{
  // Unused parameter
  (void)event;
  // Print boot message
  SID_PAL_LOG_INFO("Sidewalk BLE stack is booted: v%d.%d.%d-b%d\n", event->major, event->minor, event->patch, event->build);

  // Nothing to do, because kernel is already started and ble_adapter_init() will take care of the initial configuration
}

// Triggered when advertiser has timed out
static void sl_ble_adapter_on_advertiser_timeout(sl_bt_evt_advertiser_timeout_t *event)
{
  if ((advertising_set_handle != SL_BT_INVALID_ADVERTISING_SET_HANDLE)
      && (advertising_set_handle == event->handle)
      && (is_adv_active)) {
    // If the app is advertising with the basic advertising APIs, stop it
    (void)sl_bt_advertiser_stop(advertising_set_handle);
    is_adv_active = false;
  }
}

// Triggered when a new connection has been opened
static void sl_ble_adapter_on_connection_opened(sl_bt_evt_connection_opened_t *event)
{
  if ((advertising_set_handle != SL_BT_INVALID_ADVERTISING_SET_HANDLE)
      && (advertising_set_handle == event->advertiser)) {
    // Use the adress the device connected with on this connection
    bd_addr remote_addr;
    memcpy(remote_addr.addr, event->address.addr, sizeof(remote_addr.addr));

    // Let the GATT Server call the corresponding callback
    ble_connection_cb_fnc(event->connection, true, &remote_addr);

    // Handle the implicit stop of the advertiser
    if (is_adv_active) {
      // If the app is advertising with the basic advertising APIs, stop it
      (void)sl_bt_advertiser_stop(advertising_set_handle);
      is_adv_active = false;
    }
  }
}

// Triggered when a connection has been closed
static void sl_ble_adapter_on_connection_closed(sl_bt_evt_connection_closed_t *event)
{
  // Let the GATT Server call the corresponding callback
  bd_addr remote_addr;
  memcpy(remote_addr.addr, ctx.bt_addr, sizeof(remote_addr.addr));
  ble_connection_cb_fnc(event->connection, false, &remote_addr);
}

static void sl_ble_adapter_on_gatt_server_characteristic_status_id(sl_bt_evt_gatt_server_characteristic_status_t *event)
{
  if (event->status_flags == sl_bt_gatt_server_confirmation) {
    // Call the application as a result of prvSendIndication as confirmation of indication has been received
    ctx.callback->ind_callback(true);
  }
}

static void sl_ble_adapter_on_gatt_server_indication_timeout_id(sl_bt_evt_gatt_server_indication_timeout_t *event)
{
  // Unused parameter
  (void)event;

  // Call the application as a result of prvSendIndication as confirmation of indication has been timeout
  ctx.callback->ind_callback(false);
}

static void sl_ble_adapter_on_gatt_server_user_write_request_id(sl_bt_evt_gatt_server_user_write_request_t *event)
{
  bool is_need_rsp = false;
  if ((event->att_opcode == sl_bt_gatt_write_request) || (event->att_opcode == sl_bt_gatt_prepare_write_request)) {
    is_need_rsp = true;
  }

  switch (event->att_opcode) {
    case sl_bt_gatt_write_request:
    case sl_bt_gatt_write_command:
      if (event->connection == ctx.conn_id) {
        bd_addr remote_addr;
        memcpy(remote_addr.addr, ctx.bt_addr, sizeof(remote_addr.addr));
        ble_request_write_cb_fnc(event->connection,
                                 SL_BT_GATTS_TRAN_TYPE_WRITE,
                                 &remote_addr,
                                 event->characteristic,
                                 event->offset,
                                 event->value.len,
                                 is_need_rsp,
                                 false,
                                 event->value.data);
      }
      break;

    case sl_bt_gatt_prepare_write_request:
      if (event->connection == ctx.conn_id) {
        bd_addr remote_addr;
        memcpy(remote_addr.addr, ctx.bt_addr, sizeof(remote_addr.addr));
        ble_request_write_cb_fnc(event->connection,
                                 SL_BT_GATTS_TRAN_TYPE_PREP_WRITE,
                                 &remote_addr,
                                 event->characteristic,
                                 event->offset,
                                 event->value.len,
                                 is_need_rsp,
                                 true,
                                 event->value.data);
      }
      break;

    default:
      // Nothing to do
      break;
  }
}

static void sl_ble_adapter_on_gatt_mtu_exchanged_id(sl_bt_evt_gatt_mtu_exchanged_t *event)
{
  ctx.callback->mtu_callback(event->mtu);
}

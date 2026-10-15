/***************************************************************************//**
 * @file
 * @brief ble_adapter.c
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

#if defined(SL_SIDEWALK_UNIT_TEST)
#pragma message "Unit test enabled"
#include "ble_adapter_mock.h"
#include "sid_ble_config_ifc_mock.h"
#include "sid_pal_ble_adapter_ifc_mock.h"
#include "sl_sidewalk_log_pal_mock.h"
#else
#include "sid_pal_ble_adapter_ifc.h"
#include "sid_ble_config_ifc.h"
#include "sl_sidewalk_log_pal.h"
#include "ble_adapter.h"
#endif

#include "sl_bt_api.h"
#include "sl_bluetooth_config.h"
#include "sl_memory_manager.h"

#if defined(SL_SIDEWALK_UNIT_TEST)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#endif
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define BLE_NOTIF_LENGTH                            (2)
#define BLE_NOTIF_ENABLED                           (1)
#define BLE_COMPANY_ID_BYTE_LEN                     (2)
#define BLE_COMPANY_ID                              (0x0171) // Amazon.com Services LLC
#define SL_BT_DEFAULT_SM_CONFIG_FLAGS               ((uint8_t) 0x0E)
#define SL_BT_HAL_SM_MAX_BONDING_COUNT              ((uint8_t) 0x08)
#define SL_BT_HAL_SM_POLICY_FLAGS                   ((uint8_t) 0x02)
#define UUID_LEN_16BIT                              (2)
#define UUID_LEN_32BIT                              (4)
#define UUID_LEN_128BIT                             (16)
#define SL_BT_PERM_NONE                             (0)
#define SL_BT_PERM_READ                             (1)
#define SL_BT_PERM_WRITE                            (16)
#define SL_BT_PERM_READ_ENCRYPTED                   (2)
#define SL_BT_PERM_READ_ENCRYPTED_MITM              (4)
#define SL_BT_PERM_WRITE_ENCRYPTED                  (32)
#define SL_BT_PERM_WRITE_ENCRYPTED_MITM             (64)
#define SL_BT_ADDR_TYPE_BYTE_INDEX                  (5)
#define SL_BT_MAX_LEGACY_ADV_DATA_LEN               (31)
#define SL_BT_ADDR_TYPE_MASK                        ((uint8_t) 0xC0)
#define SL_BT_ADDR_TYPE_NON_RESOLVABLE_PRIVATE      ((uint8_t) 0x00)
#define SL_BT_ADDR_TYPE_STATIC_RANDOM               ((uint8_t) 0xC0)
#define SL_BT_CHANNEL_MAP                           ((uint8_t) 7)
#define SL_BT_ADV_FLAG_GENERAL_DISCOVERABLE         ((uint8_t) 0x02)
#define SL_BT_ADV_FLAG_BR_EDR_NOT_SUPPORTED         ((uint8_t) 0x04)
#define SL_BT_ADV_DATA_TYPE_FLAGS                   ((uint8_t) 0x01)
#define SL_BT_ADV_DATA_TYPE_COMPLETE_16BIT_UUIDS    ((uint8_t) 0x03)
#define SL_BT_ADV_DATA_TYPE_COMPLETE_LOCAL_NAME     ((uint8_t) 0x09)
#define SL_BT_ADV_DATA_TYPE_MANUFACTURER_DATA       ((uint8_t) 0xFF)
#define SL_BT_GATTS_TRAN_TYPE_INVALID               ((uint32_t) 0x00)
#define SL_BT_GATTS_TRAN_TYPE_READ                  ((uint32_t) 0x01)
#define SL_BT_GATTS_TRAN_TYPE_WRITE                 ((uint32_t) 0x02)
#define SL_BT_GATTS_TRAN_TYPE_PREP_WRITE            ((uint32_t) 0x03)
#define SILABS_BLE_TX_POWER_SCALE_KOEF              ((int8_t) 10)
#define BLE_PAL                                     "pal ble: "

typedef struct ble_flags {
  uint8_t is_ble_started              : 1U;
  uint8_t is_kernel_started           : 1U;
  uint8_t is_adv_active               : 1U;
  uint8_t is_fast_adv_active          : 1U;
  uint8_t is_ama_active               : 1U;
  uint8_t have_adv_static_random_addr : 1U;
} ble_flags_t;
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
// Callback functions
static void ble_connection_cb_fnc(const bd_addr *bt_addr,
                                  uint16_t conn_id,
                                  bool connected);
static void ble_request_write_cb_fnc(uint32_t trans_id,
                                     size_t length,
                                     const uint8_t *data,
                                     uint16_t conn_id,
                                     int16_t attr_hnd,
                                     uint16_t offset,
                                     bool need_resp);
// BLE adapter interface functions
static sid_error_t ble_adapter_init(const sid_ble_config_t *cfg);
static sid_error_t ble_adapter_start_service(void);
static sid_error_t ble_adapter_set_adv_data(uint8_t *data, uint8_t length);
static sid_error_t ble_adapter_start_adv(void);
static sid_error_t ble_adapter_stop_adv(void);
static sid_error_t ble_adapter_get_rssi(int8_t *rssi);
static sid_error_t ble_adapter_get_tx_power(int16_t *tx_power);
static sid_error_t ble_adapter_send_data(sid_ble_cfg_service_identifier_t id,
                                         uint8_t *data,
                                         uint16_t length);
static sid_error_t ble_adapter_set_cb(const sid_pal_ble_adapter_callbacks_t *cb);
static sid_error_t ble_adapter_set_tx_power(int16_t tx_power);
static sid_error_t ble_adapter_disconnect(void);
static sid_error_t ble_adapter_deinit(void);
static sid_error_t ble_adapter_apply_user_cfg(sid_ble_user_config_t *cfg);
static sid_error_t ble_adapter_get_mac_addr(uint8_t *addr);
static void ble_adapter_update_ama_state(bool is_active);
// BLE event handlers
static void ble_adapter_on_system_boot(const sl_bt_evt_system_boot_t *event);
static void ble_adapter_on_adv_timeout(const sl_bt_evt_advertiser_timeout_t *event);
static void ble_adapter_on_conn_opened(const sl_bt_evt_connection_opened_t *event);
static void ble_adapter_on_conn_closed(const sl_bt_evt_connection_closed_t *event);
static void ble_adapter_on_gatt_server_char_status_id(const sl_bt_evt_gatt_server_characteristic_status_t *event);
static void ble_adapter_on_gatt_server_ind_timeout_id(const sl_bt_evt_gatt_server_indication_timeout_t *event);
static void ble_adapter_on_gatt_server_user_write_request_id(const sl_bt_evt_gatt_server_user_write_request_t *event);
static void ble_adapter_on_gatt_mtu_exchanged_id(const sl_bt_evt_gatt_mtu_exchanged_t *event);
// Helper functions
static void ble_adapter_init_failed(const char *msg);
static void ble_adapter_cleanup(void);
static void ble_adapter_fail_and_cleanup(const char *msg);
static void ble_adapter_abort_session(const char *msg, uint16_t session);
static void ble_adapter_cleanup_and_abort(const char *msg, uint16_t session);
static void ble_adapter_endian_convert(uint8_t *dst, const uint8_t *src, size_t len);
static sid_error_t ble_adapter_gen_random_addr(bd_addr *address);
static sid_error_t ble_adapter_set_addr_type(uint8_t addr_type);
static sid_error_t ble_adapter_update_conn_param(const sid_ble_cfg_conn_param_t *conn_param);
static sid_error_t ble_adapter_update_adv_param(const sid_ble_cfg_adv_param_t *adv_param);
static sid_error_t ble_adapter_curr_cfg_init(void);
static sid_error_t ble_adapter_apply_all_user_config(sid_ble_user_config_t *cfg);
static sid_error_t ble_adapter_apply_conn_user_config(sid_ble_user_config_t *cfg);
static sid_error_t ble_adapter_apply_adv_user_config(sid_ble_user_config_t *cfg);
static sid_error_t ble_adapter_get_adv_param(sid_ble_cfg_adv_param_t *adv_param);
static sid_error_t ble_adapter_get_last_conn_param(sid_ble_cfg_conn_param_t *conn_param);
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
static const struct sid_pal_ble_adapter_interface ble_ifc =
{
  .init             = ble_adapter_init,
  .start_service    = ble_adapter_start_service,
  .set_adv_data     = ble_adapter_set_adv_data,
  .start_adv        = ble_adapter_start_adv,
  .stop_adv         = ble_adapter_stop_adv,
  .get_rssi         = ble_adapter_get_rssi,
  .get_tx_pwr       = ble_adapter_get_tx_power,
  .send             = ble_adapter_send_data,
  .set_callback     = ble_adapter_set_cb,
  .set_tx_pwr       = ble_adapter_set_tx_power,
  .disconnect       = ble_adapter_disconnect,
  .deinit           = ble_adapter_deinit,
  .user_config      = (SID_SDK_CONFIG_ENABLE_BLE_USER_CONFIG ? &ble_adapter_apply_user_cfg : NULL),
  .notify_ama_state = (SID_SDK_CONFIG_ENABLE_BLE_USER_CONFIG ? &ble_adapter_update_ama_state : NULL),
  .get_mac_addr     = ble_adapter_get_mac_addr,
};

#if defined(SL_SIDEWALK_UNIT_TEST)
extern sid_pal_ble_adapter_ctx_t ctx;
extern sid_pal_ble_profile_config_t *ble_profile;
extern sid_ble_cfg_adv_param_t adv_timing_params;
extern uint8_t adv_set_handle;
extern bd_addr adv_static_random_addr;
extern bd_addr ble_adv_addr;
extern ble_flags_t ble_adapter_flags;
#else
// Current adapter context
static sid_pal_ble_adapter_ctx_t ctx = { 0 };
// BLE profile
static sid_pal_ble_profile_config_t *ble_profile = NULL;
// Advertising parameters
static sid_ble_cfg_adv_param_t adv_timing_params = { 0 };
// The advertising set handle allocated from Bluetooth stack
static uint8_t adv_set_handle = SL_BT_INVALID_ADVERTISING_SET_HANDLE;

// Static random address used for advertisers
// This static random Bluetooth address is used by all advertisers that
// specify the 'BTAddrTypeStaticRandom' address type in their configuration.
// The address is generated when it is first needed and remains unchanged until device reboot.
static bd_addr adv_static_random_addr = { 0 };

// The Mac address that is used for BLE advertising
// It is updated at the start of every new advertisement
static bd_addr ble_adv_addr = { 0 };

// BLE adapter flags
static ble_flags_t ble_adapter_flags = {
  .have_adv_static_random_addr = 0, // Indicate whether advertising static random address is available
  .is_ble_started = 0,              // Indicate whether BLE stack is started
  .is_kernel_started = 0,           // Indicate whether kernel is started
  .is_adv_active = 0,               // Indicate whether BLE advertising is active
  .is_fast_adv_active = 1,          // Indicate whether BLE advertising is slow or fast
  .is_ama_active = 0                // Indicate whether ama is active
 };
#endif
// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void sl_ble_adapter_on_event(sl_bt_msg_t *evt)
{
  if (evt == NULL) {
    return;
  }

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_system_boot_id:
      ble_adapter_on_system_boot(&evt->data.evt_system_boot);
      break;

    case sl_bt_evt_advertiser_timeout_id:
      ble_adapter_on_adv_timeout(&evt->data.evt_advertiser_timeout);
      if (ble_adapter_flags.is_fast_adv_active == true) {
        // Switch to slow advertisement
        ble_adapter_flags.is_fast_adv_active = false;
        adv_timing_params.fast_interval = ctx.current_adv_config.slow_interval;
        adv_timing_params.fast_timeout = ctx.current_adv_config.slow_timeout;
      } else {
        // Switch to fast advertisement
        ble_adapter_flags.is_fast_adv_active = true;
        adv_timing_params.fast_interval = ctx.current_adv_config.fast_interval;
        adv_timing_params.fast_timeout = ctx.current_adv_config.fast_timeout;
      }
      if (ctx.callback != NULL && ctx.callback->adv_start_callback != NULL) {
        ctx.callback->adv_start_callback();
      }
      break;

    case sl_bt_evt_connection_opened_id:
      ble_adapter_on_conn_opened(&evt->data.evt_connection_opened);
      break;

    case sl_bt_evt_connection_closed_id:
      ble_adapter_on_conn_closed(&evt->data.evt_connection_closed);
      // Switch back to fast advertisement
      ble_adapter_flags.is_fast_adv_active = true;
      adv_timing_params.fast_interval = ctx.current_adv_config.fast_interval;
      adv_timing_params.fast_timeout = ctx.current_adv_config.fast_timeout;
      break;

    case sl_bt_evt_gatt_server_characteristic_status_id:
      ble_adapter_on_gatt_server_char_status_id(&evt->data.evt_gatt_server_characteristic_status);
      break;

    case sl_bt_evt_gatt_server_indication_timeout_id:
      ble_adapter_on_gatt_server_ind_timeout_id(&evt->data.evt_gatt_server_indication_timeout);
      break;

    case sl_bt_evt_gatt_server_user_write_request_id:
      ble_adapter_on_gatt_server_user_write_request_id(&evt->data.evt_gatt_server_user_write_request);
      break;

    case sl_bt_evt_gatt_mtu_exchanged_id:
      ble_adapter_on_gatt_mtu_exchanged_id(&evt->data.evt_gatt_mtu_exchanged);
      break;

    case sl_bt_evt_connection_parameters_id:
      ctx.last_conn_config.min_conn_interval = evt->data.evt_connection_parameters.interval;
      ctx.last_conn_config.max_conn_interval = evt->data.evt_connection_parameters.interval;
      ctx.last_conn_config.slave_latency = evt->data.evt_connection_parameters.latency;
      ctx.last_conn_config.conn_sup_timeout = evt->data.evt_connection_parameters.timeout;
      SL_SID_LOG_PAL_INFO(BLE_PAL "Conn param updated, ama : %d, min_int: %d(%dms), max_int %d(%dms), sl %d, timeout %dms",
                          ble_adapter_flags.is_ama_active,
                          ctx.last_conn_config.min_conn_interval,
                          ((ctx.last_conn_config.min_conn_interval * 125) / 100),
                          ctx.last_conn_config.max_conn_interval,
                          ((ctx.last_conn_config.max_conn_interval * 125) / 100),
                          ctx.last_conn_config.slave_latency,
                          ctx.last_conn_config.conn_sup_timeout * 10);
      // Only update conn param once ama is finished/active and central has a different conn param
      if (SID_SDK_CONFIG_ENABLE_BLE_USER_CONFIG
          && ble_adapter_flags.is_ama_active
          && (memcmp(&(ctx.last_conn_config),
                     &(ctx.current_conn_config),
                     sizeof(sid_ble_cfg_conn_param_t)) != 0)) {
        ble_adapter_update_conn_param(&(ctx.current_conn_config));
      }
      break;

    default:
      // Other events are ignored
      break;
  }
}

// Function called by platform init when the RTOS kernel is started
void sl_ble_adapter_on_kernel_start(void)
{
  ble_adapter_flags.is_kernel_started = true;
}

sid_error_t sid_pal_ble_adapter_create(sid_pal_ble_adapter_interface_t *handle)
{
  if (!handle) {
    return SID_ERROR_INVALID_ARGS;
  }

  *handle = (sid_pal_ble_adapter_interface_t)&ble_ifc;

  return SID_ERROR_NONE;
}

sid_error_t ble_adapter_get_advertiser_address(uint8_t *addr)
{
  if (!addr) {
    return SID_ERROR_NULL_POINTER;
  }
  memcpy(addr, ble_adv_addr.addr, BLE_ADDR_MAX_LEN);
  return SID_ERROR_NONE;
}

sid_error_t ble_adapter_get_connection_address(uint8_t *addr)
{
  if (!addr) {
    return SID_ERROR_NULL_POINTER;
  }
  memcpy(addr, ctx.bt_addr, BLE_ADDR_MAX_LEN);
  return SID_ERROR_NONE;
}
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static void ble_connection_cb_fnc(const bd_addr *bt_addr,
                                  uint16_t conn_id,
                                  bool connected)
{
  if (!bt_addr
      || !ctx.callback
      || !ctx.callback->conn_callback) {
    return;
  }

  SL_SID_LOG_PAL_INFO(BLE_PAL "BLE state %sconnected",
                      connected ? "" : "dis");
  ctx.conn_id = conn_id;
  ctx.is_connected = connected;
  memcpy(ctx.bt_addr, bt_addr->addr, BLE_ADDR_MAX_LEN);
  ctx.callback->conn_callback(ctx.is_connected, ctx.bt_addr);
}

static void ble_request_write_cb_fnc(uint32_t trans_id,
                                     size_t length,
                                     const uint8_t *data,
                                     uint16_t conn_id,
                                     int16_t attr_hnd,
                                     uint16_t offset,
                                     bool need_resp)
{
  sid_ble_cfg_service_identifier_t id = { 0 };
  uint16_t notif_data = 0U;
  uint16_t sent_len = 0U;
  uint16_t rsp_val_len = 0U;

  if (data == NULL
      || ctx.cfg == NULL
      || ctx.callback == NULL
      || ctx.callback->data_callback == NULL
      || ctx.callback->notify_callback == NULL) {
    return;
  }

  id = ctx.cfg->profile[0].service.type;
  for (uint8_t j = 0U; j < ctx.cfg->profile[0].char_count; ++j) {
    if (attr_hnd == ble_profile[0].current_characteristic_handle[j]) {
      ctx.callback->data_callback(id, (uint8_t *)data, (uint16_t)length);
    }
  }
  for (uint8_t j = 0U; j < ctx.cfg->profile[0].desc_count; ++j) {
    if (attr_hnd == ble_profile[0].current_descriptor_handle[j]
        && length == BLE_NOTIF_LENGTH) {
      memcpy(&notif_data, data, sizeof(notif_data));
      ctx.callback->notify_callback(id, (notif_data == BLE_NOTIF_ENABLED));
    }
  }

  if (need_resp && conn_id) {
    // Send a response to a read/write operation
    switch (trans_id) {
      case SL_BT_GATTS_TRAN_TYPE_WRITE:
        // Send response to remote
        (void)sl_bt_gatt_server_send_user_write_response((uint8_t)conn_id,
                                                         attr_hnd,
                                                         0U);
        break;

      case SL_BT_GATTS_TRAN_TYPE_PREP_WRITE:
        // Send response to remote
        sl_bt_gatt_server_send_user_prepare_write_response((uint8_t)conn_id,
                                                           attr_hnd,
                                                           0U,
                                                           offset,
                                                           length,
                                                           data);
        break;

        case SL_BT_GATTS_TRAN_TYPE_READ:
          // Check MTU size
          if (sl_bt_gatt_server_get_mtu((uint8_t)conn_id, &rsp_val_len) != SL_STATUS_OK) {
            break;
          }
          // Compare MTU and the length of the unsent Attribute value
          if (rsp_val_len > length) {
            rsp_val_len = (uint16_t)length;
          }
          // Send response to remote
          (void)sl_bt_gatt_server_send_user_read_response((uint8_t)conn_id,
                                                          attr_hnd,
                                                          0,
                                                          rsp_val_len,
                                                          data,
                                                          &sent_len);
          break;

        default:
          break;
    }
  }
}

static void ble_adapter_init_failed(const char *msg)
{
  // When failed, stop the BLE stack to avoid getting into a partially inited state
  (void)sl_bt_system_stop_bluetooth();
  ble_adapter_flags.is_ble_started = false;
  if (msg != NULL) {
    SL_SID_LOG_PAL_ERROR(msg);
  }
}

static void ble_adapter_cleanup(void)
{
  for (uint8_t i = 0U; i < ctx.cfg->num_profile; ++i) {
    if ((ble_profile != NULL) && (ble_profile[i].current_descriptor_handle != NULL)) {
      sl_free(ble_profile[i].current_descriptor_handle);
      ble_profile[i].current_descriptor_handle = NULL;
    }
    if ((ble_profile != NULL) && (ble_profile[i].current_characteristic_handle != NULL)) {
      sl_free(ble_profile[i].current_characteristic_handle);
      ble_profile[i].current_characteristic_handle = NULL;
    }
    if (ble_profile != NULL) {
      ble_profile[i].current_service_handle = 0U;
    }
  }

  if (ble_profile != NULL) {
    sl_free(ble_profile);
    ble_profile = NULL;
  }
}

static void ble_adapter_fail_and_cleanup(const char *msg)
{
  ble_adapter_cleanup();
  ble_adapter_init_failed(msg);
}

static void ble_adapter_abort_session(const char *msg, uint16_t session)
{
  // Cancel all changes performed in current session and close the session
  (void)sl_bt_gattdb_abort(session);
  if (msg != NULL) {
    SL_SID_LOG_PAL_ERROR(msg);
  }
}

static void ble_adapter_cleanup_and_abort(const char *msg, uint16_t session)
{
  ble_adapter_cleanup();
  ble_adapter_abort_session(msg, session);
}

static sid_error_t ble_adapter_init(const sid_ble_config_t *cfg)
{
  uint8_t dummy_data = 0U;
  size_t output_len = 0U;
  int16_t tx_power_min = 0;
  int16_t tx_power_max = 0;

  if (!cfg) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "BLE cfg missing");
    ble_adapter_flags.is_ble_started = false;
    return SID_ERROR_INVALID_ARGS;
  }

  SL_SID_LOG_PAL_INFO(BLE_PAL "BLE init started");

  // Save BLE configuration
  ctx.cfg = cfg;
  ctx.mtu_size = cfg->mtu;

  if (ble_adapter_curr_cfg_init() != SID_ERROR_NONE) {
    return SID_ERROR_GENERIC;
  }

  // Allocate memory dinamically for BLE profile
  ble_profile = (sid_pal_ble_profile_config_t *)sl_malloc(ctx.cfg->num_profile * sizeof(sid_pal_ble_profile_config_t));
  if (!ble_profile) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "BLE profile malloc err");
    return SID_ERROR_GENERIC;
  }

  // Allocate memory dinamically for BLE characteristics and descriptors based on config file
  for (uint8_t i = 0U; i < ctx.cfg->num_profile; ++i) {
    ble_profile[i].current_service_handle = 0U;
    ble_profile[i].current_characteristic_handle = (uint16_t *)sl_malloc(ctx.cfg->profile[i].char_count * sizeof(uint16_t));
    if (!ble_profile[i].current_characteristic_handle) {
      SL_SID_LOG_PAL_ERROR(BLE_PAL "BLE characteristic malloc err");
      ble_adapter_cleanup();
      return SID_ERROR_GENERIC;
    }
    ble_profile[i].current_descriptor_handle = (uint16_t *)sl_malloc(ctx.cfg->profile[i].desc_count * sizeof(uint16_t));
    if (!ble_profile[i].current_descriptor_handle) {
      sl_free(ble_profile[i].current_characteristic_handle);
      ble_profile[i].current_characteristic_handle = NULL;
      SL_SID_LOG_PAL_ERROR(BLE_PAL "BLE descriptor malloc err");
      ble_adapter_cleanup();
      return SID_ERROR_GENERIC;
    }
  }

  if (ble_adapter_flags.is_fast_adv_active) {
    memcpy(&adv_timing_params, &(ctx.current_adv_config), sizeof(sid_ble_cfg_adv_param_t));
  }

  // Request the stack to start
  if (ble_adapter_flags.is_ble_started || !ble_adapter_flags.is_kernel_started) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "BLE start request failed due to %s%s",
                         ble_adapter_flags.is_ble_started ? "BLE stack has already started" : "",
                         !ble_adapter_flags.is_kernel_started ? " kernel has not started yet" : "");
    ble_adapter_flags.is_ble_started = false;
    return SID_ERROR_GENERIC;
  }

  if (sl_bt_system_start_bluetooth() != SL_STATUS_OK) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "BLE stack not ready");
    ble_adapter_flags.is_ble_started = false;
    return SID_ERROR_GENERIC;
  }

  // If the start request was successful,
  // we use sl_bt_system_get_random_data() to check for readiness
  // If successful, the BLE stack has started up
  // return error if not successful
  if (sl_bt_system_get_random_data(sizeof(dummy_data),
                                   sizeof(dummy_data),
                                   &output_len,
                                   &dummy_data) != SL_STATUS_OK) {
    ble_adapter_fail_and_cleanup(BLE_PAL "BLE stack start err");
    return SID_ERROR_GENERIC;
  }

  // Set default max MTU
  if (sl_bt_gatt_server_set_max_mtu(ctx.mtu_size,
                                    &ctx.mtu_size) != SL_STATUS_OK) {
    ble_adapter_fail_and_cleanup(BLE_PAL "default max mtu set err");
    return SID_ERROR_GENERIC;
  }

  // Set default bondable mode (bonding is disabled)
  if (sl_bt_sm_set_bondable_mode(0) != SL_STATUS_OK) {
    ble_adapter_fail_and_cleanup(BLE_PAL "default bondable mode set err");
    return SID_ERROR_GENERIC;
  }

  // Set security manager configuration
  if (sl_bt_sm_configure(SL_BT_DEFAULT_SM_CONFIG_FLAGS,
                         sl_bt_sm_io_capability_noinputnooutput) != SL_STATUS_OK) {
    ble_adapter_fail_and_cleanup(BLE_PAL "security manager cfg set err");
    return SID_ERROR_GENERIC;
  }

  // Store bonding configuration
  if (sl_bt_sm_store_bonding_configuration(SL_BT_HAL_SM_MAX_BONDING_COUNT,
                                           SL_BT_HAL_SM_POLICY_FLAGS) != SL_STATUS_OK) {
    ble_adapter_fail_and_cleanup(BLE_PAL "bonding cfg store err");
    return SID_ERROR_GENERIC;
  }

  // Set the global maximum TX power to the highest power that advertising can use
  if (sl_bt_system_set_tx_power(SL_BT_CONFIG_MIN_TX_POWER,
                                SILABS_BLE_TX_POWER_SCALE_KOEF * cfg->max_tx_power_in_dbm,
                                &tx_power_min,
                                &tx_power_max) != SL_STATUS_OK) {
    ble_adapter_fail_and_cleanup(BLE_PAL "global max TX pwr set err");
    return SID_ERROR_GENERIC;
  }

  ble_adapter_flags.is_ble_started = true;

  SL_SID_LOG_PAL_INFO(BLE_PAL "BLE init done");

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_start_service(void)
{
  static const uint8_t uuid_prim_srvc[2] = { 0x00, 0x28 };
  static const uint8_t uuid_sec_srvc[2] = { 0x01, 0x28 };
  static const uint8_t uuid_chr[2] = { 0x03, 0x28 };
  uint16_t gattdb_session_id = 0U;
  uint8_t uuid_srvc_buf[16] = { 0U };
  uint8_t len = 0U;
  uint16_t props = 0U;
  sl_bt_uuid_16_t uuid16 = { 0 };
  uuid_128 uuid128 = { 0 };
  uint16_t last_char_hnd = 0U;
  uint16_t next_pri_srv = 0U;
  uint16_t next_sec_srv = 0U;
  uint16_t service_end = 0U;
  uint16_t start = 0U;
  uint16_t next_char = 0U;
  sl_status_t sl_status = SL_STATUS_OK;

  if (!ctx.cfg || !ctx.cfg->num_profile) {
    ble_adapter_cleanup();
    return SID_ERROR_INVALID_ARGS;
  }

  // Start a new GATT database update session
  if (sl_bt_gattdb_new_session(&gattdb_session_id) != SL_STATUS_OK) {
    ble_adapter_cleanup_and_abort(BLE_PAL "gattdb update session err",
                                  gattdb_session_id);
    return SID_ERROR_GENERIC;
  }

  // Create a new service
  for (uint8_t i = 0U; i < ctx.cfg->num_profile; ++i) {
    if (ctx.cfg->profile[i].service.id.type == UUID_TYPE_16) {
      len = UUID_LEN_16BIT;
    } else if (ctx.cfg->profile[i].service.id.type == UUID_TYPE_32) {
      len = UUID_LEN_32BIT;
    } else if (ctx.cfg->profile[i].service.id.type == UUID_TYPE_128) {
      len = UUID_LEN_128BIT;
    } else {
      len = 0U;
    }
    if (len == 0U) {
      ble_adapter_cleanup_and_abort(BLE_PAL "invalid srvc uuid",
                                    gattdb_session_id);
      return SID_ERROR_GENERIC;
    }

    // Little endian conversion
    ble_adapter_endian_convert(uuid_srvc_buf,
                              ctx.cfg->profile[i].service.id.uu,
                              len);
    // Add a service into the local GATT database
    if (sl_bt_gattdb_add_service(gattdb_session_id,
                                 sl_bt_gattdb_primary_service,
                                 0U,
                                 len,
                                 uuid_srvc_buf,
                                 &ble_profile[i].current_service_handle) != SL_STATUS_OK) {
      ble_adapter_cleanup_and_abort(BLE_PAL "gattdb add srvc err",
                                    gattdb_session_id);
      return SID_ERROR_GENERIC;
    }

    // Fill up Characteristic properties
    for (uint8_t j = 0; j < ctx.cfg->profile[i].char_count; ++j) {
      props = (ctx.cfg->profile[i].characteristic[j].properties.is_notify
               ? SL_BT_GATTDB_CHARACTERISTIC_NOTIFY : 0U)
              | (ctx.cfg->profile[i].characteristic[j].properties.is_read
                 ? SL_BT_GATTDB_CHARACTERISTIC_READ : 0U)
              | (ctx.cfg->profile[i].characteristic[j].properties.is_write
                ? SL_BT_GATTDB_CHARACTERISTIC_WRITE : 0U)
              | (ctx.cfg->profile[i].characteristic[j].properties.is_write_no_resp
                 ? SL_BT_GATTDB_CHARACTERISTIC_WRITE_NO_RESPONSE : 0U);

      if (ctx.cfg->profile[i].characteristic[j].id.type == UUID_TYPE_16) {
        // 16-bit uuid
        memcpy(uuid16.data, ctx.cfg->profile[i].characteristic[j].id.uu, UUID_LEN_16BIT);
        // Add a 16-bits UUID characteristic to a service
        if (sl_bt_gattdb_add_uuid16_characteristic(gattdb_session_id,
                                                   ble_profile[i].current_service_handle,
                                                   props,
                                                   0,
                                                   SL_BT_GATTDB_NO_AUTO_CCCD, // Do not create client-config automatically
                                                   uuid16,
                                                   sl_bt_gattdb_user_managed_value,
                                                   0, 0, NULL, // Ignored parameters when value type is user_managed
                                                   &ble_profile[i].current_characteristic_handle[j]) != SL_STATUS_OK) {

          ble_adapter_cleanup_and_abort(BLE_PAL "add uuid16 err",
                                        gattdb_session_id);
          return SID_ERROR_GENERIC;
        }
      } else if (ctx.cfg->profile[i].characteristic[j].id.type == UUID_TYPE_128) {
        // 128-bit uuid
        memcpy(uuid128.data, ctx.cfg->profile[i].characteristic[j].id.uu, UUID_LEN_128BIT);
        // Add a 128-bits UUID characteristic to a service
        if (sl_bt_gattdb_add_uuid128_characteristic(gattdb_session_id,
                                                    ble_profile[i].current_service_handle,
                                                    props,
                                                    0,
                                                    SL_BT_GATTDB_NO_AUTO_CCCD, // Do not create client-config automatically
                                                    uuid128,
                                                    sl_bt_gattdb_user_managed_value,
                                                    0, 0, NULL, // Ignored parameters when value type is user_managed
                                                    &ble_profile[i].current_characteristic_handle[j]) != SL_STATUS_OK) {

          ble_adapter_cleanup_and_abort(BLE_PAL "add uuid128 err",
                                        gattdb_session_id);
          return SID_ERROR_GENERIC;
        }
      } else {
        ble_adapter_cleanup_and_abort(BLE_PAL "invalid characteristic uuid type",
                                      gattdb_session_id);
        return SID_ERROR_GENERIC;
      }
    }

    // Fill up Descriptor properties
    for (uint8_t j = 0U; j < ctx.cfg->profile[i].desc_count; ++j) {
      // Find attributes of a certain type from a local GATT database
      (void)sl_bt_gatt_server_find_attribute(ble_profile[i].current_service_handle,
                                             UUID_LEN_16BIT,
                                             uuid_prim_srvc,
                                             &next_pri_srv);
      // Find attributes of a certain type from a local GATT database
      (void)sl_bt_gatt_server_find_attribute(ble_profile[i].current_service_handle,
                                             UUID_LEN_16BIT,
                                             uuid_sec_srvc,
                                             &next_sec_srv);

      // Return whichever the smaller, 0 means not found
      service_end = (next_pri_srv > next_sec_srv) ? next_sec_srv : next_pri_srv;
      if (service_end > 0U) {
        // Found service, minus 1 to get last handle of previous service
        service_end--;
      }

      if (service_end == 0U) {
        // If there is no following service, set ending to 0xFFFF
        service_end = 0xFFFF;
      }

      start = ble_profile[i].current_service_handle;
      last_char_hnd = 0U;
      do {
        // Find attributes of a certain type from a local GATT database
        sl_status = sl_bt_gatt_server_find_attribute(start,
                                                     UUID_LEN_16BIT,
                                                     uuid_chr,
                                                     &next_char);
        if (sl_status != SL_STATUS_OK
            || next_char == 0
            || next_char > service_end) {
          // If no characteristic is found or found characteristic beyond the service, exit
          break;
        }
        ++next_char; // Convert to characteristic value handle
        start = next_char;
        last_char_hnd = next_char;
      } while (sl_status == SL_STATUS_OK);

      if (last_char_hnd == 0U) {
        ble_adapter_cleanup_and_abort(BLE_PAL "invalid characteristic value",
                                      gattdb_session_id);
        return SID_ERROR_GENERIC;
      }

      props = 0U;
      if (ctx.cfg->profile[i].desc[j].perm.is_read) {
        props |= SL_BT_GATTDB_DESCRIPTOR_READ;
      }
      if (ctx.cfg->profile[i].desc[j].perm.is_write) {
        props |= SL_BT_GATTDB_DESCRIPTOR_WRITE;
      }

      if (ctx.cfg->profile[i].desc[j].id.type == UUID_TYPE_16) {
        // 16-bit UUID, little endian conversion
        ble_adapter_endian_convert(uuid16.data,
                                   ctx.cfg->profile[i].desc[j].id.uu,
                                   UUID_LEN_16BIT);
        // Add a 16-bits UUID descriptor to a characteristic
        if (sl_bt_gattdb_add_uuid16_descriptor(gattdb_session_id,
                                               last_char_hnd,
                                               props,
                                               0,
                                               uuid16,
                                               sl_bt_gattdb_user_managed_value,
                                               0, 0, NULL, // Ignored parameters when value type is user_managed
                                               &ble_profile[i].current_descriptor_handle[j]) != SL_STATUS_OK) {
          ble_adapter_cleanup_and_abort(BLE_PAL "gattdb add uuid16 descriptor err",
                                        gattdb_session_id);
          return SID_ERROR_GENERIC;
        }
      } else if (ctx.cfg->profile[i].desc[j].id.type == UUID_TYPE_128) {
        // 128-bit uuid, little endian conversion
        ble_adapter_endian_convert(uuid128.data,
                                   ctx.cfg->profile[i].desc[j].id.uu,
                                   UUID_LEN_128BIT);
        // Add a 128-bits UUID descriptor to a characteristic
        if (sl_bt_gattdb_add_uuid128_descriptor(gattdb_session_id,
                                                last_char_hnd,
                                                props,
                                                0,
                                                uuid128,
                                                sl_bt_gattdb_user_managed_value,
                                                0, 0, NULL, // Ignored parameters when value type is user_managed
                                                &ble_profile[i].current_descriptor_handle[j]) != SL_STATUS_OK) {
          ble_adapter_cleanup_and_abort(BLE_PAL "gattdb add uuid128 descriptor err",
                                        gattdb_session_id);
          return SID_ERROR_GENERIC;
        }
      } else {
        ble_adapter_cleanup_and_abort(BLE_PAL "invalid descriptor uuid type",
                                      gattdb_session_id);
        return SID_ERROR_GENERIC;
      }
    }
  }

  // Start Service
  for (uint8_t i = 0U; i < ctx.cfg->num_profile; ++i) {
    if (sl_bt_gattdb_start_service(gattdb_session_id,
                                   ble_profile[i].current_service_handle) != SL_STATUS_OK) {
      ble_adapter_cleanup_and_abort(BLE_PAL "gattdb srvc err",
                                    gattdb_session_id);
      return SID_ERROR_GENERIC;
    }
  }

  // Save all changes performed in current session and close the session
  if (sl_bt_gattdb_commit(gattdb_session_id) != SL_STATUS_OK) {
    ble_adapter_cleanup_and_abort(BLE_PAL "gattdb commit err",
                                  gattdb_session_id);
    return SID_ERROR_GENERIC;
 }

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_gen_random_addr(bd_addr *address)
{
  size_t data_len = 0U;

  if (sl_bt_system_get_random_data(sizeof(address->addr),
                                   sizeof(address->addr),
                                   &data_len,
                                   address->addr) != SL_STATUS_OK) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "get random data err");
    return SID_ERROR_GENERIC;
  }

  // Make sure we got all the bytes we requested
  if (data_len < sizeof(address->addr)) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "get enough random data err");
    return SID_ERROR_GENERIC;
  }

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_set_addr_type(uint8_t addr_type)
{
  bd_addr address = { 0 };
  bd_addr addressOut = { 0 };

  // Generate random advertiser address
  if (addr_type == sl_bt_gap_static_address) {
    // Advertisers with static random address use the same shared address.
    // Generate it now if we don't have it already.
    if (!ble_adapter_flags.have_adv_static_random_addr) {
      // Get random bytes to construct a random address
      if (ble_adapter_gen_random_addr(&adv_static_random_addr) != SID_ERROR_NONE) {
        return SID_ERROR_GENERIC;
      }

      // Set the type bits to indicate the correct type
      adv_static_random_addr.addr[SL_BT_ADDR_TYPE_BYTE_INDEX] &= ~SL_BT_ADDR_TYPE_MASK;
      adv_static_random_addr.addr[SL_BT_ADDR_TYPE_BYTE_INDEX] |= SL_BT_ADDR_TYPE_STATIC_RANDOM;

      ble_adapter_flags.have_adv_static_random_addr = true;
    }
    // Copy the shared address
    memcpy(address.addr, adv_static_random_addr.addr, sizeof(address.addr));
  } else if (addr_type == sl_bt_gap_random_nonresolvable_address) {
    // Advertisers that use a random non-resolvable address get a fresh random address
    if (ble_adapter_gen_random_addr(&address) != SID_ERROR_NONE) {
      return SID_ERROR_GENERIC;
    }

    // Set the type bits to indicate the correct type
    address.addr[SL_BT_ADDR_TYPE_BYTE_INDEX] &= ~SL_BT_ADDR_TYPE_MASK;
    address.addr[SL_BT_ADDR_TYPE_BYTE_INDEX] |= SL_BT_ADDR_TYPE_NON_RESOLVABLE_PRIVATE;
  } else if (addr_type == sl_bt_gap_random_resolvable_address) {
    // The type is a private resolvable random address.
    // The Bluetooth stack will generate the address internally and ignores the passed address.
  }

  // Set the BLE advertiser address
  if (addr_type == sl_bt_gap_public_address) {
    // Clear the random address in order to use the default advertiser address
    // which is either the public device address programmed at production or the
    // address written into persistent storage using @ref sl_bt_system_set_identity_address command.
    if (sl_bt_advertiser_clear_random_address(adv_set_handle) != SL_STATUS_OK) {
      SL_SID_LOG_PAL_ERROR(BLE_PAL "clr random addr err");
      return SID_ERROR_GENERIC;
    }
  } else {
    // Set random address for this advertiser
    if (sl_bt_advertiser_set_random_address(adv_set_handle,
                                            addr_type,
                                            address,
                                            &addressOut) != SL_STATUS_OK) {
      SL_SID_LOG_PAL_ERROR(BLE_PAL "set adv random addr err");
      return SID_ERROR_GENERIC;
    }

    if (addr_type == sl_bt_gap_random_resolvable_address) {
      // Advertising address was generated by the stack
      memcpy(ble_adv_addr.addr, addressOut.addr, BLE_ADDR_MAX_LEN);
    } else {
      // Advertising address was generated by the ble adapter
      memcpy(ble_adv_addr.addr, address.addr, BLE_ADDR_MAX_LEN);
    }
  }

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_set_adv_data(uint8_t *data, uint8_t length)
{
  uint8_t idx = 0U;
  int16_t set_tx_power = 0;
  static uint8_t buf[SL_BT_MAX_LEGACY_ADV_DATA_LEN] = { 0U };
  uint8_t size_remaining = sizeof(buf);
  uint8_t entry_size = 0U;
  uint8_t buf_idx = 0U;
  uint8_t name_len = 0U;
  static const uint8_t addr_type_map[] = {
    [SID_BLE_CFG_MAC_ADDRESS_TYPE_PUBLIC] = sl_bt_gap_public_address,
    [SID_BLE_CFG_MAC_ADDRESS_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE] = sl_bt_gap_random_nonresolvable_address,
    [SID_BLE_CFG_MAC_ADDRESS_TYPE_STATIC_RANDOM] = sl_bt_gap_static_address,
    [SID_BLE_CFG_MAC_ADDRESS_TYPE_RANDOM_PRIVATE_RESOLVABLE] = sl_bt_gap_random_resolvable_address
  };

  if (!data
      || !length
      || !ctx.cfg
      || !ctx.cfg->is_adv_available
      || !ctx.cfg->adv_param.fast_enabled
      || !ctx.cfg->adv_param.slow_enabled) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "invalid adv params");
    return SID_ERROR_INVALID_ARGS;
  }

  for (idx = 0U; idx < ctx.cfg->num_profile; ++idx) {
    if (ctx.cfg->adv_param.type == ctx.cfg->profile[idx].service.type) {
        break;
    }
  }
  if (idx == ctx.cfg->num_profile) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "invalid srvc type");
    return SID_ERROR_INCOMPATIBLE_PARAMS;
  }

  // If we don't yet have an advertiser set, create one now
  if ((adv_set_handle == SL_BT_INVALID_ADVERTISING_SET_HANDLE)
      && (sl_bt_advertiser_create_set(&adv_set_handle) != SL_STATUS_OK)) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "create adv set err");
    return SID_ERROR_GENERIC;
  }

  // Set the advertising parameters
  if (ctx.cfg->mac_addr_type >= sizeof(addr_type_map)
      || ble_adapter_set_addr_type(addr_type_map[ctx.cfg->mac_addr_type]) != SID_ERROR_NONE) {
    return SID_ERROR_GENERIC;
  }

  // Set timing parameters
  if (sl_bt_advertiser_set_timing(adv_set_handle,
                                  adv_timing_params.fast_interval,
                                  adv_timing_params.fast_interval,
                                  (uint16_t)adv_timing_params.fast_timeout,
                                  0U) != SL_STATUS_OK) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "set timing params err");
    return SID_ERROR_GENERIC;
  }

  // Set the channel map
  if (sl_bt_advertiser_set_channel_map(adv_set_handle,
                                       SL_BT_CHANNEL_MAP) != SL_STATUS_OK) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "set channel map err");
    return SID_ERROR_GENERIC;
  }

  // Set the power level
  if (sl_bt_advertiser_set_tx_power(adv_set_handle,
                                    SILABS_BLE_TX_POWER_SCALE_KOEF * ctx.cfg->max_tx_power_in_dbm,
                                    &set_tx_power) != SL_STATUS_OK) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "set pwr lvl err");
    return SID_ERROR_GENERIC;
  }

  // Generate the legacy advertisement data
  // Optionally append advertisement flags
  // Make sure the data fits (size_remaining should be greater or equal then entry_size)
  // We need one extra byte for type and another for length
  entry_size = sizeof(uint8_t) + 2U;
  // Set the length, type, and data
  buf[buf_idx] = sizeof(uint8_t) + 1; // + 1 byte for the type
  buf[buf_idx + 1] = SL_BT_ADV_DATA_TYPE_FLAGS;
  buf[buf_idx + 2] = SL_BT_ADV_FLAG_GENERAL_DISCOVERABLE
                     | SL_BT_ADV_FLAG_BR_EDR_NOT_SUPPORTED;
  size_remaining -= entry_size;
  buf_idx += entry_size;

  // Optionally append service UUIDs
  // Make sure the data fits (size_remaining should be greater or equal then entry_size)
  // We need one extra byte for type and another for length
  entry_size = UUID_LEN_16BIT + 2U;
  // Set the length, type, and data
  buf[buf_idx] = UUID_LEN_16BIT + 1U; // + 1 byte for the type
  buf[buf_idx + 1] = SL_BT_ADV_DATA_TYPE_COMPLETE_16BIT_UUIDS;
  // Little endian conversion (16-bit UUID)
  buf[buf_idx + 2] = ctx.cfg->profile[0].service.id.uu[1];
  buf[buf_idx + 3] = ctx.cfg->profile[0].service.id.uu[0];
  size_remaining -= entry_size;
  buf_idx += entry_size;

  // Optionally append manufacturer data
  // Make sure the data fits (size_remaining should be greater or equal then entry_size)
  // We need one extra byte for type and another for length
  entry_size = length + BLE_COMPANY_ID_BYTE_LEN + 2U;
  // Set the length, type, and data
  buf[buf_idx] = length + BLE_COMPANY_ID_BYTE_LEN + 1; // + 1 byte for the type
  buf[buf_idx + 1] = SL_BT_ADV_DATA_TYPE_MANUFACTURER_DATA;
  buf[buf_idx + 2] = (uint8_t)(BLE_COMPANY_ID & 0xFFU);
  buf[buf_idx + 3] = (uint8_t)(BLE_COMPANY_ID >> 0x08U);
  memcpy(&buf[buf_idx + 4], data, length);
  size_remaining -= entry_size;
  buf_idx += entry_size;

  // Set the user data to the BLE stack
  if (sl_bt_legacy_advertiser_set_data(adv_set_handle,
                                       sl_bt_advertiser_advertising_data_packet,
                                       sizeof(buf) - size_remaining,
                                       buf) != SL_STATUS_OK) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "set adv data err");
    return SID_ERROR_GENERIC;
  }

  // Generate the scan response data
  buf_idx = 0U;
  size_remaining = sizeof(buf);
  // Optionally append device name
  if (ctx.cfg->name != NULL) {
    name_len = (uint8_t)strlen(ctx.cfg->name);
    if (name_len > 0U) {
      // Make sure the data fits (size_remaining should be greater or equal then entry_size)
      // We need one extra byte for type and another for length
      entry_size = name_len + 2U;
      // Set the length, type, and data
      buf[buf_idx] = (uint8_t)(name_len + 1U); // + 1 byte for the type
      buf[buf_idx + 1] = SL_BT_ADV_DATA_TYPE_COMPLETE_LOCAL_NAME;
      memcpy(&buf[buf_idx + 2], ctx.cfg->name, name_len);
      size_remaining -= entry_size;
      buf_idx += entry_size;
    }
  }

  // Set the user data to the Bluetooth stack
  if (sl_bt_legacy_advertiser_set_data(adv_set_handle,
                                       sl_bt_advertiser_scan_response_packet,
                                       sizeof(buf) - size_remaining,
                                       buf) != SL_STATUS_OK) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "scan resp data set err");
    return SID_ERROR_GENERIC;
  }

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_start_adv(void)
{
  // If we don't yet have an advertiser set, create one now
  if (adv_set_handle == SL_BT_INVALID_ADVERTISING_SET_HANDLE
      && sl_bt_advertiser_create_set(&adv_set_handle) != SL_STATUS_OK) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "create adv set err");
    return SID_ERROR_GENERIC;
  }

  // Start advertising with user-defined data to listen for incoming connections
  if (sl_bt_legacy_advertiser_start(adv_set_handle,
                                    sl_bt_legacy_advertiser_connectable) != SL_STATUS_OK) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "start adv err");
    return SID_ERROR_GENERIC;
  }

  ble_adapter_flags.is_adv_active = true;

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_stop_adv(void)
{
  // Stop advertising if we have a handle and are currently active
  if ((adv_set_handle != SL_BT_INVALID_ADVERTISING_SET_HANDLE)
      && (ble_adapter_flags.is_adv_active)) {
    if (sl_bt_advertiser_stop(adv_set_handle) != SL_STATUS_OK) {
      SL_SID_LOG_PAL_ERROR(BLE_PAL "stop adv err");
      return SID_ERROR_GENERIC;
    }

    ble_adapter_flags.is_adv_active = false;
  }

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_get_rssi(int8_t *rssi)
{
  sl_status_t status = SL_STATUS_OK;

  if (!rssi) {
    return SID_ERROR_NULL_POINTER;
  }

  status = sl_bt_connection_get_median_rssi((uint8_t)ctx.conn_id, rssi);
  return (status == SL_STATUS_OK) ? SID_ERROR_NONE : SID_ERROR_GENERIC;
}

static sid_error_t ble_adapter_send_data(sid_ble_cfg_service_identifier_t id,
                                         uint8_t *data,
                                         uint16_t length)
{
  uint16_t handle = 0U;
  bool found = false;

  if (!ctx.callback
      || !ctx.callback->ind_callback) {
    return SID_ERROR_NULL_POINTER;
  }

  if (!ctx.is_connected) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "BLE not connected");
    return SID_ERROR_PORT_NOT_OPEN;
  }

  if (!data
      || !length
      || (length > (ctx.mtu_size - 3U))
      || !ctx.cfg
      || !ctx.cfg->num_profile) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "invalid args");
    return SID_ERROR_INVALID_ARGS;
  }

  for (uint8_t i = 0U; i < ctx.cfg->num_profile && !found; ++i) {
    if (id != ctx.cfg->profile[i].service.type) {
      continue;
    }

    for (uint8_t j = 0U; j < ctx.cfg->profile[i].char_count; ++j) {
      if (ctx.cfg->profile[i].characteristic[j].properties.is_notify) {
        handle = ble_profile[i].current_characteristic_handle[j];
        found = true;
        break;
      }
    }
  }

  if (!found) {
    SL_SID_LOG_PAL_ERROR(BLE_PAL "invalid send notif arg");
    return SID_ERROR_INVALID_ARGS;
  }

  // Notification does not need confirmation
  if (sl_bt_gatt_server_send_notification((uint8_t)ctx.conn_id,
                                          handle,
                                          length,
                                          data) != SL_STATUS_OK) {
    // Call the application (failure)
    ctx.callback->ind_callback(false);

    SL_SID_LOG_PAL_ERROR(BLE_PAL "notif send err");
    return SID_ERROR_GENERIC;
  }

  // Call the application (success)
  ctx.callback->ind_callback(true);

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_set_cb(const sid_pal_ble_adapter_callbacks_t *cb)
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
  // Disconnect a remote device or cancel a pending connection
  if (sl_bt_connection_close((uint8_t)ctx.conn_id) != SL_STATUS_OK) {
    return SID_ERROR_GENERIC;
  }

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_deinit(void)
{
  // If we have an advertiser set, clean that up
  if (adv_set_handle != SL_BT_INVALID_ADVERTISING_SET_HANDLE) {
    sl_bt_advertiser_delete_set(adv_set_handle);
    adv_set_handle = SL_BT_INVALID_ADVERTISING_SET_HANDLE;
  }

  // Cleanup the resources
  ble_adapter_cleanup();

  // Stop the Bluetooth stack
  // We ignore any errors, as there's nothing we could or should do if an error
  // is returned. The command commits to shutting down as much as it can, and the
  // way to continue is to start the stack again, regardless of what happened
  // at the time of stopping.
  (void)sl_bt_system_stop_bluetooth();
  ble_adapter_flags.is_ble_started = false;

  return SID_ERROR_NONE;
}

// Triggered on system boot event
static void ble_adapter_on_system_boot(const sl_bt_evt_system_boot_t *event)
{
  // Print boot message
  SL_SID_LOG_PAL_INFO(BLE_PAL "BLE boot: v%d.%d.%d-b%d",
                      event->major,
                      event->minor,
                      event->patch,
                      event->build);

  // Nothing to do, because kernel is already started
  // and ble_adapter_init() will take care of the initial configuration
}

// Triggered when advertiser has timed out
static void ble_adapter_on_adv_timeout(const sl_bt_evt_advertiser_timeout_t *event)
{
  if (event == NULL) {
    return;
  }

  if ((adv_set_handle != SL_BT_INVALID_ADVERTISING_SET_HANDLE)
      && (adv_set_handle == event->handle)
      && (ble_adapter_flags.is_adv_active)) {
    (void)sl_bt_advertiser_stop(adv_set_handle);
    ble_adapter_flags.is_adv_active = false;
  }
}

// Triggered when a new connection has been opened
static void ble_adapter_on_conn_opened(const sl_bt_evt_connection_opened_t *event)
{
  bd_addr remote_addr = { 0 };

  if (event == NULL) {
    return;
  }

  if ((adv_set_handle != SL_BT_INVALID_ADVERTISING_SET_HANDLE)
      && (adv_set_handle == event->advertiser)) {
    // Use the adress the device connected with on this connection
    memcpy(remote_addr.addr, event->address.addr, sizeof(remote_addr.addr));
    // Let the GATT Server call the corresponding callback
    ble_connection_cb_fnc(&remote_addr, event->connection, true);

    // Handle the implicit stop of the advertiser
    if (ble_adapter_flags.is_adv_active) {
      (void)sl_bt_advertiser_stop(adv_set_handle);
      ble_adapter_flags.is_adv_active = false;
    }
  }
}

// Triggered when a connection has been closed
static void ble_adapter_on_conn_closed(const sl_bt_evt_connection_closed_t *event)
{
  bd_addr remote_addr = { 0 };

  if (event == NULL) {
    return;
  }

  memcpy(remote_addr.addr, ctx.bt_addr, sizeof(remote_addr.addr));
  // Let the GATT Server call the corresponding callback
  ble_connection_cb_fnc(&remote_addr, event->connection, false);
}

static void ble_adapter_on_gatt_server_char_status_id(const sl_bt_evt_gatt_server_characteristic_status_t *event)
{
  if (event == NULL
      || event->status_flags != sl_bt_gatt_server_confirmation
      || !ctx.callback
      || !ctx.callback->ind_callback) {
    return;
  }

  // Call the application as a result of prvSendIndication
  // as confirmation of indication has been received
  ctx.callback->ind_callback(true);
}

static void ble_adapter_on_gatt_server_ind_timeout_id(const sl_bt_evt_gatt_server_indication_timeout_t *event)
{
  (void)event;

  if (ctx.callback && ctx.callback->ind_callback) {
    // Call the application as a result of prvSendIndication
    // as confirmation of indication has been timeout
    ctx.callback->ind_callback(false);
  }
}

static void ble_adapter_on_gatt_server_user_write_request_id(const sl_bt_evt_gatt_server_user_write_request_t *event)
{
  if (event == NULL) {
    return;
  }

  if (event->att_opcode != sl_bt_gatt_write_request
      && event->att_opcode != sl_bt_gatt_write_command
      && event->att_opcode != sl_bt_gatt_prepare_write_request
      && event->connection != (uint8_t)ctx.conn_id) {
    return;
  }

  ble_request_write_cb_fnc((event->att_opcode == sl_bt_gatt_write_request)
                            || (event->att_opcode == sl_bt_gatt_write_command)
                            ? SL_BT_GATTS_TRAN_TYPE_WRITE : SL_BT_GATTS_TRAN_TYPE_PREP_WRITE,
                           event->value.len,
                           event->value.data,
                           event->connection,
                           event->characteristic,
                           event->offset,
                           (event->att_opcode == sl_bt_gatt_write_request)
                            || (event->att_opcode == sl_bt_gatt_prepare_write_request));
}

static void ble_adapter_on_gatt_mtu_exchanged_id(const sl_bt_evt_gatt_mtu_exchanged_t *event)
{
  if (event == NULL
      || !ctx.callback
      || !ctx.callback->mtu_callback) {
    return;
  }

  ctx.callback->mtu_callback(event->mtu);
}

static void ble_adapter_update_ama_state(bool is_active)
{
  ble_adapter_flags.is_ama_active = is_active;
}

static sid_error_t ble_adapter_curr_cfg_init(void)
{
  if (!ctx.cfg) {
    return SID_ERROR_INVALID_ARGS;
  }

  ctx.current_adv_config = ctx.cfg->adv_param;
  ctx.current_conn_config = ctx.cfg->conn_param;

  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_update_adv_param(const sid_ble_cfg_adv_param_t *adv_param)
{
  sid_error_t ret_code = SID_ERROR_NONE;

  if (!adv_param) {
    return SID_ERROR_NULL_POINTER;
  }
  if (!ble_adapter_flags.is_ble_started) {
    return SID_ERROR_UNINITIALIZED;
  }

  memcpy(&adv_timing_params, adv_param, sizeof(sid_ble_cfg_adv_param_t));

  if (ble_adapter_flags.is_adv_active) {
    ret_code = ble_adapter_stop_adv();
    if (ret_code != SID_ERROR_NONE) {
      return ret_code;
    }
    if (sl_bt_advertiser_set_timing(adv_set_handle,
                                    adv_param->fast_interval,
                                    adv_param->fast_interval,
                                    (uint16_t)(adv_param->fast_timeout),
                                    0U) != SL_STATUS_OK) {
      return SID_ERROR_GENERIC;
    }

    ret_code = ble_adapter_start_adv();
  }
  ctx.current_adv_config = adv_timing_params;
  return ret_code;
}

static sid_error_t ble_adapter_update_conn_param(const sid_ble_cfg_conn_param_t *conn_param)
{
  if (!conn_param) {
    return SID_ERROR_NULL_POINTER;
  }
  if (!ble_adapter_flags.is_ble_started) {
    return SID_ERROR_UNINITIALIZED;
  }

  if (ctx.is_connected
      && ble_adapter_flags.is_ama_active
      && sl_bt_connection_set_parameters((uint8_t)(ctx.conn_id),
                                         conn_param->min_conn_interval,
                                         conn_param->max_conn_interval,
                                         conn_param->slave_latency,
                                         conn_param->conn_sup_timeout,
                                         0x0000,
                                         0xffff) != SL_STATUS_OK) {
    return SID_ERROR_GENERIC;
  }
  ctx.current_conn_config = *conn_param;
  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_get_last_conn_param(sid_ble_cfg_conn_param_t *conn_param)
{
  if (!conn_param) {
    return SID_ERROR_NULL_POINTER;
  }
  if (!ble_adapter_flags.is_ble_started) {
    return SID_ERROR_UNINITIALIZED;
  }
  *conn_param = ctx.last_conn_config;
  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_get_adv_param(sid_ble_cfg_adv_param_t *adv_param)
{
  if (!adv_param) {
    return SID_ERROR_NULL_POINTER;
  }

  if (!ble_adapter_flags.is_ble_started) {
    return SID_ERROR_UNINITIALIZED;
  }

  memcpy(adv_param, &(ctx.current_adv_config), sizeof(sid_ble_cfg_adv_param_t));
  return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_apply_adv_user_config(sid_ble_user_config_t *cfg)
{
  if (!cfg) {
    return SID_ERROR_NULL_POINTER;
  }

  if (cfg->is_set) {
    return ble_adapter_update_adv_param(&cfg->adv_param);
  }

  return ble_adapter_get_adv_param(&cfg->adv_param);
}

static sid_error_t ble_adapter_apply_conn_user_config(sid_ble_user_config_t *cfg)
{
  if (!cfg) {
    return SID_ERROR_NULL_POINTER;
  }

  if (cfg->is_set) {
    return ble_adapter_update_conn_param(&cfg->conn_param);
  }

  return ble_adapter_get_last_conn_param(&cfg->conn_param);
}

static sid_error_t ble_adapter_apply_all_user_config(sid_ble_user_config_t *cfg)
{
  sid_error_t ret_code = SID_ERROR_NONE;

  if (!cfg) {
    return SID_ERROR_NULL_POINTER;
  }

  ret_code = ble_adapter_apply_adv_user_config(cfg);
  if (ret_code == SID_ERROR_NONE) {
    ret_code = ble_adapter_apply_conn_user_config(cfg);
  }
  return ret_code;
}

static sid_error_t ble_adapter_apply_user_cfg(sid_ble_user_config_t *cfg)
{
  if (!cfg) {
    return SID_ERROR_NULL_POINTER;
  }
  switch (cfg->cfg_type) {
    case SID_BLE_USER_CFG_ADV:
      return ble_adapter_apply_adv_user_config(cfg);
    case SID_BLE_USER_CFG_CONN:
      return ble_adapter_apply_conn_user_config(cfg);
    case SID_BLE_USER_CFG_ADV_AND_CONN:
      return ble_adapter_apply_all_user_config(cfg);
    default:
      return SID_ERROR_INCOMPATIBLE_PARAMS;
  }
}

static sid_error_t ble_adapter_get_mac_addr(uint8_t *addr)
{
  uint8_t type = 0U;

  if (!addr) {
    return SID_ERROR_NULL_POINTER;
  }

  if (sl_bt_system_get_identity_address((bd_addr *)addr, &type) != SL_STATUS_OK) {
    return SID_ERROR_NOT_FOUND;
  }
  return SID_ERROR_NONE;
}

static void ble_adapter_endian_convert(uint8_t *dst, const uint8_t *src, size_t len)
{
  if (!dst || !src || len == 0) {
    return;
  }
  for (size_t i = 0U; i < len; ++i) {
    dst[i] = src[len - 1U - i];
  }
}

static sid_error_t ble_adapter_set_tx_power(int16_t tx_power)
{
  int16_t cmd_tx_pwr = SILABS_BLE_TX_POWER_SCALE_KOEF * tx_power;
  sid_error_t ret = SID_ERROR_NONE;
  bool restore_adv = false;
  int16_t sup_max_tx_pwr = 0;
  int16_t sup_min_tx_pwr = 0;
  int16_t prev_set_min_tx_pwr = 0;
  int16_t prev_set_max_tx_pwr = 0;
  int16_t rf_path_gain = 0;

  if (cmd_tx_pwr < SL_BT_CONFIG_MIN_TX_POWER) {
    return SID_ERROR_INCOMPATIBLE_PARAMS;
  }

  // if previously set min power < new max_tx_power change min_tx power also
  if (sl_bt_system_get_tx_power_setting(&sup_min_tx_pwr,
                                        &sup_max_tx_pwr,
                                        &prev_set_min_tx_pwr,
                                        &prev_set_max_tx_pwr,
                                        &rf_path_gain) != SL_STATUS_OK) {
    return SID_ERROR_GENERIC;
  }

  if (prev_set_min_tx_pwr > cmd_tx_pwr) {
    prev_set_min_tx_pwr = cmd_tx_pwr;
  }

  if (ble_adapter_flags.is_adv_active) {
    restore_adv = true;
    if (ble_adapter_stop_adv() != SID_ERROR_NONE) {
      return SID_ERROR_GENERIC;
    }
  }

  if (sl_bt_system_set_tx_power(prev_set_min_tx_pwr,
                                cmd_tx_pwr,
                                &sup_min_tx_pwr,
                                &sup_max_tx_pwr) != SL_STATUS_OK) {
      ble_adapter_init_failed("Set global max TX pwr err");
      return SID_ERROR_GENERIC;
  }

  if (restore_adv) {
    ret = ble_adapter_start_adv();
  }

  return ret;
}

static sid_error_t ble_adapter_get_tx_power(int16_t *tx_power)
{
  int16_t sup_max_tx_pwr = 0;
  int16_t sup_min_tx_pwr = 0;
  int16_t prev_set_min_tx_pwr = 0;
  int16_t prev_set_max_tx_pwr = 0;
  int16_t rf_path_gain = 0;

  if (tx_power == NULL) {
    return SID_ERROR_INVALID_ARGS;
  }

  if (sl_bt_system_get_tx_power_setting(&sup_min_tx_pwr,
                                        &sup_max_tx_pwr,
                                        &prev_set_min_tx_pwr,
                                        &prev_set_max_tx_pwr,
                                        &rf_path_gain) != SL_STATUS_OK) {
    return SID_ERROR_GENERIC;
  }
  *tx_power = prev_set_max_tx_pwr / SILABS_BLE_TX_POWER_SCALE_KOEF;
  return SID_ERROR_NONE;
}

#if defined(SL_SIDEWALK_UNIT_TEST)
#pragma GCC diagnostic pop
#endif

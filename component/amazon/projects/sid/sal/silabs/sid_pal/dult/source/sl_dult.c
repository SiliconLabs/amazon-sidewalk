/***************************************************************************//**
 * @file  sl_dult.c
 * @brief DULT implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
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

#include "sl_dult.h"
#include "sli_dult.h"
#include "sl_bt_api.h"
#include "sl_status.h"
#include "sl_bluetooth_connection_config.h"
#if !SL_DULT_CONFIG_DYNAMIC_GATT
#include "gatt_db.h"
#endif

// -----------------------------------------------------------------------------
// Definitions

#define SL_DULT_INSTANCE_MAX SL_DULT_CONFIG_INSTANCE_MAX

// -----------------------------------------------------------------------------
// Local variables

// Store connections managed by the DULT
static sli_dult_ctx_t sli_dult_ctx[SL_DULT_INSTANCE_MAX] = {
  [0 ... (SL_DULT_INSTANCE_MAX - 1)] = {
    .adv.handle = SLI_DULT_ADV_SET_INVALID,
    .info.network_id = SL_DULT_NETWORK_ID_UNKNOW,
    .connection = SL_BT_INVALID_CONNECTION_HANDLE
  }
};
static sl_status_t sli_dult_register(uint8_t *handle,
                                     sl_dult_network_id_t network);
static bool ble_boot_success = false;
// -----------------------------------------------------------------------------
// Local functions

static sl_status_t sli_dult_register(uint8_t *handle,
                                     sl_dult_network_id_t network)
{
  if (handle == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  for (uint8_t i = 0; i < SL_DULT_INSTANCE_MAX; i++) {
    if (sli_dult_ctx[i].info.network_id == network) {
      return SL_STATUS_ALREADY_INITIALIZED;
    }
  }
  for (uint8_t i = 0; i < SL_DULT_INSTANCE_MAX; i++) {
    if (sli_dult_ctx[i].info.network_id == SL_DULT_NETWORK_ID_UNKNOW) {
      sli_dult_ctx[i].info.network_id = network;
      *handle = i;
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_FAIL;
}

static sl_status_t sli_dult_unregister(uint8_t handle)
{
  if (handle < SL_DULT_INSTANCE_MAX) {
    memset(&sli_dult_ctx[handle], 0, sizeof(sli_dult_ctx_t));
    sli_dult_ctx[handle].info.network_id = SL_DULT_NETWORK_ID_UNKNOW;
    sli_dult_ctx[handle].adv.handle      = SLI_DULT_ADV_SET_INVALID;
    sli_dult_ctx[handle].connection      = SL_BT_INVALID_CONNECTION_HANDLE;
    return SL_STATUS_OK;
  }
  return SL_STATUS_FAIL;
}

/**
 * @brief Handle BLE system stopped
 * @note This function sets the DULT state back to the state after dult init
 * @return sl_status_t SL_STATUS_OK in case of success
 */
static sl_status_t sli_dult_handle_ble_system_stopped(void)
{
  sl_status_t status = SL_STATUS_OK;

  ble_boot_success = false;
  for (uint8_t i = 0; i < SL_DULT_INSTANCE_MAX; i++) {
    if (sli_dult_ctx[i].info.network_id != SL_DULT_NETWORK_ID_UNKNOW) {
      if (sli_dult_adaptation_is_timer_running(&sli_dult_ctx[i].adv.rotation_timer)) {
        status = sli_dult_adaptation_timer_stop(&sli_dult_ctx[i].adv.rotation_timer);
        SLI_DULT_ASSERT_ST(status);
      }
      status = sli_dult_identifier_payload_stop(&sli_dult_ctx[i]);
      SLI_DULT_ASSERT_ST(status);
      if (sli_dult_ctx[i].motion_ctx.is_initialized) {
        status = sli_dult_motion_detection_reset(&sli_dult_ctx[i]);
        SLI_DULT_ASSERT_ST(status);
      }
      if (sli_dult_ctx[i].sound.is_initialized) {
        sli_dult_bt_sound_reset(&sli_dult_ctx[i]);
      }

      sli_dult_ctx[i].connection = SL_BT_INVALID_CONNECTION_HANDLE;
      if (sli_dult_ctx[i].info.adv_handle == NULL) {
        sli_dult_ctx[i].adv.handle = SLI_DULT_ADV_SET_INVALID;
      }
      sli_dult_ctx[i].adv.started          = false;
      sli_dult_ctx[i].adv.require_rotation = false;
      sli_dult_ctx[i].near_owner_state     = SL_DULT_STATE_COUNT;
    }
  }
  return status;
}

// -----------------------------------------------------------------------------
// Public functions

/**
 * @brief Initialize the DULT
 *
 * @param[in] ctx The DULT owner context
 * @return[out] sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_init(const sl_dult_accessory_info_t *info, uint8_t *handle)
{
  sl_status_t status;

  if (info == NULL || handle == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (ble_boot_success == false) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  status = sli_dult_register(handle, info->network_id);
  if (status != SL_STATUS_OK) {
    DULT_LOG_ERROR("sli_dult_register failed: %#lx", status);
    return status;
  }

  sli_dult_ctx_t *sli_dult_ctx = NULL;

  status = sli_dult_get_context(*handle, &sli_dult_ctx);
  if (status == SL_STATUS_OK) {
    memcpy(&sli_dult_ctx->info, info, sizeof(sl_dult_accessory_info_t));
    sli_dult_ctx->adv.started          = false;
    sli_dult_ctx->adv.require_rotation = false;
    sli_dult_ctx->near_owner_state     = SL_DULT_STATE_COUNT;
    if (info->adv_handle != NULL) {
      sli_dult_ctx->adv.handle = *info->adv_handle;
    }
    if (info->capabilities & SL_DULT_ACCESSORY_CAPABILITY_PLAY_SOUND) {
      sli_dult_bt_sound_init(sli_dult_ctx);
    }

    if (info->capabilities & SL_DULT_ACCESSORY_CAPABILITY_MOTION_DETECTOR_UT) {
      DULT_LOG_DEBUG("Motion detection capability enabled");
      sli_dult_motion_detection_init(sli_dult_ctx);
    }
  }

  return status;
}

/**
 * @brief De-initialize the DULT
 *
 * @return sl_status_t
 */
sl_status_t sl_dult_deinit(uint8_t handle)
{
  sl_status_t status;
  sli_dult_ctx_t *dult_ctx;

  status = sli_dult_get_context(handle, &dult_ctx);
  if (status != SL_STATUS_OK || dult_ctx == NULL) {
    DULT_LOG_ERROR("Failed to get dult context: %#lx", status);
    return status;
  }

  if (dult_ctx->adv.started) {
    status = sl_dult_stop(handle);
    if (status != SL_STATUS_OK) {
      DULT_LOG_ERROR("Failed to stop advertisement: %#lx", status);
    }
  }

  if (dult_ctx->info.adv_handle == NULL) {
    if (dult_ctx->adv.handle != SLI_DULT_ADV_SET_INVALID) {
      status = sl_bt_advertiser_delete_set(dult_ctx->adv.handle);
      if (status != SL_STATUS_OK) {
        DULT_LOG_ERROR("Failed to delete advertisement set: %#lx", status);
      }
    }
  }

  status = sli_dult_unregister(handle);
  if (status != SL_STATUS_OK) {
    DULT_LOG_ERROR("sli_dult_unregister failed: %#lx", status);
  }
  return status;
}

/**
 * @brief Register to DULT callback functions
 *
 * @param[in] handle The DULT handle
 * @param[in] cb_func
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_cb_register(uint8_t handle, const sl_dult_cb_t *cb_func)
{
  if (handle >= SL_DULT_INSTANCE_MAX || cb_func == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sli_dult_ctx[handle].callback = *cb_func;
  return SL_STATUS_OK;
}

/**************************************************************************//**
 * Event handler.
 *****************************************************************************/
void sli_bt_dult_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  sli_dult_ctx_t *dult_ctx;

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      ble_boot_success = true;
      break;

      // -------------------------------
      // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      for (uint8_t i = 0; i < SL_DULT_INSTANCE_MAX; i++) {
        if (sli_dult_ctx[i].adv.handle == evt->data.evt_connection_opened.advertiser) {
          DULT_LOG_INFO("Connection opened. %u ",
                        evt->data.evt_connection_opened.connection);
          sli_dult_ctx[i].connection = evt->data.evt_connection_opened.connection;
          break;
        }
      }
      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      sc = sli_dult_get_context_by_connection(evt->data.evt_connection_closed.connection,
                                              &dult_ctx);
      if (sc == SL_STATUS_OK) {
        DULT_LOG_INFO("Connection closed, reason: %d ",
                      evt->data.evt_connection_closed.reason);
        dult_ctx->connection = SL_BT_INVALID_CONNECTION_HANDLE;
        sli_dult_adv_connection_closed_notify(dult_ctx);
      }

      break;

    case sl_bt_evt_gatt_mtu_exchanged_id:
      // update MTU value, used for segmentation
      break;
    //--------------------------------
    // Triggered whenever the connection parameters are changed
    case sl_bt_evt_connection_parameters_id:
      break;
    case sl_bt_evt_connection_data_length_id:
      break;
    case sl_bt_evt_sm_confirm_bonding_id:
    case sl_bt_evt_sm_confirm_passkey_id:
    case sl_bt_evt_sm_bonded_id:
    case sl_bt_evt_sm_bonding_failed_id:
      // Bonding does not support
      break;
    //--------------------------------
    // This event indicates that a remote GATT is attempting to read a
    // value of an attribute from the local GATT database.
    case sl_bt_evt_gatt_server_characteristic_status_id:
      break;
    case sl_bt_evt_gatt_server_user_read_request_id:
      break;
    case sl_bt_evt_gatt_server_attribute_value_id:
      break;
    case sl_bt_evt_gatt_server_user_write_request_id:
      sc = sli_dult_get_context_by_connection(
          evt->data.evt_gatt_server_user_write_request.connection,
          &dult_ctx);
      if (sc == SL_STATUS_OK) {
        if (sli_dult_get_non_owner_characteristic_handle() ==
            evt->data.evt_gatt_server_user_write_request.characteristic) {
          sc = sli_dult_non_owner_operation_handler(evt->data.evt_gatt_server_user_write_request.connection,
                                                    evt->data.evt_gatt_server_user_write_request.value.data,
                                                    evt->data.evt_gatt_server_user_write_request.value.len);
          if (sc != SL_STATUS_OK) {
            DULT_LOG_ERROR("Non-owner operation handler failed: 0x%#lx", sc);
          }
        }
      }
      break;

    case sl_bt_evt_system_stopped_id:
      DULT_LOG_INFO("BLE stack stopped - resetting DULT state to initial state (after dult init)");
      sc = sli_dult_handle_ble_system_stopped();
      if (sc != SL_STATUS_OK) {
        DULT_LOG_ERROR("Failed to handle ble system stopped: %#lx", sc);
      }
      break;

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}

/**
 * @brief Get DULT context by handle
 *
 * @param[in] handle The DULT handle
 * @param[out] dult The DULT context
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sli_dult_get_context(uint8_t handle, sli_dult_ctx_t **dult)
{
  if (dult == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (handle >= SL_DULT_INSTANCE_MAX) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  *dult = &sli_dult_ctx[handle];
  return SL_STATUS_OK;
}

/**
 * @brief Get DULT context by connection
 *
 * @param[in] connection The current BLE connection handle
 * @param[out] dult The DULT context
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sli_dult_get_context_by_connection(uint8_t connection,
                                               sli_dult_ctx_t **dult)
{
  if (dult == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  for (uint8_t i = 0; i < SL_DULT_INSTANCE_MAX; i++) {
    if (sli_dult_ctx[i].connection == connection) {
      *dult = &sli_dult_ctx[i];
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_FAIL;
}

/**
 * @brief Update the level of battery
 *
 * @param[in] handle The DULT handle
 * @param[in] current level of battery
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_set_battery_level(uint8_t handle,
                                      sl_dult_battery_level current_value)
{
  sli_dult_ctx_t *dult_ctx;
  sl_status_t sc;

  // Validate battery level parameter
  if (current_value > SL_DULT_BATTERY_CRITICALLY_LOW) {
    DULT_LOG_ERROR("%s: Invalid battery level: %d", __func__, current_value);
    return SL_STATUS_INVALID_PARAMETER;
  }

  sc = sli_dult_get_context(handle, &dult_ctx);
  if (SL_STATUS_OK != sc) {
    DULT_LOG_ERROR("%s: Invalid dult handle: 0x%#lx", __func__, sc);
    return sc;
  }

  dult_ctx->battery_level = current_value;

  return SL_STATUS_OK;
}

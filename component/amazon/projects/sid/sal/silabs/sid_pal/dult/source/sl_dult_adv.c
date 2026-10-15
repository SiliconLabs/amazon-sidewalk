/***************************************************************************//**
 * @file  sl_dult_adv.c
 * @brief Implementation of DULT Advertisement
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
#include "sl_bt_api.h"
#include "sl_status.h"
#include "sli_dult.h"
#include "sli_dult_adaptation.h"
// -----------------------------------------------------------------------------
// Definitions
#define MAXIMUM_SIZE_ADV 31

#define SERVICR_DATA_TLV_LENGTH 5
#define SERVICR_DATA_TLV_TYPE   0x16
#define SERVICR_DATA_TLV_VALUE  0xFCB2

#define DULT_ADV_INTERVAL_MIN SL_DULT_CONFIG_ADV_INTERVAL_MIN
#define DULT_ADV_INTERVAL_MAX SL_DULT_CONFIG_ADV_INTERVAL_MAX
#define DULT_ADV_DURATION     SL_DULT_CONFIG_ADV_DURATION

#define DULT_ADV_ROTATION_INTERVAL_NEAR_OWNER_MS                               \
  (SL_DULT_CONFIG_ADV_ROTATION_INTERVAL_NEAR_OWNER_MINUTE * 60 * 1000)
#define DULT_ADV_ROTATION_INTERVAL_SEPARATED_MS                                \
  (SL_DULT_CONFIG_ADV_ROTATION_INTERVAL_SEPARATED_MINUTE * 60 * 1000)

static void scheduler_mac_address_rotation_timer(sli_dult_ctx_t *dult_ctx);

static void rotation_timer_callback(void *data);

/**
 * @brief create advertising payload
 *
 * @return
 */
sl_status_t sli_dult_adv_int_payload(sli_dult_ctx_t *dult_ctx, bool rotation)
{
  sl_status_t status;
  uint8_t payload[MAXIMUM_SIZE_ADV];
  uint8_t payload_size            = 0;
  uint8_t index                   = 0;
  uint8_t proprietary_payload_len = 0;
  bd_addr address = { 0 };
  bd_addr rpa_address;
  sl_dult_near_owner_state_t current_near_owner_state;

  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  current_near_owner_state = dult_ctx->near_owner_state;

  if (rotation) {
    status =
        sl_bt_advertiser_set_random_address(dult_ctx->adv.handle,
                                            sl_bt_gap_random_resolvable_address,
                                            address,
                                            &rpa_address);
    if (status != SL_STATUS_OK) {
      return status;
    } else {
      DULT_LOG_INFO("Bluetooth address: %02X:%02X:%02X:%02X:%02X:%02X",
                    rpa_address.addr[5],
                    rpa_address.addr[4],
                    rpa_address.addr[3],
                    rpa_address.addr[2],
                    rpa_address.addr[1],
                    rpa_address.addr[0]);
    }
  }

  // Add Flags TLV (if exists)
  payload[index++] = 2;                            // Length of the TLV
  payload[index++] = 1;                            // Type of the TLV
  payload[index++] = dult_ctx->info.adv_tlv_flags; // Value of the TLV

  // Add Service Data TLV
  payload[index++] = SERVICR_DATA_TLV_LENGTH; // Length of the TLV
  payload[index++] = SERVICR_DATA_TLV_TYPE;   // Type of the TLV (0x16)
  payload[index++] =
      (SERVICR_DATA_TLV_VALUE >> 0) & 0xFF; // Value of the TLV (0xFCB2)
  payload[index++] = (SERVICR_DATA_TLV_VALUE >> 8) & 0xFF;

  // Add Network ID
  payload[index++] = dult_ctx->info.network_id;

  // Add Near-owner + Reserved
  payload[index++] =
      (SL_DULT_STATE_NEAR_OWNER == current_near_owner_state) ? 1 : 0;

  // Add Proprietary Payload (if exists)
  if (dult_ctx->info.adv_company_data != NULL) {
    proprietary_payload_len = strlen((char *) dult_ctx->info.adv_company_data);
    DULT_LOG_INFO("set_data | company_data len: %d ", proprietary_payload_len);

    if (proprietary_payload_len > 0) {
      if (index + proprietary_payload_len > MAXIMUM_SIZE_ADV) {
        DULT_LOG_ERROR("Payload size would exceed maximum: %d + %d > %d",
                       index, proprietary_payload_len, MAXIMUM_SIZE_ADV);
        return SL_STATUS_INVALID_PARAMETER;
      }

      memcpy(&payload[index],
             dult_ctx->info.adv_company_data,
             proprietary_payload_len);
      index += proprietary_payload_len;
    }
  }

  // Set user-defined data for advertising
  payload_size = index;
  status =
      sl_bt_legacy_advertiser_set_data(dult_ctx->adv.handle,
                                       sl_bt_advertiser_advertising_data_packet,
                                       payload_size,
                                       (uint8_t *) &payload);

  if (status != SL_STATUS_OK) {
    DULT_LOG_ERROR("Failed to set advertisement data: %#x", (unsigned)status);
  }
  return status;
}

/**
 * @brief DULT advertisement start
 *
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_start(uint8_t handle)
{
  sl_status_t status;
  sli_dult_ctx_t *dult_ctx;
  bd_addr address = { 0 };
  bd_addr rpa_address;

  status = sli_dult_get_context(handle, &dult_ctx);
  if (status != SL_STATUS_OK || dult_ctx == NULL) {
    DULT_LOG_ERROR("Failed to get dult context: %#x", (unsigned)status);
    return status;
  }

  if (dult_ctx->adv.started) {
    DULT_LOG_ERROR("Advertising already started");
    return SL_STATUS_INVALID_STATE;
  }
  if (dult_ctx->info.adv_handle == NULL) {
    if (dult_ctx->adv.handle == SLI_DULT_ADV_SET_INVALID) {
      status = sl_bt_advertiser_create_set(&dult_ctx->adv.handle);
      if (status != SL_STATUS_OK) {
        DULT_LOG_ERROR("Failed to create advertisement set: %#x", (unsigned)status);
        return status;
      }
    }
  }
  DULT_LOG_DEBUG("%s | adv handle: %d", __func__, dult_ctx->adv.handle);

  status =
      sl_bt_advertiser_set_random_address(dult_ctx->adv.handle,
                                          sl_bt_gap_random_resolvable_address,
                                          address,
                                          &rpa_address);
  if (status != SL_STATUS_OK) {
    return status;
  } else {
    DULT_LOG_INFO("Bluetooth address: %02X:%02X:%02X:%02X:%02X:%02X",
                  rpa_address.addr[5],
                  rpa_address.addr[4],
                  rpa_address.addr[3],
                  rpa_address.addr[2],
                  rpa_address.addr[1],
                  rpa_address.addr[0]);
  }

  status = sli_dult_adv_int_payload(dult_ctx, false);
  if (status != SL_STATUS_OK) {
    return status;
  }

  status = sl_bt_advertiser_set_timing(
      dult_ctx->adv.handle,
      DULT_ADV_INTERVAL_MIN, // min. adv. interval
      DULT_ADV_INTERVAL_MAX, // max. adv. interval
      DULT_ADV_DURATION,     // adv. duration (units of 10 ms)
      0);                    // max. num. adv. events

  if (status != SL_STATUS_OK) {
    DULT_LOG_ERROR("sl_bt_advertiser_set_timing failed: %#x", (unsigned)status);
    return status;
  }

  status = sl_bt_legacy_advertiser_start(dult_ctx->adv.handle,
                                         sl_bt_legacy_advertiser_connectable);
  if (status != SL_STATUS_OK) {
    DULT_LOG_ERROR("sl_bt_legacy_advertiser_start failed: %#x", (unsigned)status);
    return status;
  }

  status = sli_dult_adaptation_timer_create(&dult_ctx->adv.rotation_timer,
                                            false,
                                            rotation_timer_callback);
  SLI_DULT_ASSERT_ST(status);
  dult_ctx->adv.started = true;

  scheduler_mac_address_rotation_timer(dult_ctx);
  return status;
}

/**
 * @brief DULT advertisement stop
 *
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_stop(uint8_t handle)
{
  sl_status_t status;
  sli_dult_ctx_t *dult_ctx;

  status = sli_dult_get_context(handle, &dult_ctx);
  if (status != SL_STATUS_OK || dult_ctx == NULL) {
    DULT_LOG_ERROR("Failed to get dult context: %#x", (unsigned)status);
    return status;
  }

  if (!dult_ctx->adv.started) {
    DULT_LOG_ERROR("Advertising already stopped");
    return SL_STATUS_INVALID_STATE;
  }

  status = sl_bt_advertiser_stop(dult_ctx->adv.handle);
  SLI_DULT_ASSERT_ST(status);
  if (sli_dult_adaptation_is_timer_running(&dult_ctx->adv.rotation_timer)) {
    status = sli_dult_adaptation_timer_stop(&dult_ctx->adv.rotation_timer);
    SLI_DULT_ASSERT_ST(status);
  }

  status = sli_dult_identifier_payload_stop(dult_ctx);
  SLI_DULT_ASSERT_ST(status);

  status = sli_dult_motion_detection_reset(dult_ctx);
  SLI_DULT_ASSERT_ST(status);

  dult_ctx->adv.started = false;
  return status;
}

void sli_dult_adv_near_owner_changed_notify(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t sc;

  if (dult_ctx == NULL) {
    return;
  }

  if (!dult_ctx->adv.started) {
    return;
  }
  DULT_LOG_INFO("Update the advertise payload due to state changed");
  sc = sl_bt_advertiser_stop(dult_ctx->adv.handle);
  SLI_DULT_ASSERT_ST(sc);

  sc = sli_dult_adv_int_payload(dult_ctx, false);
  SLI_DULT_ASSERT_ST(sc);

  sc = sl_bt_legacy_advertiser_start(dult_ctx->adv.handle,
                                     sl_bt_legacy_advertiser_connectable);
  SLI_DULT_ASSERT_ST(sc);

  scheduler_mac_address_rotation_timer(dult_ctx);
}

void sli_dult_adv_connection_closed_notify(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t sc;

  if (dult_ctx == NULL) {
    return;
  }

  if (dult_ctx->adv.started) {
    if (dult_ctx->adv.require_rotation) {
      sc = sli_dult_adv_int_payload(dult_ctx, true);
      SLI_DULT_ASSERT_ST(sc);
      dult_ctx->adv.require_rotation = false;
    }
    sc = sl_bt_legacy_advertiser_start(dult_ctx->adv.handle,
                                       sl_bt_legacy_advertiser_connectable);
    SLI_DULT_ASSERT_ST(sc);
  }
}

static void scheduler_mac_address_rotation_timer(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t sc;
  uint32_t interval;

  if (dult_ctx == NULL) {
    return;
  }

  if (SL_DULT_STATE_NEAR_OWNER == dult_ctx->near_owner_state) {
    interval = DULT_ADV_ROTATION_INTERVAL_NEAR_OWNER_MS;
  } else if (SL_DULT_STATE_SEPARATED == dult_ctx->near_owner_state) {
    interval = DULT_ADV_ROTATION_INTERVAL_SEPARATED_MS;
  } else {
    DULT_LOG_ERROR("Invalid near owner state transition: %d",
                   dult_ctx->near_owner_state);
    return;
  }
  if (sli_dult_adaptation_is_timer_running(&dult_ctx->adv.rotation_timer)) {
    sc = sli_dult_adaptation_timer_stop(&dult_ctx->adv.rotation_timer);
    SLI_DULT_ASSERT_ST(sc);
  }
  sc = sli_dult_adaptation_timer_start(&dult_ctx->adv.rotation_timer,
                                       interval,
                                       dult_ctx);
  SLI_DULT_ASSERT_ST(sc);

  DULT_LOG_DEBUG("Rotation timer updated to %u ms", (unsigned)interval);
}

/***************************************************************************//**
 * Callbacks
 ******************************************************************************/

static void rotation_timer_callback(void *data)
{
  sli_dult_ctx_t *dult_ctx = (sli_dult_ctx_t *) data;
  sl_status_t status;

  if (data == NULL) {
    return;
  }

  if (dult_ctx->connection != SL_BT_INVALID_CONNECTION_HANDLE) {
    dult_ctx->adv.require_rotation = true;
    return;
  }
  DULT_LOG_DEBUG("%s", __func__);
  if (dult_ctx->adv.started == true) {
    status = sl_bt_advertiser_stop(dult_ctx->adv.handle);
    SLI_DULT_ASSERT_ST(status);

    status = sli_dult_adv_int_payload(dult_ctx, true);
    SLI_DULT_ASSERT_ST(status);
    status = sl_bt_legacy_advertiser_start(dult_ctx->adv.handle,
                                           sl_bt_legacy_advertiser_connectable);
    SLI_DULT_ASSERT_ST(status);
  } else {
    status = sli_dult_adv_int_payload(dult_ctx, true);
    SLI_DULT_ASSERT_ST(status);
  }
}

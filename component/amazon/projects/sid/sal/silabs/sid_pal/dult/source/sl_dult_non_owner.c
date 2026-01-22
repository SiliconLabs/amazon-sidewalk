/***************************************************************************//**
 * @file  sl_dult_non_owner.c
 * @brief Implementation of DULT Non-Owner Operations
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

#include "sli_dult.h"
#include "sl_bt_api.h"
#if !SL_DULT_CONFIG_DYNAMIC_GATT
#include "gatt_db.h"
#endif

/*******************************************************************************
 *****************************   DEFINITIONS   *********************************
 ******************************************************************************/
#define SL_DULT_CMD_RESP_STATUS_SUCCESS               0x0000
#define SL_DULT_CMD_RESP_STATUS_INVALID_STATE         0x0001
#define SL_DULT_CMD_RESP_STATUS_INVALID_CONFIGURATION 0x0002
#define SL_DULT_CMD_RESP_STATUS_INVALID_LENGTH        0x0003
#define SL_DULT_CMD_RESP_STATUS_INVALID_PARAM         0x0004
#define SL_DULT_CMD_RESP_STATUS_INVALID_COMMAND       0xFFFF

#define SL_DULT_MAX_IDENTIFIER_SIZE 32 // TODO: Add to config?

#define SL_DULT_GATT_ERROR_CODE_NONE     0x00
#define SL_DULT_GATT_ERROR_CODE_NO_MEM   0x11
#define SL_DULT_GATT_ERROR_CODE_UNLIKELY 0x0E

// -----------------------------------------------------------------------------
// Local variables
#if SL_DULT_CONFIG_DYNAMIC_GATT
const uint8_t dult_non_user_service_uuid[16] = {0x85,
                                                0x2A,
                                                0x9F,
                                                0x57,
                                                0xC5,
                                                0x2A,
                                                0xED,
                                                0x88,
                                                0x26,
                                                0xC2,
                                                0xF4,
                                                0x12,
                                                0x01,
                                                0x00,
                                                0x19,
                                                0x15};

const uint8_t dult_non_user_characteristic_uuid[16] = {0x0E,
                                                       0x68,
                                                       0x21,
                                                       0x74,
                                                       0x37,
                                                       0x48,
                                                       0x61,
                                                       0xBF,
                                                       0x92,
                                                       0xFB,
                                                       0x68,
                                                       0x1D,
                                                       0x01,
                                                       0x00,
                                                       0x0C,
                                                       0x8E};

static uint16_t dult_non_user_session;
static uint16_t dult_non_user_service;
static uint16_t dult_non_user_characteristic;
static uuid_128 dult_non_user_characteristic_uuid128;
static uint16_t gattdb_dult_non_owner = 0xFFFF;
#endif

/* Non-owner responses definition */
typedef struct {
  sl_dult_opcode_t opcode;
  sl_cmd_resp_status_t status;
} __attribute__((packed)) sl_dult_cmd_resp_payload_t;

typedef struct {
  sl_dult_opcode_t opcode;
  union {
    uint8_t identifier[SL_DULT_MAX_IDENTIFIER_SIZE];
    uint8_t product_data[8];
    char manufacturer_name[64];
    char model_name[64];
    uint8_t accessory_category[8];
    sl_dult_protocol_implementation_version_t protocol_implementation_version;
    uint32_t accessory_capabilities;
    uint8_t network_id;
    sl_dult_firmware_version_t firmware_version;
    sl_dult_battery_type battery_type;
    sl_dult_battery_level battery_level;
    sl_dult_cmd_resp_payload_t cmd_resp;
  } data;
} __attribute__((packed)) sl_dult_non_owner_read_resp_t;

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

static sl_status_t get_product_data_handler(sli_dult_ctx_t *dult_ctx);
static sl_status_t get_manufacture_name_handler(sli_dult_ctx_t *dult_ctx);
static sl_status_t get_model_name_handler(sli_dult_ctx_t *dult_ctx);
static sl_status_t get_accessory_category_handler(sli_dult_ctx_t *dult_ctx);
static sl_status_t get_protocol_implementation_version_handler(sli_dult_ctx_t *dult_ctx);
static sl_status_t get_accessory_capabilities_handler(sli_dult_ctx_t *dult_ctx);
static sl_status_t get_network_id_handler(sli_dult_ctx_t *dult_ctx);
static sl_status_t get_firmware_version_handler(sli_dult_ctx_t *dult_ctx);
static sl_status_t get_battery_type_handler(sli_dult_ctx_t *dult_ctx);
static sl_status_t get_battery_level_handler(sli_dult_ctx_t *dult_ctx);
static sl_status_t get_identifier_handler(sli_dult_ctx_t *dult_ctx);
static sl_status_t send_command_response(uint8_t connection,
                                         uint16_t characteristic,
                                         sl_dult_opcode_t opcode,
                                         sl_cmd_resp_status_t status);

/*******************************************************************************
 **********************   GLOBAL INTERNAL FUNCTIONS   **************************
 ******************************************************************************/

sl_status_t sli_dult_non_owner_operation_handler(uint8_t connection,
                                                 uint8_t *data,
                                                 uint16_t len)
{
  sl_status_t status = SL_STATUS_OK;
  sl_dult_opcode_t opcode;
  sli_dult_ctx_t *dult_ctx;
  uint8_t gatt_error_code;
  bool flag_handled;

  if (data == NULL) {
    DULT_LOG_ERROR("%s: NULL data pointer", __func__);
    return SL_STATUS_NULL_POINTER;
  }

  if (len < sizeof(sl_dult_opcode_t)) {
    DULT_LOG_ERROR("%s: Invalid length: %d", __func__, len);
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = sli_dult_get_context_by_connection(connection, &dult_ctx);
  if (status != SL_STATUS_OK) {
    DULT_LOG_ERROR("%s: Failed to get dult context: %#lx", __func__, status);
    return status;
  }

  DULT_LOG_DEBUG("%s: network_id: %d", __func__, dult_ctx->info.network_id);
  DULT_ADV_LOG_HEXDUMP_ADDRESS(data, len);

  memcpy(&opcode, data, sizeof(sl_dult_opcode_t));

  /**
   * Non-Owner Operations Opcodes
   */
  flag_handled = true;
  switch (opcode) {
    case SLI_DULT_OC_NON_OWNER_SOUND_START:
      DULT_LOG_DEBUG("==== Non-Owner Sound Start ====");
      status = sli_dult_bt_sound_start(dult_ctx);
      break;
    case SLI_DULT_OC_NON_OWNER_SOUND_STOP:
      DULT_LOG_DEBUG("==== Non-Owner Sound Stop ====");
      status = sli_dult_bt_sound_stop(dult_ctx);
      break;
    case SLI_DULT_OC_NON_OWNER_GET_ID:
      DULT_LOG_DEBUG("==== Non-Owner Get Identifier ====");
      status = get_identifier_handler(dult_ctx);
      break;
    default:
      flag_handled = false;
      break;
  }

  /**
   * Accessory Info Opcodes.
   *
   * These opcodes SHALL be available when the accessory is in separated
   * state.These opcodes SHALL NOT be available when the accessory is in
   * the near-owner state.
   */
  if (!flag_handled && dult_ctx->near_owner_state == SL_DULT_STATE_SEPARATED) {
    flag_handled = true;
    switch (opcode) {
      case SLI_DULT_OC_GET_PRODUCT_DATA:
        DULT_LOG_DEBUG("== Non-Owner Get Product Data ==");
        status = get_product_data_handler(dult_ctx);
        break;
      case SLI_DULT_OC_GET_MANUFACTURER_NAME:
        DULT_LOG_DEBUG("== Non-Owner Get Manufacture Name ==");
        status = get_manufacture_name_handler(dult_ctx);
        break;
      case SLI_DULT_OC_GET_MODEL_NAME:
        DULT_LOG_DEBUG("==== Non-Owner Get Model Name ==");
        status = get_model_name_handler(dult_ctx);
        break;
      case SLI_DULT_OC_GET_ACCESSORY_CATEGORY:
        DULT_LOG_DEBUG("== Non-Owner Get Accessory Category ==");
        status = get_accessory_category_handler(dult_ctx);
        break;
      case SLI_DULT_OC_GET_PROTOCOL_IMPLEMENTATION_VERSION:
        DULT_LOG_DEBUG("== Non-Owner Get Protocol Implementation Version ==");
        status = get_protocol_implementation_version_handler(dult_ctx);
        break;
      case SLI_DULT_OC_GET_ACCESSORY_CAPABILITIES:
        DULT_LOG_DEBUG("== Non-Owner Get Accessory Capabilities ==");
        status = get_accessory_capabilities_handler(dult_ctx);
        break;
      case SLI_DULT_OC_GET_NETWORK_ID:
        DULT_LOG_DEBUG("== Non-Owner Get Network Id ==");
        status = get_network_id_handler(dult_ctx);
        break;
      case SLI_DULT_OC_GET_FIRMWARE_VERSION:
        DULT_LOG_DEBUG("== Non-Owner Get Firmware Version ==");
        status = get_firmware_version_handler(dult_ctx);
        break;
      case SLI_DULT_OC_GET_BATTERY_TYPE:
        DULT_LOG_DEBUG("== Non-Owner Get Battery Type ==");
        status = get_battery_type_handler(dult_ctx);
        break;
      case SLI_DULT_OC_GET_BATTERY_LEVEL:
        DULT_LOG_DEBUG("== Non-Owner Get Battery Level ==");
        status = get_battery_level_handler(dult_ctx);
        break;
      default:
        flag_handled = false;
        break;
    }
  }

  if (!flag_handled) {
    /**
     *  The accessory SHALL respond to any invalid opcode with Command_Response
     * and Invalid_command as the ResponseStatus.
     */
    status = send_command_response(dult_ctx->connection,
                                   gattdb_dult_non_owner,
                                   opcode,
                                   SL_DULT_CMD_RESP_STATUS_INVALID_COMMAND);
  }

  if (status == SL_STATUS_OK) {
    gatt_error_code = SL_DULT_GATT_ERROR_CODE_NONE;
  } else {
    gatt_error_code = SL_DULT_GATT_ERROR_CODE_UNLIKELY;
  }
  sl_bt_gatt_server_send_user_write_response(connection,
                                             gattdb_dult_non_owner,
                                             gatt_error_code);

  return status;
}

uint16_t sli_dult_get_non_owner_characteristic_handle(void)
{
  return gattdb_dult_non_owner;
}

sl_status_t sli_dult_sound_cmd_resp(sli_dult_ctx_t *dult_ctx,
                                    sl_dult_opcode_t opcode,
                                    sl_status_t status)
{
  sl_cmd_resp_status_t resp;

  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (status == SL_STATUS_OK) {
    resp = SL_DULT_CMD_RESP_STATUS_SUCCESS;
  } else if (status == SL_STATUS_INVALID_STATE) {
    resp = SL_DULT_CMD_RESP_STATUS_INVALID_STATE;
  } else {
    resp = SL_DULT_CMD_RESP_STATUS_INVALID_COMMAND;
  }
  return send_command_response(dult_ctx->connection,
                               gattdb_dult_non_owner,
                               opcode,
                               resp);
}

sl_status_t sli_dult_non_owner_send_sound_cmpl(sli_dult_ctx_t *dult_ctx)
{
  sl_dult_opcode_t opcode = SLI_DULT_OC_NON_OWNER_SOUND_COMPL;

  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  return sl_bt_gatt_server_send_indication(dult_ctx->connection,
                                           gattdb_dult_non_owner,
                                           sizeof(opcode),
                                           (const uint8_t *) &opcode);
}

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

/**
 * @brief Send command response
 *
 * Command response is described at:
 * https://www.ietf.org/archive/id/draft-ledvina-dult-accessory-protocol-00.html#command-response
 *
 * @param connection The BLE connection handle
 * @param characteristic The characteristic handle
 * @param opcode The opcode
 * @param status The command response status
 * @return sl_status_t SL_STATUS_OK in case of success
 */
static sl_status_t send_command_response(uint8_t connection,
                                         uint16_t characteristic,
                                         sl_dult_opcode_t opcode,
                                         sl_cmd_resp_status_t status)
{
  sl_status_t sc;
  sl_dult_non_owner_read_resp_t response = {0};

  response.opcode               = SLI_DULT_OC_NON_OWNER_CMD_RSP;
  response.data.cmd_resp.opcode = opcode;
  response.data.cmd_resp.status = status;

  DULT_LOG_DEBUG("%s: opcode: 0x%04X, status: 0x%04X",
                 __func__,
                 opcode,
                 status);
  sc = sl_bt_gatt_server_send_indication(connection,
                                         characteristic,
                                         sizeof(sl_dult_opcode_t) +
                                             sizeof(sl_dult_cmd_resp_payload_t),
                                         (const uint8_t *) &response);
  if (sc) {
    DULT_LOG_ERROR("%s: Failed to send indication", __func__);
    return SL_STATUS_FAIL;
  }
  return sc;
}

/**
 * @brief
 *
 *
 * @return sl_status_t SL_STATUS_OK in case of success
 */
static sl_status_t get_product_data_handler(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t status;
  sl_dult_non_owner_read_resp_t response = {0};

  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  response.opcode = SLI_DULT_OC_GET_PRODUCT_DATA_RESPONSE;

  size_t len;
  if (dult_ctx->info.product_data == NULL) {
    DULT_LOG_ERROR("%s: Failed to get product data", __func__);
    status = send_command_response(dult_ctx->connection,
                                   gattdb_dult_non_owner,
                                   SLI_DULT_OC_GET_PRODUCT_DATA_RESPONSE,
                                   SL_DULT_CMD_RESP_STATUS_INVALID_STATE);
    if (status) {
      DULT_LOG_ERROR("%s: Failed to send command response", __func__);
    }
    return status;
  }
  memcpy(response.data.product_data,
         dult_ctx->info.product_data,
         sizeof(response.data.product_data));

  len = sizeof(sl_dult_opcode_t) + sizeof(response.data.product_data);

  /* Send response */
  DULT_LOG_DEBUG("%s: %d bytes product data", __func__, len);
  status = sl_bt_gatt_server_send_indication(dult_ctx->connection,
                                             gattdb_dult_non_owner,
                                             len,
                                             (const uint8_t *) &response);
  if (status) {
    DULT_LOG_ERROR("%s: Failed to send indication: %#lx", __func__, status);
    return SL_STATUS_FAIL;
  }
  return status;
}

/**
 * @brief
 *
 *
 * @return sl_status_t SL_STATUS_OK in case of success
 */
static sl_status_t get_manufacture_name_handler(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t status;
  sl_dult_non_owner_read_resp_t response = {0};

  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  response.opcode = SLI_DULT_OC_GET_MANUFACTURER_NAME_RESPONSE;

  size_t len;
  if (dult_ctx->info.manufacturer_name == NULL) {
    DULT_LOG_ERROR("%s: Failed to get product data", __func__);
    status = send_command_response(dult_ctx->connection,
                                   gattdb_dult_non_owner,
                                   SLI_DULT_OC_GET_MANUFACTURER_NAME_RESPONSE,
                                   SL_DULT_CMD_RESP_STATUS_INVALID_STATE);
    if (status) {
      DULT_LOG_ERROR("%s: Failed to send command response", __func__);
    }
    return status;
  }
  len = strlen((char *) dult_ctx->info.manufacturer_name);
  strncpy((char *) response.data.manufacturer_name,
          (char *) dult_ctx->info.manufacturer_name,
          sizeof(response.data.manufacturer_name));
  len = sizeof(sl_dult_opcode_t) + len;

  /* Send response */
  DULT_LOG_DEBUG("%s: %d bytes manufacture name", __func__, len);
  status = sl_bt_gatt_server_send_indication(dult_ctx->connection,
                                             gattdb_dult_non_owner,
                                             len,
                                             (const uint8_t *) &response);
  if (status) {
    DULT_LOG_ERROR("%s: Failed to send indication: %#lx", __func__, status);
  }
  return status;
}

/**
 * @brief
 *
 *
 * @return sl_status_t SL_STATUS_OK in case of success
 */
static sl_status_t get_model_name_handler(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t status;
  sl_dult_non_owner_read_resp_t response = {0};

  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  response.opcode = SLI_DULT_OC_GET_MODEL_NAME_RESPONSE;

  size_t len;
  if (dult_ctx->info.model_name == NULL) {
    DULT_LOG_ERROR("%s: Failed to get product data", __func__);
    status = send_command_response(dult_ctx->connection,
                                   gattdb_dult_non_owner,
                                   SLI_DULT_OC_GET_MODEL_NAME_RESPONSE,
                                   SL_DULT_CMD_RESP_STATUS_INVALID_STATE);
    if (status) {
      DULT_LOG_ERROR("%s: Failed to send command response", __func__);
    }
    return status;
  }
  len = strlen((char *) dult_ctx->info.model_name);
  strncpy((char *) response.data.model_name,
          (char *) dult_ctx->info.model_name,
          sizeof(response.data.model_name));
  len = sizeof(sl_dult_opcode_t) + len;

  /* Send response */
  DULT_LOG_DEBUG("%s: %d bytes manufacture name", __func__, len);
  status = sl_bt_gatt_server_send_indication(dult_ctx->connection,
                                             gattdb_dult_non_owner,
                                             len,
                                             (const uint8_t *) &response);
  if (status) {
    DULT_LOG_ERROR("%s: Failed to send indication: %#lx", __func__, status);
  }
  return status;
}

/**
 * @brief
 *
 *
 * @return sl_status_t SL_STATUS_OK in case of success
 */
static sl_status_t get_accessory_category_handler(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t status;
  sl_dult_non_owner_read_resp_t response = {0};

  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  response.opcode = SLI_DULT_OC_GET_ACCESSORY_CATEGORY_RESPONSE;

  size_t len;
  /** Category of the device see DULT 4. Accessory Category Value for the
   * list of possible values
   */
  if (dult_ctx->info.category == 0) {
    DULT_LOG_ERROR("%s: Failed to get product data", __func__);
    status = send_command_response(dult_ctx->connection,
                                   gattdb_dult_non_owner,
                                   SLI_DULT_OC_GET_ACCESSORY_CATEGORY_RESPONSE,
                                   SL_DULT_CMD_RESP_STATUS_INVALID_STATE);
    if (status) {
      DULT_LOG_ERROR("%s: Failed to send command response", __func__);
    }
    return status;
  }

  /**
     * Byte 0: Uint8 value of Accessory Category Value (Section 4)
     * Byte 1-7: Reserved
     */
  memset(response.data.accessory_category,
         0,
         sizeof(response.data.accessory_category));
  response.data.accessory_category[0] = dult_ctx->info.category;
  len                                 = sizeof(sl_dult_opcode_t) + 8;

  /* Send response */
  DULT_LOG_DEBUG("%s: %d bytes Accessory Category", __func__, len);
  status = sl_bt_gatt_server_send_indication(dult_ctx->connection,
                                             gattdb_dult_non_owner,
                                             len,
                                             (const uint8_t *) &response);
  if (status) {
    DULT_LOG_ERROR("%s: Failed to send indication: %#lx", __func__, status);
  }
  return status;
}

/**
 * @brief
 *
 *
 * @return sl_status_t SL_STATUS_OK in case of success
 */
static sl_status_t get_protocol_implementation_version_handler(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t status;
  sl_dult_non_owner_read_resp_t response = {0};

  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  response.opcode = SLI_DULT_OC_GET_PROTOCOL_IMPLEMENTATION_VERSION_RESPONSE;

  size_t len;
  response.data.protocol_implementation_version = dult_ctx->info.protocol_implementation_version;
  len = sizeof(sl_dult_opcode_t) + sizeof(sl_dult_protocol_implementation_version_t);

  /* Send response */
  DULT_LOG_DEBUG("%s: %d bytes "
                 "Protocol Implementation Version",
                 __func__,
                 len);
  status = sl_bt_gatt_server_send_indication(dult_ctx->connection,
                                             gattdb_dult_non_owner,
                                             len,
                                             (const uint8_t *) &response);
  if (status) {
    DULT_LOG_ERROR("%s: Failed to "
                   "send indication: %#lx",
                   __func__,
                   status);
    return SL_STATUS_FAIL;
  }
  return status;
}

/**
 * @brief
 *
 *
 * @return sl_status_t SL_STATUS_OK in case of success
 */
static sl_status_t get_accessory_capabilities_handler(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t status;
  sl_dult_non_owner_read_resp_t response = {0};

  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  response.opcode = SLI_DULT_OC_GET_ACCESSORY_CAPABILITIES_RESPONSE;

  size_t len;
  response.data.accessory_capabilities = dult_ctx->info.capabilities & 0xFFFF;
  len = sizeof(sl_dult_opcode_t) + sizeof(uint32_t);

  /* Send response */
  DULT_LOG_DEBUG("%s: %d bytes "
                 "Accessory Capabilities ",
                 __func__,
                 len);
  status = sl_bt_gatt_server_send_indication(dult_ctx->connection,
                                             gattdb_dult_non_owner,
                                             len,
                                             (const uint8_t *) &response);
  if (status) {
    DULT_LOG_ERROR("%s: Failed to send "
                   "indication: %#lx",
                   __func__,
                   status);
    return SL_STATUS_FAIL;
  }
  return status;
}

/**
 * @brief
 *
 *
 * @return sl_status_t SL_STATUS_OK in case of success
 */
static sl_status_t get_network_id_handler(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t status;
  sl_dult_non_owner_read_resp_t response = {0};

  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  response.opcode = SLI_DULT_OC_GET_NETWORK_ID_RESPONSE;

  size_t len;
  response.data.network_id = dult_ctx->info.network_id;
  len                      = sizeof(sl_dult_opcode_t) + 1;

  /* Send response */
  DULT_LOG_DEBUG("%s: %d bytes Network ID ", __func__, len);
  status = sl_bt_gatt_server_send_indication(dult_ctx->connection,
                                             gattdb_dult_non_owner,
                                             len,
                                             (const uint8_t *) &response);
  if (status) {
    DULT_LOG_ERROR("%s: Failed to send indication: %#lx", __func__, status);
    return SL_STATUS_FAIL;
  }
  return status;
}

/**
 * @brief
 *
 *
 * @return sl_status_t SL_STATUS_OK in case of success
 */
static sl_status_t get_firmware_version_handler(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t status;
  sl_dult_non_owner_read_resp_t response = {0};

  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  response.opcode = SLI_DULT_OC_GET_FIRMWARE_VERSION_RESPONSE;

  size_t len;
  response.data.firmware_version = dult_ctx->info.firmware_version;
  len = sizeof(sl_dult_opcode_t) + sizeof(sl_dult_firmware_version_t);

  /* Send response */
  DULT_LOG_DEBUG("%s: %d bytes Firmware Version ", __func__, len);
  status = sl_bt_gatt_server_send_indication(dult_ctx->connection,
                                             gattdb_dult_non_owner,
                                             len,
                                             (const uint8_t *) &response);
  if (status) {
    DULT_LOG_ERROR("%s: Failed to send indication: %#lx", __func__, status);
    return SL_STATUS_FAIL;
  }
  return status;
}

/**
 * @brief
 *
 *
 * @return sl_status_t SL_STATUS_OK in case of success
 */
static sl_status_t get_battery_type_handler(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t status;
  sl_dult_non_owner_read_resp_t response = {0};

  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  response.opcode = SLI_DULT_OC_GET_BATTERY_TYPE_RESPONSE;

  size_t len;
  response.data.battery_type = dult_ctx->info.battery_type;
  len = sizeof(sl_dult_opcode_t) + sizeof(sl_dult_battery_type);

  /* Send response */
  DULT_LOG_DEBUG("%s: %d bytes Battery Type ", __func__, len);
  status = sl_bt_gatt_server_send_indication(dult_ctx->connection,
                                             gattdb_dult_non_owner,
                                             len,
                                             (const uint8_t *) &response);
  if (status) {
    DULT_LOG_ERROR("%s: Failed to send indication: %#lx", __func__, status);
    return SL_STATUS_FAIL;
  }
  return status;
}

/**
 * @brief
 *
 *
 * @return sl_status_t SL_STATUS_OK in case of success
 */
static sl_status_t get_battery_level_handler(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t status;
  sl_dult_non_owner_read_resp_t response = {0};

  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  response.opcode = SLI_DULT_OC_GET_BATTERY_LEVEL_RESPONSE;

  size_t len;
  response.data.battery_level = dult_ctx->battery_level;
  len = sizeof(sl_dult_opcode_t) + sizeof(sl_dult_battery_level);

  /* Send response */
  DULT_LOG_DEBUG("%s: %d bytes Battery Level ", __func__, len);
  status = sl_bt_gatt_server_send_indication(dult_ctx->connection,
                                             gattdb_dult_non_owner,
                                             len,
                                             (const uint8_t *) &response);
  if (status) {
    DULT_LOG_ERROR("%s: Failed to send indication: %#lx", __func__, status);
    return SL_STATUS_FAIL;
  }
  return status;
}

/**
 * @brief Get the identifier handler object
 *
 * Identifier Command and its response is described at:
 * https://www.ietf.org/archive/id/draft-ledvina-dult-accessory-protocol-00.html#name-identifier-payload
 *
 * @param connection The BLE connection handle
 * @param characteristic The characteristic handle
 * @return sl_status_t SL_STATUS_OK in case of success
 */
static sl_status_t get_identifier_handler(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t status;
  sl_dult_non_owner_read_resp_t response = {0};

  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (!dult_ctx->id_read_ctx.read_state_enabled) {
    DULT_LOG_INFO("%s: Identifier read state is not active", __func__);
    /**
     * If the accessory is not in identifier read state, it MUST
     * send Command_Response (Section 3.13.4.1.1) with the Invalid_command
     * as the ResponseStatus
     */
    status = send_command_response(dult_ctx->connection,
                                   gattdb_dult_non_owner,
                                   SLI_DULT_OC_NON_OWNER_GET_ID,
                                   SL_DULT_CMD_RESP_STATUS_INVALID_COMMAND);
    if (status) {
      DULT_LOG_ERROR("%s: Failed to send command response", __func__);
    }
    return status;
  }

  response.opcode = SLI_DULT_OC_NON_OWNER_GET_ID_RSP;

  size_t len =
      dult_ctx->callback.on_identifier_read(response.data.identifier,
                                            SL_DULT_MAX_IDENTIFIER_SIZE,
                                            dult_ctx->callback.context);
  if (len == 0) {
    DULT_LOG_ERROR("%s: Failed to get identifier", __func__);
    return SL_STATUS_FAIL;
  }
  len = sizeof(sl_dult_opcode_t) + len;

  /* Send response */
  DULT_LOG_DEBUG("%s: %d bytes identifier", __func__, len);
  status = sl_bt_gatt_server_send_indication(dult_ctx->connection,
                                             gattdb_dult_non_owner,
                                             len,
                                             (const uint8_t *) &response);
  if (status) {
    DULT_LOG_ERROR("%s: Failed to send indication: %#lx", __func__, status);
  }
  return status;
}

#if SL_DULT_CONFIG_DYNAMIC_GATT
/**
 * @brief Add non user service
 *
 * @return sl_status_t SL_STATUS_OK in case of success
 */

sl_status_t sli_dult_non_user_add_service(void)
{
  sl_status_t sc;
  memcpy(dult_non_user_characteristic_uuid128.data,
         dult_non_user_characteristic_uuid,
         16);

  // Create a session for the database
  sc = sl_bt_gattdb_new_session(&dult_non_user_session);
  if (sc != SL_STATUS_OK) {
    DULT_LOG_ERROR("sl_bt_gattdb_new_session error = %#lx", sc);
  }
  // Add the service to the database as an advertised primary service
  sc = sl_bt_gattdb_add_service(dult_non_user_session,
                                sl_bt_gattdb_primary_service,
                                SL_BT_GATTDB_ADVERTISED_SERVICE,
                                16,
                                dult_non_user_service_uuid,
                                &dult_non_user_service);
  if (sc != SL_STATUS_OK) {
    DULT_LOG_ERROR("sl_bt_gattdb_add_service error = %#lx", sc);
  }

  // Define the characteristic properties
  uint8_t property =
      (SL_BT_GATTDB_CHARACTERISTIC_READ | SL_BT_GATTDB_CHARACTERISTIC_INDICATE |
       SL_BT_GATTDB_CHARACTERISTIC_WRITE);

  // Add the characteristic
  sc = sl_bt_gattdb_add_uuid128_characteristic(dult_non_user_session, // Database update session
                                               dult_non_user_service, // Service handle
                                               property,              // Characteristic properties
                                               0,                     // Security requirement (none in this case)
                                               0,                     // No additional flags
                                               dult_non_user_characteristic_uuid128, // 128-bit UUID
                                               sl_bt_gattdb_user_managed_value,      // Value type
                                               0,                                    // Maximum length
                                               0,                     // Initial value length (0 for empty)
                                               NULL,                  // Initial value (NULL for empty)
                                               &gattdb_dult_non_owner // Output: characteristic handle
  );

  if (sc != SL_STATUS_OK) {
    DULT_LOG_ERROR("sl_bt_gattdb_add_uuid128_characteristic error = %#lx", sc);
    gattdb_dult_non_owner = 0xFFFF;
  }

  // Activate the new service
  sc = sl_bt_gattdb_start_service(dult_non_user_session, dult_non_user_service);
  if (sc != SL_STATUS_OK) {
    DULT_LOG_ERROR("sl_bt_gattdb_start_service error = %#lx", sc);
  }

  // Activate the new characteristic
  sl_bt_gattdb_start_characteristic(dult_non_user_session,
                                    dult_non_user_characteristic);
  if (sc != SL_STATUS_OK) {
    DULT_LOG_ERROR("sl_bt_gattdb_start_characteristic error = %#lx", sc);
  }

  // Save changes and close the database editing session
  sl_bt_gattdb_commit(dult_non_user_session);
  if (sc != SL_STATUS_OK) {
    DULT_LOG_ERROR("sl_bt_gattdb_commit error = %#lx", sc);
  }
  return sc;
}
#endif

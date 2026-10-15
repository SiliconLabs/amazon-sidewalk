/***************************************************************************//**
 * @file  sl_dult.h
 * @brief DULT header file internal use
 * This file is used internally of DULT and does not required to include from
 * application
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

#ifndef SLI_DULT_H
#define SLI_DULT_H

#include "sl_dult_config.h"
#include "sl_dult.h"
#include "sl_status.h"
#include "sl_bt_api.h"
#include "sli_dult_adaptation.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#ifdef SL_CATALOG_APP_LOG_PRESENT
#include "app_log.h"
#endif
#ifdef SL_CATALOG_APP_ASSERT_PRESENT
#include "app_assert.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SLI_DULT_ADV_SET_INVALID 0xFF

/** 3.12.1. Opcodes  */
// Required Opcodes
#define SLI_DULT_OC_GET_PRODUCT_DATA                             0x0003
#define SLI_DULT_OC_GET_PRODUCT_DATA_RESPONSE                    0x0803
#define SLI_DULT_OC_GET_MANUFACTURER_NAME                        0x0004
#define SLI_DULT_OC_GET_MANUFACTURER_NAME_RESPONSE               0x0804
#define SLI_DULT_OC_GET_MODEL_NAME                               0x0005
#define SLI_DULT_OC_GET_MODEL_NAME_RESPONSE                      0x0805
#define SLI_DULT_OC_GET_ACCESSORY_CATEGORY                       0x0006
#define SLI_DULT_OC_GET_ACCESSORY_CATEGORY_RESPONSE              0x0806
#define SLI_DULT_OC_GET_PROTOCOL_IMPLEMENTATION_VERSION          0x0007
#define SLI_DULT_OC_GET_PROTOCOL_IMPLEMENTATION_VERSION_RESPONSE 0x0807
#define SLI_DULT_OC_GET_ACCESSORY_CAPABILITIES                   0x0008
#define SLI_DULT_OC_GET_ACCESSORY_CAPABILITIES_RESPONSE          0x0808
#define SLI_DULT_OC_GET_NETWORK_ID                               0x0009
#define SLI_DULT_OC_GET_NETWORK_ID_RESPONSE                      0x0809
#define SLI_DULT_OC_GET_FIRMWARE_VERSION                         0x000A
#define SLI_DULT_OC_GET_FIRMWARE_VERSION_RESPONSE                0x080A

// Optional Opcodes
#define SLI_DULT_OC_GET_BATTERY_TYPE             0x000B
#define SLI_DULT_OC_GET_BATTERY_TYPE_RESPONSE    0x080B
#define SLI_DULT_OC_GET_BATTERY_LEVEL            0x000C
#define SLI_DULT_OC_GET_BATTERY_LEVEL_RESPONSE   0x080C
#define SLI_DULT_OC_GET_NETWORK_VERSION          0x000D
#define SLI_DULT_OC_GET_NETWORK_VERSION_RESPONSE 0x080D

// Reserved Opcodes
#define SLI_DULT_OC_RESERVED_OPCODE_MIN   0x000E
#define SLI_DULT_OC_RESERVED_OPCODE_MAX   0x005F
#define SLI_DULT_OC_RESERVED_RESPONSE_MIN 0x080E
#define SLI_DULT_OC_RESERVED_RESPONSE_MAX 0x085F

/**  3.13.4. Non-owner controls  */
// Required Opcodes
#define SLI_DULT_OC_NON_OWNER_SOUND_START 0x0300
#define SLI_DULT_OC_NON_OWNER_SOUND_STOP  0x0301
#define SLI_DULT_OC_NON_OWNER_CMD_RSP     0x0302
#define SLI_DULT_OC_NON_OWNER_SOUND_COMPL 0x0303
// Optional Opcodes
#define SLI_DULT_OC_NON_OWNER_GET_ID     0x0404
#define SLI_DULT_OC_NON_OWNER_GET_ID_RSP 0x0405

// -----------------------------------------------------------------------------
// Logging

#ifdef SL_CATALOG_APP_LOG_PRESENT
#define DULT_LOG_INFO(...)                                                     \
  do {                                                                         \
    app_log_info("[DULT]"__VA_ARGS__);                                         \
  } while (0)
#define DULT_LOG_ERROR(...)                                                    \
  do {                                                                         \
    app_log_error("[DULT]"__VA_ARGS__);                                        \
  } while (0)
#define DULT_LOG_DEBUG(...)                                                    \
  do {                                                                         \
    app_log_debug("[DULT]"__VA_ARGS__);                                        \
  } while (0)
#define DULT_ADV_LOG_HEXDUMP_ADDRESS(x, y)                                     \
  do {                                                                         \
    app_log_hexdump_info_s(APP_LOG_HEXDUMP_SEPARATOR_COLON, x, y);             \
    app_log_nl();                                                              \
  } while (0)
#else
#define DULT_LOG_INFO(...)
#define DULT_LOG_ERROR(...)
#define DULT_LOG_DEBUG(...)
#define DULT_ADV_LOG_HEXDUMP_ADDRESS(x, y)
#endif

#ifdef SL_CATALOG_APP_ASSERT_PRESENT
#define SLI_DULT_ASSERT_ST app_assert_status
#else
#define SLI_DULT_ASSERT_ST(status)
#endif

/** 3.4.2. Location-enabled advertisement payload format */
// Structure to represent a generic Flags TLV (Type-Length-Value)
typedef struct {
  uint8_t length; // Length of the TLV
  uint8_t type;   // Type of the TLV
  uint8_t value;  // Value of the TLV
} __attribute__((packed)) sl_dult_flag_tlv_t;

/** 3.6. Service data TLV  */
// Structure to represent Service Data TLV
typedef struct {
  uint8_t length; // Length of the TLV
  uint8_t type;   // Type of the TLV (e.g., 0x16)
  uint16_t value; // Value of the TLV (e.g., 0xFCB2)
} __attribute__((packed)) sl_dult_service_data_tlv_t;

// Structure to represent Near-owner bit and Reserved bits
typedef struct {
  uint8_t near_owner_bit : 1; // 1-bit for near-owner status
  uint8_t reserved_bits : 7;  // 7 bits reserved for future use
} __attribute__((packed)) sl_dult_near_owner_reserved_t;

// Structure to represent Proprietary Payload
typedef struct {
  uint8_t *payload; // Pointer to the payload data (variable length)
  uint16_t length;  // Length of the payload in bytes
} __attribute__((packed)) sl_dult_proprietary_payload_t;

typedef struct {
  sl_dult_mac_address_t mac;                // MAC address (can be random)
  sl_dult_flag_tlv_t *flags_tlv;            // Pointer to Flags TLV (Optional)
  sl_dult_service_data_tlv_t service_data;  // Service Data TLV (Required)
  uint8_t network_id;                       // Network ID (Required)
  sl_dult_near_owner_reserved_t near_owner; // Near-owner bit + Reserved bits
  sl_dult_proprietary_payload_t
      proprietary; // Proprietary company payload (Optional, variable length)
} __attribute__((packed)) sli_dult_adv_payload_t;

// Advertising context
typedef struct {
  sli_dult_timer_t rotation_timer;
  bool started;
  uint8_t handle;
  bool require_rotation;
} sli_dult_adv_ctx_t;

// Identifier read state context
typedef struct {
  volatile bool read_state_enabled;
  sl_sleeptimer_timer_handle_t read_state_timer;
} sli_dult_id_read_state_t;

typedef enum {
  SLI_DULT_SOUND_STATE_IDLE,
  SLI_DULT_SOUND_STATE_START_REQUEST,
  SLI_DULT_SOUND_STATE_START_ACK,
  SLI_DULT_SOUND_STATE_STOP_REQUEST,
  SLI_DULT_SOUND_STATE_EXTERNAL_TAKEOVER,
} sli_dult_sound_state_t;

typedef struct {
  bool is_initialized;
  sli_dult_sound_state_t state;
  uint32_t start_time;
  bool active;
  sl_dult_non_owner_find_event_src_t src;
} sli_dult_sound_ctx_t;

typedef struct {
  bool is_initialized;
  bool is_enabled;
  uint8_t state;
  uint8_t sound_count;
  struct {
    sli_dult_timer_t motion_enable;
    sli_dult_timer_t poll;
    sli_dult_timer_t motion_poll_duration;
  } timer;
} sli_dult_motion_detection_ctx_t;

// DULT object context
typedef struct {
  sl_dult_accessory_info_t info;
  sl_dult_cb_t callback;
  sl_dult_near_owner_state_t near_owner_state;
  sli_dult_adv_ctx_t adv;
  sli_dult_id_read_state_t id_read_ctx;
  sli_dult_motion_detection_ctx_t motion_ctx;
  uint8_t connection;
  sli_dult_sound_ctx_t sound;
  sl_dult_battery_level battery_level;
} sli_dult_ctx_t;

typedef uint16_t sl_dult_opcode_t;
typedef uint16_t sl_cmd_resp_status_t;

/**
 * @brief Handle non-owner operations.
 *
 * @param connection BLE connection that triggered the operation.
 * @param characteristic Non-Owner characteristic.
 * @param data Pointer to the data buffer.
 * @param len Length of the data buffer.
 * @return sl_status_t Status of the operation.
 */
sl_status_t sli_dult_non_owner_operation_handler(uint8_t handle,
                                                 uint8_t *data,
                                                 uint16_t len);

/**
 * @brief Get gattdb dult non owner.
 *
 * @return gattdb_dult_non_owner
 */
uint16_t sli_dult_get_non_owner_characteristic_handle(void);

/**
 * @brief Get the DULT callback object by handle
 *
 * @param handle The DULT handle
 * @return const sl_dult_cb_t* Pointer to the DULT callback object
 */
const sl_dult_cb_t *sli_dult_cb_get(uint8_t handle);

/**
 * @brief Handle Bluetooth events for DULT
 *
 * This function handles Bluetooth events related to the DULT module.
 *
 * @param evt Pointer to the Bluetooth event structure.
 */
void sli_bt_dult_on_event(sl_bt_msg_t *evt);

/**
 * @brief create advertising payload
 *
 * @param[in] dult_ctx The pointer to DULT context
 * @param[in] rotation The Bluetooth address shall be rotated
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sli_dult_adv_int_payload(sli_dult_ctx_t *dult_ctx, bool rotation);

/**
 * @brief Get DULT context by handle
 *
 * @param[in] handle The DULT handle
 * @param[out] dult The DULT context
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sli_dult_get_context(uint8_t handle, sli_dult_ctx_t **dult);

/**
 * @brief Get DULT context by connection
 *
 * @param[in] connection The BLE connection handle
 * @param[out] dult The DULT context
 * @return sl_status_t
 */
sl_status_t sli_dult_get_context_by_connection(uint8_t connection,
                                               sli_dult_ctx_t **dult);

void sli_dult_motion_detection_init(sli_dult_ctx_t *dult_ctx);
sl_status_t sli_dult_motion_detection_reset(sli_dult_ctx_t *dult_ctx);
void sli_dult_motion_near_owner_changed_notify(sli_dult_ctx_t *dult_ctx);
void sli_dult_motion_detection_sound_notify(sli_dult_ctx_t *dult_ctx,
                                            bool started);

void sli_dult_adv_near_owner_changed_notify(sli_dult_ctx_t *dult_ctx);
void sli_dult_adv_connection_closed_notify(sli_dult_ctx_t *dult_ctx);

#if SL_DULT_CONFIG_DYNAMIC_GATT
/**
 * @brief Add non user service
 *
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sli_dult_non_user_add_service(void);
#endif

/**
 * @brief Initialize the DULT
 *
 * @param[in] dult_ctx The pointer to DULT context
 */
void sli_dult_bt_sound_init(sli_dult_ctx_t *dult_ctx);

/**
 * @brief Reset sound context to the state after initialization
 * @param[in] dult_ctx The pointer to DULT context
 */
void sli_dult_bt_sound_reset(sli_dult_ctx_t *dult_ctx);

/**
 * @brief Sound start handle
 * @param[in] dult_ctx The pointer to DULT context
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sli_dult_bt_sound_start(sli_dult_ctx_t *dult_ctx);

/**
 * @brief Sound stop handle
 * @param[in] dult_ctx The pointer to DULT context
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sli_dult_bt_sound_stop(sli_dult_ctx_t *dult_ctx);

/**
 * @brief Motion sound start handle
 *
 * @param dult_ctx The pointer to DULT context
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sli_dult_motion_sound_start(sli_dult_ctx_t *dult_ctx);

/**
 * @brief Send sound command response to non-owner
 *
 * This function sends a command response for sound-related operations
 * (start/stop) to the non-owner device via GATT indication. It maps
 * the provided status to appropriate DULT command response status codes.
 *
 * @param[in] dult_ctx Pointer to the DULT context containing connection info
 * @param[in] opcode The sound command opcode (e.g., SLI_DULT_OC_NON_OWNER_SOUND_START)
 * @param[in] status The operation status to be mapped to DULT response status
 *
 * @return sl_status_t Status of the operation:
 *         - SL_STATUS_OK: Command response sent successfully
 *         - SL_STATUS_NULL_POINTER: dult_ctx is NULL
 *         - Other: Error from underlying send_command_response function
 *
 * @note This function is typically called after sound start/stop operations
 *       to notify the non-owner device of the operation result.
 */
sl_status_t sli_dult_sound_cmd_resp(sli_dult_ctx_t *dult_ctx,
                                    sl_dult_opcode_t opcode,
                                    sl_status_t status);

/**
 * @brief Send sound completion indication to non-owner
 *
 * @param dult_ctx The pointer to DULT context
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sli_dult_non_owner_send_sound_cmpl(sli_dult_ctx_t *dult_ctx);

/**
 * @brief Stop identifier payload
 *
 * @param dult_ctx The pointer to DULT context
 * @note This function stops the timer and resets the read state enabled flag
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sli_dult_identifier_payload_stop(sli_dult_ctx_t *dult_ctx);

#ifdef __cplusplus
}
#endif

#endif // SLI_DULT_H

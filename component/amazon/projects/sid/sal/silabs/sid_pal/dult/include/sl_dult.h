/***************************************************************************//**
 * @file  sl_dult.h
 * @brief DULT header file
 * Detecting Unwanted Location Trackers
 * APIs for DULT to comply with the specification at
 * https://datatracker.ietf.org/doc/draft-ledvina-dult-accessory-protocol/
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
#ifndef SL_DULT_H
#define SL_DULT_H

#include <stdbool.h>
#include <stddef.h>
#include "sl_status.h"
#include "sl_enum.h"
#include "sl_bgapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 3.3. Location Tracking
 * Near-owner state can take on two values, either near-owner mode or separated mode.
 * Near-owner mode is denoted as the opposite of separated mode.
*/
typedef enum {
  SL_DULT_STATE_SEPARATED,
  SL_DULT_STATE_NEAR_OWNER,
  SL_DULT_STATE_COUNT,
} sl_dult_near_owner_state_t;

/** 3.5. MAC address */
typedef bd_addr sl_dult_mac_address_t;

/** 3.7.  Network ID *
 * The 1-byte Network ID SHALL be set based on a registered value for the manufacturer,
 * as defined in (TODO: Section Finding Network Registry has been removed).
 *
 */
/** The IDs are in section 10.1.1. Temporary Registry , they will be corrected when DULT is ratified ) */
typedef enum {
  SL_DULT_NETWORK_ID_UNKNOW   = 0x00,
  SL_DULT_NETWORK_ID_APPLE    = 0x01,
  SL_DULT_NETWORK_ID_GOOGLE   = 0x02,
  SL_DULT_NETWORK_ID_SIDEWALK = 0x03,
} sl_dult_network_id_t;

/** 3.12.1.6. Accessory capabilities
 * The Accessory Capabilities operand enumerates the various capabilities
 * supported on the accessory as defined in
 * https://www.ietf.org/archive/id/draft-ledvina-dult-accessory-protocol-00.html#_table-accessory-capability
 */
typedef enum {
  /** Bit mask for the play sound capability. */
  SL_DULT_ACCESSORY_CAPABILITY_PLAY_SOUND = (0x01 << 0),
  /** Bit mask for the motion detector unwanted tracking capability. */
  SL_DULT_ACCESSORY_CAPABILITY_MOTION_DETECTOR_UT = (0x01 << 1),
  /** Bit mask for the identifier lookup by NFC capability. */
  SL_DULT_ACCESSORY_CAPABILITY_ID_LOOKUP_NFC = (0x01 << 2),
  /** Bit mask for the identifier lookup by BLE capability. */
  SL_DULT_ACCESSORY_CAPABILITY_ID_LOOKUP_BLE = (0x01 << 3),
} sl_dult_accessory_capability_t;

typedef enum {
  /** Indicates that the battery is powered */
  SL_DULT_BATTERY_POWERED = 0,
  /** Indicates that the battery is non rechargeable */
  SL_DULT_BATTERY_NON_RECHARGEABLE = 1,
  /** Indicates that the battery is rechargeable */
  SL_DULT_BATTERY_RECHARGEABLE = 2,
} sl_dult_battery_type;

typedef enum {
  /** Indicates that the battery is full */
  SL_DULT_BATTERY_FULL = 0,
  /** Indicates that the battery is medium */
  SL_DULT_BATTERY_MEDIUM = 1,
  /** Indicates that the battery is low */
  SL_DULT_BATTERY_LOW = 2,
  /** Indicates that the battery is critically low */
  SL_DULT_BATTERY_CRITICALLY_LOW = 3,
} sl_dult_battery_level;

/** 3.12.1.5. Firmware version
 * The Protocol Implementation Version operand contains a value indicating an
 * implementation version of these protocols.
 *  The Protocol Implementation string SHALL use the x[.y[.z]] format where :
 * <x> is the major version number, required.
 * <y> is the minor version number, required if it is non zero or if <z> is present.
 * <z> is the revision version number, required if non zero.
 */
typedef struct {
  /** Protocol revision. */
  uint8_t revision;
  /** Minor firmware version. */
  uint8_t minor;
  /** Major firmware version. */
  uint16_t major;
} sl_dult_protocol_implementation_version_t;

/** 3.12.1.8. Firmware version
 * The Firmware Version describes the current firmware version running on the accessory.
 *  The firmware revision string SHALL use the x[.y[.z]] format where :
 * <x> is the major version number, required.
 * <y> is the minor version number, required if it is non zero or if <z> is present.
 * <z> is the revision version number, required if non zero.
 */
typedef struct {
  /** Firmware revision. */
  uint8_t revision;
  /** Minor firmware version. */
  uint8_t minor;
  /** Major firmware version. */
  uint16_t major;
} sl_dult_firmware_version_t;

/**
 * Describes the device accessory information.
 */
typedef struct {
  /** Pointer to the product data array */
  const uint8_t *product_data;

  /** Pointer to the manufacturer name array, end with NULL character  */
  const uint8_t *manufacturer_name;

  /** Pointer to the model name array, end with NULL character  */
  const uint8_t *model_name;

  /** Accessory capabilities, see #sl_dult_accessory_capability_t */
  sl_dult_accessory_capability_t capabilities;

  /** Protocol implementation version of the device ,see #sl_dult_protocol_implementation_version_t */
  sl_dult_protocol_implementation_version_t protocol_implementation_version;

  /** Firmware version of the device ,see #sl_dult_firmware_version_t */
  sl_dult_firmware_version_t firmware_version;

  /** Category of the device see DULT 4. Accessory Category Value for the
   * list of possible values
   */
  uint8_t category;

  /** Network id, see #sl_dult_network_id_t */
  sl_dult_network_id_t network_id;

  /* The type of battery in the device */
  sl_dult_battery_type battery_type;

  /** TLV flags data, the DULT spec v01 does not define for now*/
  uint8_t adv_tlv_flags;
  /** Pointer to the Company data array, end with NULL character*/
  const uint8_t *adv_company_data;

  /** Pointer to the Advertising set handle ,see #sl_bt_advertiser_create_set
   * Set NULL then DULT manages it own adv set.
   */
  uint8_t *adv_handle;

} sl_dult_accessory_info_t;

/**
 * Describes the Non-Owner hardware to trigger for
 * locating the device
 */
typedef enum {
  /** Trigger sound hardware to locate device */
  SL_DULT_NON_OWNER_FIND_EVENT_TYPE_SOUND = 1,
  /** Trigger lights to locate device UNSUPPORTED*/
  SL_DULT_NON_OWNER_FIND_EVENT_TYPE_LIGHTS = 2,
  /** Trigger haptic feedback to locate device UNSUPPORTED*/
  SL_DULT_NON_OWNER_FIND_EVENT_TYPE_HAPTIC = 3,
} sl_dult_non_owner_find_event_type_t;

/**
 * Describes the source of the Non-Owner find event
 */
typedef enum {
  /** Event was generated from BLE GATT opcode */
  SL_DULT_NON_OWNER_FIND_EVENT_SRC_BT_GATT = 1,
  /** Event was generated from the motion detection */
  SL_DULT_NON_OWNER_FIND_EVENT_SRC_MOTION_DETECTION = 2,
  /** Event was generated from an external API */
  SL_DULT_NON_OWNER_FIND_EVENT_SRC_EXTERNAL = 3,
  /** Number of events */
  SL_DULT_NON_OWNER_FIND_EVENT_SRC_COUNT,
} sl_dult_non_owner_find_event_src_t;

/**
 * Describes the control action to take for the
 * Non-Owner find event
 */
typedef enum {
  /** Start the non-owner find hardware */
  SL_DULT_NON_OWNER_FIND_EVENT_ACTION_START = 1,
  /** Stop the non-owner find hardware */
  SL_DULT_NON_OWNER_FIND_EVENT_ACTION_STOP = 2,
} sl_dult_non_owner_find_event_action_t;

/**
 * Describes the Non-Owner Find Event
 */
typedef struct sl_dult_non_owner_find_event {
  /** Type of the event */
  sl_dult_non_owner_find_event_type_t type;
  /** Source of the event */
  sl_dult_non_owner_find_event_src_t src;
  /** Action that needs to be taken for the event */
  sl_dult_non_owner_find_event_action_t action;
} sl_dult_non_owner_find_event_t;

/**
 * Describes the action of motion detection event
 */
typedef enum {
  /** Start motion detection */
  SL_DULT_MOTION_DETECTION_EVENT_TYPE_START = 1,
  /** Stop motion detection */
  SL_DULT_MOTION_DETECTION_EVENT_TYPE_STOP = 2,
  /** Retrieve the state of motion detection */
  SL_DULT_MOTION_DETECTION_EVENT_TYPE_GET_MOTION_STATUS = 3,
} sl_dult_motion_detection_event_action_t;

/** Callback functions register struct */
typedef struct sl_dult_cb {
  /** User context data */
  void *context;

  /**
   * Callback that is invoked whenever the owner proximity changes
   *
   * The dult calls this callback to let the user know that it near to
   * owner or it has been separated from the owner.
   *
   * @param[in] state #sl_dult_near_owner_state_t value indicating the owner proximity state
   * @param[in] context The context pointer given in sl_dult_cb.context
   */
  void (*on_near_owner_state_change)(sl_dult_near_owner_state_t state,
                                     void *context);

  /**
   * Callback that is invoked when non-owner find event is triggered
   *
   * The dult calls this callback to let the user know that a non-owner find
   * event has been triggered either from ble_gatt or motion detector
   *
   * @param[in] event The #sl_dult_non_owner_find_event_t
   * @param[in] context The context pointer given in sl_dult_cb_t.context
   */
  void (*on_non_owner_find_event)(const sl_dult_non_owner_find_event_t *event,
                                  void *context);

  /**
   * Callback that is invoked when motion-detection event is triggered
   *
   * @param[in] event The #sl_dult_motion_detection_event_action_t
   * @param[in] context The context pointer given in sl_dult_cb_t.context
   */
  void (*on_motion_detection_event)(
      const sl_dult_motion_detection_event_action_t *event,
      void *context);

  /**
   * Callback that is invoked when a non-owner wants to read the Identifier Payload
   *
   * @param[out] buf Buffer that user will fill with the Identifier Payload
   * @param[in] len The len of the buffer
   * @param[in] context The context pointer given in sl_dult_cb_t.context
   *
   * @returns 0 in case of an error else the filled up buffer len
   */
  size_t (*on_identifier_read)(uint8_t *buf, size_t len, void *context);

} sl_dult_cb_t;

/**
 * @brief Initialize the DULT
 *
 * @param[in] ctx The DULT owner context
 * @param[out] handle Pointer the DULT handle
 * @return[out] sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_init(const sl_dult_accessory_info_t *info, uint8_t *handle);

/**
 * @brief De-initialize the DULT
 *
 * @param[in] handle The DULT handle
 * @return sl_status_t
 */
sl_status_t sl_dult_deinit(uint8_t handle);

/**
 * @brief DULT advertisement start
 *
 * @param[in] handle The DULT handle
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_start(uint8_t handle);

/**
 * @brief DULT advertisement stop
 *
 * @param[in] handle The DULT handle
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_stop(uint8_t handle);

/**
 * @brief Enters DULT Read Identifier State
 *
 *  3.12.4.2. Identifier Payload
 *  The identifier read state MUST be enabled for 5 minutes once the user action
 *  on the accessory is successfully performed. When the accessory is in this
 * mode, it MUST respond with Get_Identifier_Response opcode and Identifier
 * Payload operand.
 *
 * @param[in] handle The DULT handle
 * @returns sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_identifier_payload_enter(uint8_t handle);

/**
 * @brief Update the find event action status
 *
 *  This Api should be called with the status of once the Application receives a
 * sl_dult_cb_t.on_non_owner_find_event
 *
 * Note: if application sets event type is Sound, sound_active=true means Sound start successfully
 * sound_active=false means Sound stop successfully
 *
 * @param[in] handle The DULT handle
 * @param[in] sound_active The status of on_non_owner_find_event
 * @param[in] event The event of on_non_owner_find_event
 * @return[out] sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_update_find_event_status(
    uint8_t handle,
    bool sound_active,
    const sl_dult_non_owner_find_event_src_t src,
    const sl_dult_non_owner_find_event_type_t type);

/**
 * @brief Set the current owner proximity state
 *
 * @param[in] handle The DULT handle
 * @param[in] state Indicates if the device is near owner or separated
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_set_near_owner_state(uint8_t handle,
                                         sl_dult_near_owner_state_t state);

/**
 * @brief Get the current owner proximity state
 *
 * @param[in] handle The DULT handle
 * @param[out] state Pointer to store the current owner proximity state
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_get_near_owner_state(uint8_t handle, sl_dult_near_owner_state_t *state);

/**
 * @brief Register to DULT callback functions
 *
 * @param[in] handle The DULT handle
 * @param[in] cb_func
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_cb_register(uint8_t handle, const sl_dult_cb_t *cb_func);

/**
 * @brief Update the status of motion detection
 *
 * The motion detection implementation should be followed by 3.13.2.1. Implementation
 *
 * @param[in] handle The DULT handle
 * @param[in] status True if motion has been detected else false
 * @return[out] sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_update_motion_detection(uint8_t handle, bool status);

/**
 * @brief Enable/Disable trigger non-owner find event based on motion detection
 *
 * The motion detection implementation should be followed by 3.13.2.1. Implementation
 *
 * @param[in] handle The DULT handle
 * @param[in] set True to enable motion detection, false to disable motion detection
 * @return[out] sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_set_motion_detection(uint8_t handle, bool set);

/**
 * @brief Update the level of battery
 *
 * @param[in] handle The DULT handle
 * @param[in] current level of battery
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_set_battery_level(uint8_t handle,
                                      sl_dult_battery_level current_value);

#ifdef __cplusplus
}
#endif

#endif // SL_DULT_H

/*
 * Copyright 2024 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <sid_pal_dult_ifc.h>
#include <sid_pal_log_ifc.h>

#include "sid_error.h"
#include "sl_dult.h"

#include <sl_status.h>

#include <stdint.h>

static uint8_t dult_handle;
static struct sid_detect_unwanted_location_tracker_event_callbacks *sid_callbacks;

static void on_near_owner_state_change(sl_dult_near_owner_state_t state, void *context);
static void on_non_owner_find_event(const sl_dult_non_owner_find_event_t *event, void *context);
static void on_motion_detection_event(const sl_dult_motion_detection_event_action_t *event,
                                      void *context);

static sl_dult_cb_t sl_dult_cb = {
    .on_near_owner_state_change = on_near_owner_state_change,
    .on_non_owner_find_event = on_non_owner_find_event,
    .on_motion_detection_event = on_motion_detection_event,
};

static sid_error_t status_to_serr(sl_status_t status);

static sid_error_t status_to_serr(sl_status_t status)
{
    sid_error_t ret = (status == SL_STATUS_OK)                  ? SID_ERROR_NONE                :
                      (status == SL_STATUS_IN_PROGRESS)         ? SID_ERROR_IN_PROGRESS         :
                      (status == SL_STATUS_INVALID_PARAMETER)   ? SID_ERROR_INVALID_ARGS        :
                      (status == SL_STATUS_ALREADY_INITIALIZED) ? SID_ERROR_ALREADY_INITIALIZED :
                      (status == SL_STATUS_NOT_INITIALIZED)     ? SID_ERROR_UNINITIALIZED       :
                      SID_ERROR_GENERIC;

    return ret;
}

static enum sid_owner_proximity_state to_sid_owner_proximity(sl_dult_near_owner_state_t state)
{
    switch (state) {
        case SL_DULT_STATE_SEPARATED:  return SID_OWNER_PROXIMITY_STATE_SEPARATED;
        case SL_DULT_STATE_NEAR_OWNER: return SID_OWNER_PROXIMITY_STATE_NEAR;
        case SL_DULT_STATE_COUNT:      return SID_OWNER_PROXIMITY_STATE_INVALID;
        default:                       return SID_OWNER_PROXIMITY_STATE_INVALID;
    }
}

static sl_dult_near_owner_state_t from_sid_owner_proximity(enum sid_owner_proximity_state state)
{
    switch (state) {
        case SID_OWNER_PROXIMITY_STATE_SEPARATED:   return SL_DULT_STATE_SEPARATED;
        case SID_OWNER_PROXIMITY_STATE_NEAR:        return SL_DULT_STATE_NEAR_OWNER;
        case SID_OWNER_PROXIMITY_STATE_INVALID:     return SL_DULT_STATE_COUNT;
        default:                                    return SL_DULT_STATE_COUNT;
    }
}

static enum sid_non_owner_find_event_type to_sid_non_owner_find_event_type(
    sl_dult_non_owner_find_event_type_t type)
{
    switch (type) {
        case SL_DULT_NON_OWNER_FIND_EVENT_TYPE_SOUND:  return SID_NON_OWNER_FIND_EVENT_TYPE_SOUND;
        case SL_DULT_NON_OWNER_FIND_EVENT_TYPE_LIGHTS: return SID_NON_OWNER_FIND_EVENT_TYPE_LIGHTS;
        case SL_DULT_NON_OWNER_FIND_EVENT_TYPE_HAPTIC: return SID_NON_OWNER_FIND_EVENT_TYPE_HAPTIC;
        default:                                       return SID_NON_OWNER_FIND_EVENT_TYPE_SOUND;
    }
}

static enum sid_non_owner_find_event_src to_sid_non_owner_find_event_src(
    sl_dult_non_owner_find_event_src_t src)
{
    switch (src) {
        case SL_DULT_NON_OWNER_FIND_EVENT_SRC_BT_GATT:          return SID_NON_OWNER_FIND_EVENT_SRC_BT_GATT;
        case SL_DULT_NON_OWNER_FIND_EVENT_SRC_MOTION_DETECTION: return SID_NON_OWNER_FIND_EVENT_SRC_MOTION_DETECTION;
        case SL_DULT_NON_OWNER_FIND_EVENT_SRC_EXTERNAL:         return SID_NON_OWNER_FIND_EVENT_SRC_EXTERNAL;
        case SL_DULT_NON_OWNER_FIND_EVENT_SRC_COUNT:            return SID_NON_OWNER_FIND_EVENT_SRC_COUNT;
        default:                                                return SID_NON_OWNER_FIND_EVENT_SRC_COUNT;
    }
}

static enum sid_non_owner_find_event_action to_sid_non_owner_find_event_action(
    sl_dult_non_owner_find_event_action_t action)
{
    switch (action) {
        case SL_DULT_NON_OWNER_FIND_EVENT_ACTION_START: return SID_NON_OWNER_FIND_EVENT_ACTION_START;
        case SL_DULT_NON_OWNER_FIND_EVENT_ACTION_STOP:  return SID_NON_OWNER_FIND_EVENT_ACTION_STOP;
        default:                                        return SID_NON_OWNER_FIND_EVENT_ACTION_START;
    }
}

static enum sid_motion_detection_event_action to_sid_motion_detection_event_action(
    sl_dult_motion_detection_event_action_t action)
{
    switch (action) {
        case SL_DULT_MOTION_DETECTION_EVENT_TYPE_START:             return SID_MOTION_DETECTION_EVENT_TYPE_START;
        case SL_DULT_MOTION_DETECTION_EVENT_TYPE_STOP:              return SID_MOTION_DETECTION_EVENT_TYPE_STOP;
        case SL_DULT_MOTION_DETECTION_EVENT_TYPE_GET_MOTION_STATUS: return SID_MOTION_DETECTION_EVENT_TYPE_GET_MOTION_STATUS;
        default:                                                    return SID_MOTION_DETECTION_EVENT_TYPE_START;
    }
}

static sl_dult_non_owner_find_event_type_t from_sid_non_owner_find_event_type(
    enum sid_non_owner_find_event_type type)
{
    switch (type) {
        case SID_NON_OWNER_FIND_EVENT_TYPE_SOUND:   return SL_DULT_NON_OWNER_FIND_EVENT_TYPE_SOUND;
        case SID_NON_OWNER_FIND_EVENT_TYPE_LIGHTS:  return SL_DULT_NON_OWNER_FIND_EVENT_TYPE_LIGHTS;
        case SID_NON_OWNER_FIND_EVENT_TYPE_HAPTIC:  return SL_DULT_NON_OWNER_FIND_EVENT_TYPE_HAPTIC;
        default:                                    return SL_DULT_NON_OWNER_FIND_EVENT_TYPE_SOUND;
    }
}

static sl_dult_non_owner_find_event_src_t from_sid_non_owner_find_event_src(
    enum sid_non_owner_find_event_src src)
{
    switch (src) {
        case SID_NON_OWNER_FIND_EVENT_SRC_BT_GATT:          return SL_DULT_NON_OWNER_FIND_EVENT_SRC_BT_GATT;
        case SID_NON_OWNER_FIND_EVENT_SRC_MOTION_DETECTION: return SL_DULT_NON_OWNER_FIND_EVENT_SRC_MOTION_DETECTION;
        case SID_NON_OWNER_FIND_EVENT_SRC_EXTERNAL:         return SL_DULT_NON_OWNER_FIND_EVENT_SRC_EXTERNAL;
        case SID_NON_OWNER_FIND_EVENT_SRC_COUNT:
        default:                                            return SL_DULT_NON_OWNER_FIND_EVENT_SRC_COUNT;
    }
}

static sl_dult_battery_level from_sid_battery_level(
    enum sid_detect_unwanted_location_tracker_accessory_battery_level level)
{
    switch (level) {
        case SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_BATTERY_LEVEL_FULL:           return SL_DULT_BATTERY_FULL;
        case SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_BATTERY_LEVEL_MEDIUM:         return SL_DULT_BATTERY_MEDIUM;
        case SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_BATTERY_LEVEL_LOW:            return SL_DULT_BATTERY_LOW;
        case SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_BATTERY_LEVEL_CRITICALLY_LOW: return SL_DULT_BATTERY_CRITICALLY_LOW;
        default:                                                                          return SL_DULT_BATTERY_FULL;
    }
}

static sl_dult_battery_type from_sid_battery_type(
    enum sid_detect_unwanted_location_tracker_accessory_battery_type type)
{
    switch (type) {
        case SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_BATTERY_POWERED:            return SL_DULT_BATTERY_POWERED;
        case SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_BATTERY_NON_RECHARGEABLE:   return SL_DULT_BATTERY_NON_RECHARGEABLE;
        case SID_DETECT_UNWANTED_LOCATION_TRACKER_ACCESSORY_BATTERY_RECHARGEABLE:       return SL_DULT_BATTERY_RECHARGEABLE;
        default:                                                                        return SL_DULT_BATTERY_POWERED;
    }
}

static void on_near_owner_state_change(sl_dult_near_owner_state_t state, void *context)
{
    sid_callbacks->on_owner_proximity_change(to_sid_owner_proximity(state), context);
}

static void on_non_owner_find_event(const sl_dult_non_owner_find_event_t *event, void *context)
{
    const struct sid_non_owner_find_event sid_event = {
        .type   = to_sid_non_owner_find_event_type(event->type),
        .src    = to_sid_non_owner_find_event_src(event->src),
        .action = to_sid_non_owner_find_event_action(event->action),
    };
    sid_callbacks->on_non_owner_find_event(&sid_event, context);
}

static void on_motion_detection_event(const sl_dult_motion_detection_event_action_t *event,
                                        void *context)
{
    const struct sid_motion_detection_event sid_event = {
        .action = to_sid_motion_detection_event_action(*event),
    };
    sid_callbacks->on_motion_detection_event(&sid_event, context);
}

sid_error_t sid_pal_dult_start(void)
{
    return status_to_serr(sl_dult_start(dult_handle));
}

sid_error_t sid_pal_dult_stop(void)
{
    return status_to_serr(sl_dult_stop(dult_handle));
}

sid_error_t sid_pal_dult_identifier_payload_enter(void)
{
    return status_to_serr(sl_dult_identifier_payload_enter(dult_handle));
}

sid_error_t sid_pal_dult_update_motion_detection(bool status)
{
    return status_to_serr(sl_dult_update_motion_detection(dult_handle, status));
}

sid_error_t sid_pal_dult_set_motion_detection(bool set)
{
    return status_to_serr(sl_dult_set_motion_detection(dult_handle, set));
}

sid_error_t sid_pal_dult_update_find_event_status(bool status,
                                                  enum sid_non_owner_find_event_src src,
                                                  enum sid_non_owner_find_event_type type)
{
    if ((src == SID_NON_OWNER_FIND_EVENT_SRC_MOTION_DETECTION) ||
        (type != SID_NON_OWNER_FIND_EVENT_TYPE_SOUND)) {
        return SID_ERROR_NOSUPPORT;
    }

    return status_to_serr(sl_dult_update_find_event_status(dult_handle, status,
        from_sid_non_owner_find_event_src(src), from_sid_non_owner_find_event_type(type)));
}

sid_error_t sid_pal_dult_set_current_owner_proximity_state(enum sid_owner_proximity_state state)
{
    return status_to_serr(sl_dult_set_near_owner_state(dult_handle, from_sid_owner_proximity(state)));
}

sid_error_t sid_pal_dult_set_battery_level(enum sid_detect_unwanted_location_tracker_accessory_battery_level level)
{
    return status_to_serr(sl_dult_set_battery_level(dult_handle, from_sid_battery_level(level)));
}

sid_error_t sid_pal_dult_init(const struct sid_detect_unwanted_location_tracker_accessory_info *info,
                              struct sid_detect_unwanted_location_tracker_event_callbacks *cbs)
{
    sid_error_t ret;

    const sl_dult_accessory_info_t acc_info = {
        .product_data = info->product_data,
        .manufacturer_name = info->manufacturer_name,
        .model_name = info->model_name,
        .capabilities = info->capabilities,
        .firmware_version =
            {
                .major = sid_detect_unwanted_location_tracker_firmware_major_version_from_firmware_version(
                    info->firmware_version),
                .minor = sid_detect_unwanted_location_tracker_firmware_minor_version_from_firmware_version(
                    info->firmware_version),
                .revision = sid_detect_unwanted_location_tracker_firmware_revision_from_firmware_version(
                    info->firmware_version),
            },
        .category = info->category,
        .network_id = info->network_id,
        .battery_type = from_sid_battery_type(info->battery),
        .adv_handle = NULL, // To avoid garbage data
    };

    ret = status_to_serr(sl_dult_init(&acc_info, &dult_handle));
    if (ret == SID_ERROR_NONE) {
        sid_callbacks = cbs;
        sl_dult_cb.context = sid_callbacks->context;
        sl_dult_cb.on_identifier_read = sid_callbacks->on_identifier_read;
        ret = status_to_serr(sl_dult_cb_register(dult_handle, &sl_dult_cb));
    }

    if (ret == SID_ERROR_NONE) {
        ret = status_to_serr(sl_dult_set_near_owner_state(dult_handle, SL_DULT_STATE_NEAR_OWNER));
    }
    return ret;
}

sid_error_t sid_pal_dult_deinit(void)
{
    return status_to_serr(sl_dult_deinit(dult_handle));
}
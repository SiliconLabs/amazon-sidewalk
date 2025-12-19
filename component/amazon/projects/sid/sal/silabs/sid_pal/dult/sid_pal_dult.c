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

static void on_near_owner_state_change(sl_dult_near_owner_state_t state, void *context)
{
    sid_callbacks->on_owner_proximity_change(state, context);
}

static void on_non_owner_find_event(const sl_dult_non_owner_find_event_t *event, void *context)
{
    sid_callbacks->on_non_owner_find_event((struct sid_non_owner_find_event *)event, context);
}

static void on_motion_detection_event(const sl_dult_motion_detection_event_action_t *event,
                                        void *context)
{
    sid_callbacks->on_motion_detection_event((struct sid_motion_detection_event *)event, context);
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

    return status_to_serr(sl_dult_update_find_event_status(dult_handle, status, src, type));
}

sid_error_t sid_pal_dult_set_current_owner_proximity_state(enum sid_owner_proximity_state state)
{
    return status_to_serr(sl_dult_set_near_owner_state(dult_handle, state));
}

sid_error_t sid_pal_dult_set_battery_level(enum sid_detect_unwanted_location_tracker_accessory_battery_level level)
{
    return status_to_serr(sl_dult_set_battery_level(dult_handle, level));
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
        .battery_type = info->battery,
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
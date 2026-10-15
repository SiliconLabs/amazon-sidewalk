/***************************************************************************//**
 * @file
 * @brief app_process.h
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef APP_PROCESS_H
#define APP_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include "app_init.h"
#include "sl_sidewalk_led_manager.h"
#include "app_bluetooth.h"
#include "sl_sidewalk_log_app.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#if defined(SL_SID_APP_MSG_PRESENT)
#define APP_DROP_REQUEST_IF_ONGOING_OTHERWISE_ACCEPT(app_action_ctx, temp_action_ctx) \
  if (app_action_ctx.hdl.processing) {                                                \
    SL_SID_LOG_APP_WARNING("request already ongoing, dropped");                       \
    return;                                                                           \
  } else {                                                                            \
    app_action_ctx.hdl.processing = true;                                             \
    app_action_ctx = *temp_action_ctx;                                                \
  }
#endif

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/*******************************************************************************
 * Main task
 ******************************************************************************/
void main_thread(void *context);

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
/*******************************************************************************
 * Application function to trigger connection request
 ******************************************************************************/
void app_trigger_connection_request(void);

/*******************************************************************************
 * Application function to connect, update and send counter
 ******************************************************************************/
void app_trigger_connect_and_send(void);
#endif

#if defined(SL_SID_APP_MSG_PRESENT)
/*******************************************************************************
 * Application function to connect, update and send counter
 ******************************************************************************/
void app_trigger_update_counter(sl_sid_app_msg_dmp_soc_light_update_counter_ctx_t *ctx);

/*******************************************************************************
 * Application function to trigger device reset
 ******************************************************************************/
void app_trigger_device_reset(sl_sid_app_msg_dev_mgmt_rst_dev_ctx_t *ctx);

/*******************************************************************************
 * Application function to trigger get time
 ******************************************************************************/
void app_trigger_time(sl_sid_app_msg_sid_time_ctx_t *ctx);

/*******************************************************************************
 * Application function to trigger get MTU
 ******************************************************************************/
void app_trigger_mtu(sl_sid_app_msg_sid_mtu_ctx_t *ctx);

/*******************************************************************************
 * Application function to trigger LED toggle
 ******************************************************************************/
void app_trigger_toggle_led(const sl_sid_app_msg_dmp_soc_light_toggle_led_ctx_t *ctx, enum toggle_led_source source);

/*******************************************************************************
 * Application function to start/stop ble
 ******************************************************************************/
void app_trigger_ble_start_stop(sl_sid_app_msg_dmp_soc_light_ble_start_stop_ctx_t *ctx);
#endif

/*******************************************************************************
 * Function to get the LED status
 *
 * @return LED status
 ******************************************************************************/
app_led_status_t app_get_led_status(void);

#ifdef __cplusplus
}
#endif

#endif // APP_PROCESS_H

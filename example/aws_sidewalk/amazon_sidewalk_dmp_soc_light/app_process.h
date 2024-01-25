/***************************************************************************//**
 * @file
 * @brief app_process.h
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#if defined(SL_SID_APP_MSG_PRESENT)
#define APP_DROP_REQUEST_IF_ONGOING_OTHERWISE_ACCEPT(app_action_ctx, temp_action_ctx) \
if (app_action_ctx.hdl.processing) {                                                  \
  app_log_warning("app: request already ongoing - drop");                             \
  return;                                                                             \
} else {                                                                              \
  app_action_ctx.hdl.processing = true;                                               \
  app_action_ctx = *temp_action_ctx;                                                  \
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
void app_trigger_toggle_led(sl_sid_app_msg_dev_mgmt_toggle_led_ctx_t *ctx);

/*******************************************************************************
 * Application function to start/stop ble
 ******************************************************************************/
void app_trigger_ble_start_stop(sl_sid_app_msg_dmp_soc_light_ble_start_stop_ctx_t *ctx);
#endif

#ifdef __cplusplus
}
#endif

#endif // APP_PROCESS_H

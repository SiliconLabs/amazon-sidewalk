/***************************************************************************//**
 * @file
 * @brief app_init.h
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
#ifndef APP_INIT_H
#define APP_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

#include "sl_led.h"

#if defined(SL_SID_APP_MSG_PRESENT)
#include "sl_sidewalk_app_msg_dev_mgmt.h"
#include "sl_sidewalk_app_msg_dmp_soc_light.h"
#include "sl_sidewalk_app_msg_sid.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// Sidewalk Events
enum event_type{
  EVENT_TYPE_SID_PROCESS_NEEDED = 0,
  EVENT_TYPE_COUNTER_UPDATE,
  EVENT_TYPE_DEVICE_RESET,
  EVENT_TYPE_LINK_SWITCH,
  EVENT_TYPE_TIME,
  EVENT_TYPE_MTU,
  EVENT_TYPE_DEV_REGISTERED,
  EVENT_TYPE_BLE_START_STOP,
  EVENT_TYPE_TOGGLE_LED,
  EVENT_TYPE_BTN_PRESS,
  EVENT_TYPE_BTN_PRESS_SEND_RESP,
  EVENT_TYPE_INVALID
};

// Sidewalk States defined in application context
enum app_state{
  STATE_INIT = 0,
  STATE_SIDEWALK_READY,
  STATE_SIDEWALK_NOT_READY,
  STATE_SIDEWALK_SECURE_CONNECTION
};

// Application context
typedef struct app_context{
  TaskHandle_t main_task;
  QueueHandle_t event_queue;
  struct sid_handle *sidewalk_handle;
  enum app_state state;
  uint8_t counter;
  uint32_t current_link_type;
  bool is_ble_running;
#if defined(SL_SID_APP_MSG_PRESENT)
  TimerHandle_t device_reset_timer;
  struct {
    sl_sid_app_msg_dev_mgmt_rst_dev_ctx_t rst_dev_ctx;
    sl_sid_app_msg_dev_mgmt_button_press_ctx_t button_press_ctx;
    sl_sid_app_msg_dev_mgmt_toggle_led_ctx_t toggle_led_ctx;
    sl_sid_app_msg_dmp_soc_light_ble_start_stop_ctx_t ble_start_stop_ctx;
    sl_sid_app_msg_dmp_soc_light_update_counter_ctx_t update_counter_ctx;
    sl_sid_app_msg_sid_mtu_ctx_t mtu_ctx;
    sl_sid_app_msg_sid_time_ctx_t time_ctx;
  } app_msg;
#endif
} app_context_t;

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * The function is used for application initialization.
 *
 * @param None
 * @returns None
 *****************************************************************************/
void app_init(void);

#ifdef __cplusplus
}
#endif

#endif // APP_INIT_H

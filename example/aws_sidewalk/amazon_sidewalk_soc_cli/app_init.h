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

#include "FreeRTOS.h"
#include "queue.h"
#include "sid_api.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// Sidewalk Events
enum event_type{
  EVENT_TYPE_SIDEWALK = 0,
  EVENT_TYPE_SID_RESET,
  EVENT_TYPE_SID_INIT,
  EVENT_TYPE_SID_START,
  EVENT_TYPE_SID_STOP,
  EVENT_TYPE_SID_DEINIT,
  EVENT_TYPE_SID_BLE_CONNECTION_REQUEST,
  EVENT_TYPE_SID_SEND,
  EVENT_TYPE_SID_GET_CSS_DEV_PROF_ID,
  EVENT_TYPE_SID_SET_CSS_DEV_PROF_ID,
  EVENT_TYPE_SID_GET_FSK_DEV_PROF_ID,
  EVENT_TYPE_SID_SET_FSK_DEV_PROF_ID,
  EVENT_TYPE_SID_SET_DEV_PROF_ID,
  EVENT_TYPE_SID_GET_DEV_PROF_RX_WIN_CNT,
  EVENT_TYPE_SID_SET_DEV_PROF_RX_WIN_CNT,
  EVENT_TYPE_SID_GET_DEV_PROF_RX_INTERV_MS,
  EVENT_TYPE_SID_SET_DEV_PROF_RX_INTERV_MS,
  EVENT_TYPE_SID_GET_DEV_PROF_WAKEUP_TYPE,
  EVENT_TYPE_SID_SET_DEV_PROF_WAKEUP_TYPE,
  EVENT_TYPE_SEND_COUNTER_UPDATE,
  EVENT_TYPE_FACTORY_RESET,
  EVENT_TYPE_GET_TIME,
  EVENT_TYPE_GET_STATUS,
  EVENT_TYPE_GET_MTU_BLE,
  EVENT_TYPE_GET_MTU_FSK,
  EVENT_TYPE_GET_MTU_CSS,
  EVENT_TYPE_SEND,
  EVENT_TYPE_INVALID
};

// Sidewalk States defined in application context
enum app_state{
  STATE_INIT = 0,
  STATE_SIDEWALK_READY,
  STATE_SIDEWALK_NOT_READY,
  STATE_SIDEWALK_SECURE_CONNECTION
};

// Link status type
typedef struct link_status{
  uint32_t link_mask;
  uint32_t supported_link_mode[SID_LINK_TYPE_MAX_IDX];
} link_status_t;

// Application context
typedef struct app_context{
  TaskHandle_t main_task;
  QueueHandle_t event_queue;
  struct sid_handle *sidewalk_handle;
  enum app_state state;
  link_status_t link_status;
  struct sid_event_callbacks sid_event_cb;
  struct sid_config sid_cfg;
  uint8_t counter;
#if defined(SL_BLE_SUPPORTED)
  bool ble_connection_status;
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

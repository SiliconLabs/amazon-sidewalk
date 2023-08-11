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

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// Sidewalk Events
enum event_type{
  EVENT_TYPE_SIDEWALK = 0,
#if defined(SL_BLE_SUPPORTED)
  EVENT_TYPE_CONNECTION_REQUEST,
  EVENT_TYPE_GET_CONNECTION_STATUS,
#endif
  EVENT_TYPE_SEND_COUNTER_UPDATE,
  EVENT_TYPE_FACTORY_RESET,
  EVENT_TYPE_LINK_SWITCH,
  EVENT_TYPE_GET_TIME,
  EVENT_TYPE_GET_MTU,
  EVENT_TYPE_REGISTERED,
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

// Application context
typedef struct app_context{
  TaskHandle_t main_task;
  QueueHandle_t event_queue;
  struct sid_handle *sidewalk_handle;
  enum app_state state;
  uint8_t counter;
  uint32_t current_link_type;
#if defined(SL_BLE_SUPPORTED)
  bool connection_request;
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

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

#include <stdint.h>

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/*******************************************************************************
 * Main task
 ******************************************************************************/
void main_thread(void * context);

/*******************************************************************************
 * Application function to update counter and send
 ******************************************************************************/
void app_trigger_send_counter_update(void);

/*******************************************************************************
 * Application function to connect, update and send counter
 ******************************************************************************/
void app_trigger_connect_and_send(void);

/*******************************************************************************
 * Application function to trigger Factory reset
 ******************************************************************************/
void app_trigger_factory_reset(void);

/*******************************************************************************
 * Application function to trigger get time
 ******************************************************************************/
void app_trigger_get_time(void);

/*******************************************************************************
 * Application function to trigger get MTU
 ******************************************************************************/
void app_trigger_get_mtu(void);

/*******************************************************************************
 * Application function to switch between BLE/FSK/CSS
 ******************************************************************************/
void app_trigger_link_switch(void);

/*******************************************************************************
 * Application function to trigger connection request
 ******************************************************************************/
void app_trigger_connection_request(void);

/*******************************************************************************
 * Application function to trigger get connection status
 ******************************************************************************/
void app_trigger_get_connection_status(void);

#ifdef __cplusplus
}
#endif

#endif // APP_PROCESS_H

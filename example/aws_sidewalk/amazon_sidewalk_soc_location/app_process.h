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
#include <sid_location.h>

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
 * Application function to trigger connection request
 ******************************************************************************/
void app_trigger_connection_request(void);

/*******************************************************************************
 * Application function to update and send counter
 ******************************************************************************/
void app_trigger_send_counter_update(void);

/*******************************************************************************
 * Application function to trigger Factory reset
 ******************************************************************************/
void app_trigger_factory_reset(void);

/*******************************************************************************
 * Application function to switch between BLE/FSK/CSS
 ******************************************************************************/
void app_trigger_link_switch(void);

/*******************************************************************************
 * Application function to trigger location init
 ******************************************************************************/
void app_trigger_location_init(void);

/*******************************************************************************
 * Application function to trigger location deinit
 ******************************************************************************/
void app_trigger_location_deinit(void);

/*******************************************************************************
 * Application function to trigger location send
 ******************************************************************************/
void app_trigger_location_send(enum sid_location_effort_mode effort);

#if defined(SL_LOCATION_FULL)
/*******************************************************************************
 * Application function to trigger location send buf
 ******************************************************************************/
void app_trigger_location_send_buf(enum sid_location_effort_mode effort);

/*******************************************************************************
 * Application function to trigger location scan
 ******************************************************************************/
void app_trigger_location_scan(enum sid_location_effort_mode effort);
#endif

/*******************************************************************************
 * Application function to trigger location alm start
 ******************************************************************************/
void app_trigger_location_alm_start(void);
#ifdef __cplusplus
}
#endif

#endif // APP_PROCESS_H

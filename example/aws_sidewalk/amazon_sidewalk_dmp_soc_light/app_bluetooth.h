/***************************************************************************//**
 * @file
 * @brief app_bluetooth.h
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

#ifndef APP_BLUETOOTH_H
#define APP_BLUETOOTH_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include "app_init.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * Application Init.
 *
 * @param None
 * @returns None
 *****************************************************************************/
void app_bluetooth_init(void);

/**************************************************************************//**
 * BLE Application Process Action.
 *
 * @param None
 * @returns None
 *****************************************************************************/
void app_bluetooth_update_led_status(uint8_t value, enum toggle_led_source source);

/**************************************************************************//**
 * BLE init and start advertisement
 *
 * @param None
 * @returns None
 *****************************************************************************/
void app_bluetooth_init_and_start_advertisement(void);

/**************************************************************************//**
 * BLE start advertisement
 *
 * @param None
 * @returns None
 *****************************************************************************/
void app_bluetooth_start_advertisement(void);

/**************************************************************************//**
 * BLE write response
 *
 * @param success true if the write request was successful, false otherwise
 * @returns None
 *****************************************************************************/
void app_bluetooth_toggle_led_write_response(bool success);

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
/**************************************************************************//**
 * BLE stop advertisement
 *
 * @param None
 * @returns None
 *****************************************************************************/
void app_bluetooth_stop_advertisement(void);

/**************************************************************************//**
 * Get BLE init state
 *
 * @param None
 * @returns None
 *****************************************************************************/
bool app_bluetooth_get_regular_ble_inited(void);
#endif

#ifdef __cplusplus
}
#endif

#endif // APP_BLUETOOTH_H

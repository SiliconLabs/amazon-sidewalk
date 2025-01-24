/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_led_manager.h
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

#ifndef SL_SIDEWALK_LED_MANAGER_H
#define SL_SIDEWALK_LED_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_led.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * Toggle led
 *
 * @param led_id Led ID
 *****************************************************************************/
void sl_sidewalk_led_manager_toggle_led(uint8_t led_id);

/**************************************************************************//**
 * Callback called when led state changes
 *
 * @param led_id Led ID
 * @param new_led_state New led state
 *****************************************************************************/
void sl_sidewalk_led_manager_led_state_changed(uint8_t led_id, sl_led_state_t new_led_state);

#ifdef __cplusplus
}
#endif

#endif //  SL_SIDEWALK_LED_MANAGER_H

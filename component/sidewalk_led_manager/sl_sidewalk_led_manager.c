/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_led_manager.c
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_sidewalk_led_manager.h"
#include "sl_common.h"
#include "app_log.h"
#include "sl_simple_led_instances.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sl_sidewalk_led_manager_toggle_led(uint8_t led_id)
{
  if (led_id > SL_SIMPLE_LED_COUNT - 1) {
    app_log_error("app: led %d does not exist", led_id);
    return;
  }

  const sl_led_t *l = SL_SIMPLE_LED_INSTANCE(led_id);

  sl_led_toggle(l);

  sl_led_state_t new_led_state = sl_led_get_state(l);

  switch (new_led_state) {
    case SL_LED_CURRENT_STATE_OFF:
      app_log_info("app: LED off");
      break;
    case SL_LED_CURRENT_STATE_ON:
      app_log_info("app: LED on");
      break;
    default:
      app_log_error("app: led manager toggle led error: %d", new_led_state);
      return;
  }

  sl_sidewalk_led_manager_led_state_changed(led_id, new_led_state);
}

////////////////////////////////////////////////////////////////////////////////
// Weak implementation of Callbacks                                           //
////////////////////////////////////////////////////////////////////////////////
SL_WEAK void sl_sidewalk_led_manager_led_state_changed(uint8_t led_id, sl_led_state_t new_led_state)
{
  (void)led_id;
  (void)new_led_state;
}

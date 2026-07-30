/***************************************************************************//**
 * @file app_cmd.c
 * @brief Contains the command callback function implementations.
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <string.h>

#include "app_process.h"
#include "sl_sidewalk_log_app.h"

#if (defined(SL_CATALOG_SIMPLE_LED_LED0_PRESENT) || defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT))
#include "sl_simple_led_instances.h"
#endif

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
#include "sl_sidewalk_display.h"
#endif

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void sl_sidewalk_web_utils_cmd_cb_led0_set(const char* cmd_arg)
{
#if defined(SL_CATALOG_SIMPLE_LED_LED0_PRESENT)
  if (strcmp(cmd_arg, "0") == 0) {
    sl_simple_led_turn_off(sl_led_led0.context);
  } else if (strcmp(cmd_arg, "1") == 0) {
    sl_simple_led_turn_on(sl_led_led0.context);
  } else {
    SL_SID_LOG_APP_WARNING("led0_set invalid command argument: %s", cmd_arg);
  }
#else
  (void)cmd_arg;
  SL_SID_LOG_APP_WARNING("led0_set ignored: LED0 not in this build");
#endif // defined(SL_CATALOG_SIMPLE_LED_LED0_PRESENT)
}

void sl_sidewalk_web_utils_cmd_cb_led1_set(const char* cmd_arg)
{
#if defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT)
  if (strcmp(cmd_arg, "0") == 0) {
    sl_simple_led_turn_off(sl_led_led1.context);
  } else if (strcmp(cmd_arg, "1") == 0) {
    sl_simple_led_turn_on(sl_led_led1.context);
  } else {
    SL_SID_LOG_APP_WARNING("led1_set invalid command argument: %s", cmd_arg);
  }
#else
  (void)cmd_arg;
  SL_SID_LOG_APP_WARNING("led1_set ignored: LED1 not in this build");
#endif // defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT)
}

void sl_sidewalk_web_utils_cmd_cb_link_switch_to_next(const char* cmd_arg)
{
  (void)cmd_arg;
  SL_SID_LOG_APP_INFO("link_switch_to_next command received");
  trigger_link_switch();
}

void sl_sidewalk_web_utils_cmd_cb_display_text(const char* cmd_arg)
{
  SL_SID_LOG_APP_INFO("display_text command received: %s", cmd_arg);
#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
  sl_sidewalk_display_message((char*) cmd_arg);
#else
  (void)cmd_arg;
#endif // defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
}
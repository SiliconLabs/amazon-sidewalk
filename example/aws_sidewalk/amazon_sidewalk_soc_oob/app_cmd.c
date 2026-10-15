/***************************************************************************//**
 * @file
 * @brief app_cmd.c
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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "sl_sidewalk_utils.h"
#include "sl_sidewalk_sender.h"
#include "sl_sidewalk_utils_config.h"
#include "sl_simple_led_instances.h"
#include "sl_sidewalk_log_app.h"

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
#include "sl_sidewalk_display.h"
#endif
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

bool sidewalk_cmd_cb_led0(void *payload, size_t payload_size)
{
  (void)payload_size;

#if defined(SL_CATALOG_LED0_PRESENT)
  if (payload) {
    int state = atoi(payload);
    state == 0 ? sl_simple_led_turn_off(sl_led_led0.context) : sl_simple_led_turn_on(sl_led_led0.context);
  }
#else
  (void)payload;
#endif

  return true;
}

bool sidewalk_cmd_cb_led1(void *payload, size_t payload_size)
{
  (void)payload_size;

#if defined(SL_CATALOG_LED1_PRESENT)
  if (payload) {
    int state = atoi(payload);
    state == 0 ? sl_simple_led_turn_off(sl_led_led1.context) : sl_simple_led_turn_on(sl_led_led1.context);
  }
#else
  (void)payload;
#endif

  return true;
}

bool sidewalk_cmd_cb_disconnect(void *payload, size_t payload_size)
{
  (void)payload;
  (void)payload_size;

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
  char *msg = "Website disconnected";
  sl_sidewalk_display_message(msg);
#endif

  return true;
}

bool sidewalk_cmd_cb_display(void *payload, size_t payload_size)
{
  (void)payload_size;

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
  sl_sidewalk_display_message(payload);
#else
  (void)payload;
#endif

  return true;
}

bool sidewalk_cmd_cb_ready(void *payload, size_t payload_size)
{
  (void)payload;
  (void)payload_size;
  char capabilities_str[SL_SIDEWALK_UTILS_CAPABILITIES_STR_MAX_LENGTH];

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
  char *msg = "Waiting for your message";
  sl_sidewalk_display_message(msg);
#endif

  sl_sidewalk_utils_get_capabilities_str(capabilities_str,
                                         SL_SIDEWALK_UTILS_CAPABILITIES_STR_MAX_LENGTH);
  return sl_sidewalk_sender_queue_message(capabilities_str,
                                          strlen(capabilities_str),
                                          SL_SIDEWALK_SENDER_TYPE_PRIORITY_HIGH);
}

bool sidewalk_cmd_cb_url(void *payload, size_t payload_size)
{
  (void)payload_size;

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
  sl_sidewalk_display_qr(payload);
#else
  (void)payload;
#endif

  return true;
}

bool sidewalk_cmd_cb_ack(void *payload, size_t payload_size)
{
  int sequence_number = 0;

  (void)payload_size;

  if (payload == NULL) {
    return false;
  }

  sequence_number = atoi(payload);

  sl_sidewalk_sender_sent_handler((uint16_t)sequence_number, SID_ERROR_NONE);

  SL_SID_LOG_APP_INFO("########################################");
  SL_SID_LOG_APP_INFO("              ACK RECEIVED              ");
  SL_SID_LOG_APP_INFO("########################################");
  SL_SID_LOG_APP_INFO("ack recieved for id: %u", sequence_number);
  SL_SID_LOG_APP_INFO("########################################");
  SL_SID_LOG_APP_INFO("########################################");

  return true;
}

bool sidewalk_cmd_cb_tink(void *payload, size_t payload_size)
{
  char msg[64];

  (void)payload_size;

  if (payload == NULL) {
    return false;
  }

  memset(msg, 0, sizeof(msg));
  strcat(msg, ":tonk ");
  strcat(msg, (char*)payload);
  sl_sidewalk_sender_queue_message(msg, strlen(msg), SL_SIDEWALK_SENDER_TYPE_PRIORITY_HIGH);

  return true;
}

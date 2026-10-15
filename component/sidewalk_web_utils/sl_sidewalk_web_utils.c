/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_web_utils.c
 *******************************************************************************
 * # License
 * <b>Copyright 2026 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 * Your use of this software is governed by the terms of
 * Silicon Labs Master Software License Agreement (MSLA)available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 * This software contains Third Party Software licensed by Silicon Labs from
 * Amazon.com Services LLC and its affiliates and is governed by the sections
 * of the MSLA applicable to Third Party Software and the additional terms set
 * forth in amazon_sidewalk_license.txt.
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
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sid_pal_mfg_store_ifc.h"
#include "sl_sidewalk_log_app.h"
#include "sl_sidewalk_utils.h"
#include "sl_sidewalk_web_utils.h"
#include "sl_sidewalk_web_utils_gen.h"
#include "sl_sidewalk_web_utils_types.h"
#include "sl_status.h"

// -----------------------------------------------------------------------------
//                           External Global Variables
// -----------------------------------------------------------------------------
extern const sl_sidewalk_web_utils_capability_str_t sl_sidewalk_web_utils_capability_str[];

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define SL_SIDEWALK_WEB_UTILS_PROTOCOL_VERSION "v1"

#define SL_SIDEWALK_WEB_UTILS_HELLO_VERSION_PREFIX "$"
#define SL_SIDEWALK_WEB_UTILS_HELLO_APP_PREFIX "+"
#define SL_SIDEWALK_WEB_UTILS_HELLO_SMSN_PREFIX "#"

#define SL_SIDEWALK_WEB_UTILS_READY_MSG "ready"
#define SL_SIDEWALK_WEB_UTILS_URL_PREFIX "url "

#define SL_SIDEWALK_WEB_UTILS_CAPABILITY_PREFIX "|"

#define SL_SIDEWALK_WEB_UTILS_SENSOR_PREFIX ":"
#define SL_SIDEWALK_WEB_UTILS_COMMAND_PREFIX "!"

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// Iterator for sl_sidewalk_web_utils_get_capability_msg. Retains its value
// across calls so the function can resume where it left off, and is reset by
// sl_sidewalk_web_utils_deinit_for_test (test-only) to start over.
static size_t capability_index = 0;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
sl_status_t sl_sidewalk_web_utils_get_initial_msg(char *buf, size_t buf_size)
{
  // Consistency checks (also cannot return an empty string without buffer)
  if (buf == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (buf_size == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  int n = snprintf(buf,
                   buf_size,
                   "%s%s%s%s%s",
                   SL_SIDEWALK_WEB_UTILS_HELLO_VERSION_PREFIX,
                   SL_SIDEWALK_WEB_UTILS_PROTOCOL_VERSION,
                   SL_SIDEWALK_WEB_UTILS_HELLO_APP_PREFIX,
                   SL_SIDEWALK_WEB_UTILS_APPLICATION_NAME,
                   SL_SIDEWALK_WEB_UTILS_HELLO_SMSN_PREFIX);

  if (n < 0) {
    SL_SID_LOG_APP_ERROR("Failed to create the first part of the initial message");
    return SL_STATUS_FAIL;
  }
  if (n >= (int) buf_size) {
    SL_SID_LOG_APP_ERROR("Failed to create the first part of the initial message");
    return SL_STATUS_WOULD_OVERFLOW;
  }

  // snprintf result here is the valid length of the first part
  size_t first_part_len = (size_t) n;
  // SL_SIDEWALK_UTILS_SMSN_STR_LENGTH includes the ending '\0'
  if (first_part_len + SL_SIDEWALK_UTILS_SMSN_STR_LENGTH > buf_size) {
    SL_SID_LOG_APP_ERROR("Not enough space in the buffer for the SMSN");
    return SL_STATUS_WOULD_OVERFLOW;
  }

  sl_sidewalk_utils_get_smsn_as_str(&buf[first_part_len],
                                    SL_SIDEWALK_UTILS_SMSN_STR_LENGTH);
  return SL_STATUS_OK;
}

sl_status_t sl_sidewalk_web_utils_get_capability_msg(char *buf,
                                                     size_t buf_size,
                                                     bool *is_completed)
{
  // Consistency checks
  if (buf == NULL || is_completed == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (buf_size == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Save the capability index value upon the current function entry
  // (capability_index is a file-scope static that retains its value between calls)
  size_t initial_capability_index = capability_index;

  *is_completed = true;
  buf[0] = '\0';

  for (; capability_index < SL_SIDEWALK_WEB_UTILS_CAPABILITY_NUM; capability_index++) {
    const char* cap_id = sl_sidewalk_web_utils_capability_str[capability_index].id;
    const char* cap_dsc = sl_sidewalk_web_utils_capability_str[capability_index].dsc;

    // Check if the next capability string fits in the buffer
    // 1 is the length of the start character '|'
    // The strict less-than accounts for the ending '\0'
    if (strlen(buf) + strlen(cap_id) + strlen(cap_dsc) + 1 < buf_size) {
      strcat(buf, SL_SIDEWALK_WEB_UTILS_CAPABILITY_PREFIX);
      strcat(buf, cap_id);
      strcat(buf, cap_dsc);
    } else {
      *is_completed = false;
      break;
    }
  }

  // No-progress guard: if at least one capability was pending and not even the
  // first one fit, the caller would loop forever calling this function. Surface
  // this as a runtime error so the caller can react (e.g. enlarge the buffer).
  if (!(*is_completed) && capability_index == initial_capability_index) {
    SL_SID_LOG_APP_ERROR("Buffer too small to fit even a single capability");
    return SL_STATUS_WOULD_OVERFLOW;
  }

  return SL_STATUS_OK;
}

#if SL_SIDEWALK_WEB_UTILS_SENSOR_NUM > 0
sl_status_t sl_sidewalk_web_utils_get_sensor_msg(char *buf,
                                                 size_t buf_size,
                                                 const sl_sidewalk_web_utils_capability_t capability,
                                                 const char *sensor_value)
{
  // Consistency checks
  if (buf == NULL || sensor_value == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (buf_size == 0 ||
      (size_t) capability >= SL_SIDEWALK_WEB_UTILS_SENSOR_NUM) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  const char* cap_id = sl_sidewalk_web_utils_capability_str[capability].id;

  int n = snprintf(buf,
                   buf_size,
                   "%s%s%s%s",
                   SL_SIDEWALK_WEB_UTILS_SENSOR_PREFIX,
                   cap_id,
                   SL_SIDEWALK_WEB_UTILS_ARG_SEPARATOR,
                   sensor_value);

  if (n < 0) {
    SL_SID_LOG_APP_ERROR("Failed to create the sensor message");
    return SL_STATUS_FAIL;
  }
  if (n >= (int) buf_size) {
    SL_SID_LOG_APP_ERROR("Failed to create the sensor message");
    return SL_STATUS_WOULD_OVERFLOW;
  }

  return SL_STATUS_OK;
}
#endif // SL_SIDEWALK_WEB_UTILS_SENSOR_NUM > 0

bool sl_sidewalk_web_utils_is_cloud_ready(const void *sid_payload,
                                          size_t sid_msg_size)
{
  // Note: even NULL payload is treated as "not the ready message"
  size_t ready_length = strlen(SL_SIDEWALK_WEB_UTILS_READY_MSG);
  return ((sid_payload != NULL) &&
          (sid_msg_size == ready_length) &&
          (memcmp(sid_payload, SL_SIDEWALK_WEB_UTILS_READY_MSG, ready_length) == 0));
}

sl_status_t sl_sidewalk_web_utils_try_parse_url(char *url_buf,
                                                size_t url_buf_size,
                                                bool *is_url,
                                                const void *sid_payload,
                                                size_t sid_msg_size)
{
  // Output side: NULL/zero-size are programmer errors
  if (url_buf == NULL || is_url == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (url_buf_size == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  size_t prefix_length = strlen(SL_SIDEWALK_WEB_UTILS_URL_PREFIX);

  // Input side: NULL payload, any size mismatch or wrong prefix simply means
  // "this is not a URL message"; report it via *is_url, not via status.
  if ((sid_payload == NULL) ||
      (sid_msg_size <= prefix_length) ||
      (memcmp(sid_payload, SL_SIDEWALK_WEB_UTILS_URL_PREFIX, prefix_length) != 0)) {
    *is_url = false;
    return SL_STATUS_OK;
  }

  // It is a URL message; the output buffer must be big enough (incl. '\0')
  size_t url_length = sid_msg_size - prefix_length;
  if (url_length + 1 > url_buf_size) {
    SL_SID_LOG_APP_ERROR("URL buffer too small to hold the parsed URL");
    // Set this up defensively even if an error code is returned
    *is_url = false;
    return SL_STATUS_WOULD_OVERFLOW;
  }

  memcpy(url_buf, &((const char*)sid_payload)[prefix_length], url_length);
  url_buf[url_length] = '\0';
  *is_url = true;
  return SL_STATUS_OK;
}

sl_status_t sl_sidewalk_web_utils_try_parse_cmd(const char **cmd,
                                                size_t *cmd_size,
                                                bool *is_cmd,
                                                const void *sid_payload,
                                                size_t sid_msg_size)
{
  // Output side: NULL pointers are programmer errors
  if (cmd == NULL || cmd_size == NULL || is_cmd == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  *is_cmd = false;

  // Input side: NULL or empty payload simply means "this is not a command message"
  if ((sid_payload == NULL) || (sid_msg_size == 0)) {
    return SL_STATUS_OK;
  }

  // First byte must match a known prefix; otherwise it is not a command message
  const char input_prefix[2] = { ((const char*)sid_payload)[0], '\0' };
  if ((strcmp(input_prefix, SL_SIDEWALK_WEB_UTILS_SENSOR_PREFIX) != 0) &&
      (strcmp(input_prefix, SL_SIDEWALK_WEB_UTILS_COMMAND_PREFIX) != 0)) {
    return SL_STATUS_OK;
  }

  *cmd = &((const char*)sid_payload)[1];
  *cmd_size = sid_msg_size - 1;
  *is_cmd = true;
  return SL_STATUS_OK;
}

#ifdef SL_SIDEWALK_UNIT_TEST
void sl_sidewalk_web_utils_deinit_for_test(void)
{
  capability_index = 0;
}
#endif

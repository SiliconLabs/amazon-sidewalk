/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_web_utils.h
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

#ifndef SL_SIDEWALK_WEB_UTILS_H
#define SL_SIDEWALK_WEB_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stddef.h>
#include <stdbool.h>

#include "sl_sidewalk_web_utils_gen.h"
#include "sl_status.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define SL_SIDEWALK_WEB_UTILS_ARG_SEPARATOR "="

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/**
 * Get the initial message containing protocol version, app name and SMSN.
 *
 * @param[out] buf Buffer for the initial message. Must be allocated by the caller. Contents are unspecified on error.
 * @param[in] buf_size Size of the buffer.
 * @return SL_STATUS_OK on success, error code otherwise.
 */
sl_status_t sl_sidewalk_web_utils_get_initial_msg(char *buf, size_t buf_size);

/**
 * Get the capability message for the supported capabilities.
 *
 * Transforms each capability in the format required by the web protocol and
 * writes as many of them as fit into the buffer. This function may be called again to
 * continue with the remaining capabilities.
 *
 * Note: this function is not prepared to produce the capability string more
 * than once during runtime (the internal iterator is not reset).
 *
 * @param[out] buf Buffer for the (possibly partial) null-terminated capability message.
 *             Must be allocated by the caller. Contents are unspecified on error.
 * @param[in] buf_size Size of the buffer.
 * @param[out] is_completed True if all remaining capabilities were
 *             written into the buffer in this call. False if there are still
 *             capabilities pending and the function must be called again.
 *             Only valid when the function returns SL_STATUS_OK.
 * @return SL_STATUS_OK on success, error code otherwise.
 */
sl_status_t sl_sidewalk_web_utils_get_capability_msg(char *buf,
                                                     size_t buf_size,
                                                     bool *is_completed);

#if SL_SIDEWALK_WEB_UTILS_SENSOR_NUM > 0
/**
 * Get the sensor message for the given capability and sensor value.
 *
 * @param[out] buf Buffer for the null-terminated sensor message.
 *             Must be allocated by the caller. Contents are unspecified on error.
 * @param[in] buf_size Size of the buffer.
 * @param[in] capability The sensor capability to get the sensor message for.
 * @param[in] sensor_value The sensor value to get the message for, null terminated string.
 * @return SL_STATUS_OK on success, error code otherwise.
 */
sl_status_t sl_sidewalk_web_utils_get_sensor_msg(char *buf,
                                                 size_t buf_size,
                                                 const sl_sidewalk_web_utils_capability_t capability,
                                                 const char *sensor_value);
#endif // SL_SIDEWALK_WEB_UTILS_SENSOR_NUM > 0

/**
 * Check if the a received message is a cloud handshake response, i.e., the
 * cloud is ready to receive capabilities.
 *
 * @param[in] sid_payload The payload of the received sid message, not null
 *            terminated. May be NULL (returns false).
 * @param[in] sid_msg_size The size of the received message. May be 0 (returns false).
 * @return True if the cloud is ready to receive capabilities, false otherwise.
 */
bool sl_sidewalk_web_utils_is_cloud_ready(const void *sid_payload,
                                          size_t sid_msg_size);

/**
 * Check if the a received message is a URL and parse it into the caller's buffer.
 *
 * Note: any form of invalid input is treated as "this is not a URL message" without reporting error.
 * However invalid output arguments are reported as errors.
 *
 * @param[out] url_buf Buffer to receive the null-terminated parsed URL.
 *             Allocated by the caller. Contents are unspecified on error,
 *             and if is_url is false.
 * @param[in] url_buf_size Size of the buffer.
 * @param[out] is_url True if the URL is valid and parsed successfully.
 *             Unspecified on error.
 * @param[in] sid_payload The payload of the received sid message, not null
 *            terminated. May be NULL.
 * @param[in] sid_msg_size The size of the received message.
 * @return SL_STATUS_OK on success, error code otherwise.
 */
sl_status_t sl_sidewalk_web_utils_try_parse_url(char *url_buf,
                                                size_t url_buf_size,
                                                bool *is_url,
                                                const void *sid_payload,
                                                size_t sid_msg_size);

/**
 * Check if the a received message is a command, and "parses" it from the web protocol format.
 *
 * Note: the result is simply a pointer into the input payload. No copy is made.
 * The caller must ensure that the sid payload remains valid as long as the pointer is used.
 *
 * Note: any form of invalid input is treated as "this is not a command message"
 * without reporting an error. However invalid output arguments are reported
 * as errors.
 *
 * @param[out] cmd Pointer to the parsed command. Unspecified on error and when is_cmd is false.
 * @param[out] cmd_size Size of the command.
 * @param[out] is_cmd True if a command was identified. Unspecified on error.
 * @param[in] sid_payload The payload of the received sid message, not null
 *            terminated. May be NULL.
 * @param[in] sid_msg_size The size of the received sid message.
 * @return SL_STATUS_OK on success, error code otherwise.
 */
sl_status_t sl_sidewalk_web_utils_try_parse_cmd(const char **cmd,
                                                size_t *cmd_size,
                                                bool *is_cmd,
                                                const void *sid_payload,
                                                size_t sid_msg_size);

#ifdef SL_SIDEWALK_UNIT_TEST
/**
 * Test-only hook to reset the component's internal iterator state.
 *
 * Resets the capability iterator so each test can start the
 * capability-message iteration from the beginning. Not part of the public API;
 * only compiled in unit-test builds.
 */
void sl_sidewalk_web_utils_deinit_for_test(void);
#endif

#ifdef __cplusplus
}
#endif

#endif // SL_SIDEWALK_WEB_UTILS_H

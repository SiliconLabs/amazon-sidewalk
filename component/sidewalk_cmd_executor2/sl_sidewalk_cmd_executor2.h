/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_cmd_executor2.h
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

#ifndef SL_SIDEWALK_CMD_EXECUTOR2_H
#define SL_SIDEWALK_CMD_EXECUTOR2_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdbool.h>
#include <stddef.h>

#include "sl_sidewalk_cmd_executor2_types.h"
#include "sl_status.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/**
 * Initializes the command executor.
 *
 * @param[in] config The configuration for the command executor. Shallow copied into the component.
 * @return SL_STATUS_OK if the command executor is initialized successfully, otherwise an error code.
 */
sl_status_t sl_sidewalk_cmd_executor2_init(const sl_sidewalk_cmd_executor2_config_t *config);

/**
 * Receives a command string and queues it for execution.
 *
 * Copies the string to an internal command-sized buffer, adds null termination, then queues it.
 * Performs a length check (i.e., no silent truncation).
 * Does not perform any format check.
 *
 * @param[in] cmd The input command string, needs NOT to be null terminated.
 * @param[in] cmd_size The size of the input command, without the null termination.
 * @return SL_STATUS_OK if the input is successfully queued as a command, otherwise an error code.
 */
sl_status_t sl_sidewalk_cmd_executor2_receive(const char *cmd, size_t cmd_size);

/**
 * Executes the next queued command by calling its callback with its arguments.
 *
 * @return SL_STATUS_OK if the command is executed successfully, otherwise an error code.
 */
sl_status_t sl_sidewalk_cmd_executor2_execute(void);

#ifdef SL_SIDEWALK_UNIT_TEST
/**
 * Test-only hook to reset the component to its uninitialized state.
 *
 * Deletes the internal command queue (if any), clears the cached configuration,
 * and clears the initialized flag, so each test can start from a clean slate.
 * Not part of the public API; only compiled in unit-test builds.
 */
void sl_sidewalk_cmd_executor2_deinit_for_test(void);
#endif

#ifdef __cplusplus
}
#endif

#endif // SL_SIDEWALK_CMD_EXECUTOR2_H

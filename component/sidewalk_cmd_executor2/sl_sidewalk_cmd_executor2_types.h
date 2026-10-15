/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_cmd_executor2_types.h
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

#ifndef SL_SIDEWALK_CMD_EXECUTOR2_TYPES_H
#define SL_SIDEWALK_CMD_EXECUTOR2_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stddef.h>

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/**
 * Callback function for the command executor.
 *
 * @param[in] args The arguments of the command, null terminated string.
 */
typedef void (*sl_sidewalk_cmd_executor2_cb_t)(const char* args);

typedef struct {
  const char *cmd;
  sl_sidewalk_cmd_executor2_cb_t cb;
} sl_sidewalk_cmd_executor2_cmd_t;

typedef struct {
  /* Complete list of commands */
  const sl_sidewalk_cmd_executor2_cmd_t *commands;
  /* Number of commands in the list */
  size_t num_of_commands;
  /* Separator character between the command name and the arguments */
  char arg_separator;
  /* Maximum length of a command with arguments, including null termination */
  size_t command_max_length;
  /* Size of the command queue */
  size_t command_queue_size;
} sl_sidewalk_cmd_executor2_config_t;

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // SL_SIDEWALK_CMD_EXECUTOR2_TYPES_H

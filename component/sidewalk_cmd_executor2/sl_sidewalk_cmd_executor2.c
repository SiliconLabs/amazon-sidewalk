/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_cmd_executor2.c
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
 *  claim that you wrote the original software. If you use this software
 *  in a product, an acknowledgment in the product documentation would be
 *  appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *  misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdbool.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "sl_sidewalk_cmd_executor2.h"
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

static bool initialized = false;
static sl_sidewalk_cmd_executor2_config_t conf;
static QueueHandle_t command_queue = NULL;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
sl_status_t sl_sidewalk_cmd_executor2_init(const sl_sidewalk_cmd_executor2_config_t *config)
{
  // Consistency checks
  if (initialized) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }
  if (config == NULL ||
      config->commands == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (config->num_of_commands == 0 ||
      config->arg_separator == '\0' ||
      config->command_max_length < 2 ||
      config->command_queue_size == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Command table consistency checks
  for (size_t i = 0; i < config->num_of_commands; i++) {
    const sl_sidewalk_cmd_executor2_cmd_t *entry = &config->commands[i];
    if (entry->cmd == NULL ||
        entry->cb == NULL) {
      return SL_STATUS_NULL_POINTER;
    }
    size_t cmd_name_len = strlen(entry->cmd);
    if (cmd_name_len == 0 ||
        cmd_name_len >= config->command_max_length ||
        strchr(entry->cmd, config->arg_separator) != NULL) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }

  // Create command queue
  command_queue = xQueueCreate(config->command_queue_size, config->command_max_length);
  if (command_queue == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  // Actual component initialization
  memcpy(&conf, config, sizeof(sl_sidewalk_cmd_executor2_config_t));
  initialized = true;
  return SL_STATUS_OK;
}

sl_status_t sl_sidewalk_cmd_executor2_receive(const char *cmd, size_t cmd_size)
{
  // Consistency checks
  if (!initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (cmd == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (cmd_size == 0 ||
      cmd_size >= conf.command_max_length) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Copy command, enforce null termination, queue it
  char command_buffer[conf.command_max_length];
  memcpy(command_buffer, cmd, cmd_size);
  memset(&command_buffer[cmd_size], 0, conf.command_max_length - cmd_size);
  if (xQueueSend(command_queue, (void *)command_buffer, (TickType_t)0) != pdTRUE) {
    return SL_STATUS_FULL;
  }
  return SL_STATUS_OK;
}

sl_status_t sl_sidewalk_cmd_executor2_execute(void)
{
  // Consistency checks
  if (!initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  // Receive the next command from the queue
  char rcvd[conf.command_max_length];
  if (xQueueReceive(command_queue, &rcvd, 0) != pdTRUE) {
    return SL_STATUS_EMPTY;
  }

  // Extract the command name and the arguments from the whole queued command string
  // Both are null terminated strings, separator after command name is dropped
  char *rcvd_cmd = rcvd;
  char *rcvd_args = "";
  char *separator = strchr(rcvd, conf.arg_separator);
  if (separator != NULL) {
    *separator = '\0';
    rcvd_args = separator + 1;
  }

  // Find the command in the command table and execute it
  for (size_t i = 0; i < conf.num_of_commands; i++) {
    if (strcmp(rcvd_cmd, conf.commands[i].cmd) == 0) {
      conf.commands[i].cb(rcvd_args);
      return SL_STATUS_OK;
    }
  }

  // Command not found, return error
  return SL_STATUS_NOT_FOUND;
}

#ifdef SL_SIDEWALK_UNIT_TEST
void sl_sidewalk_cmd_executor2_deinit_for_test(void)
{
  if (command_queue != NULL) {
    vQueueDelete(command_queue);
    command_queue = NULL;
  }
  memset(&conf, 0, sizeof(conf));
  initialized = false;
}
#endif

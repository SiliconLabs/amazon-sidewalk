/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_cmd_executor.c
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "sl_common.h"
#include "sl_command_table.h"
#include "sl_sidewalk_cmd_executor.h"
#include "sl_sidewalk_utils.h"
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
extern const sl_sidewalk_command_t SL_SIDEWALK_COMMANDS[];
// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static QueueHandle_t recieve_queue = NULL;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

SL_WEAK void sl_sidewalk_cmd_executor_common_cb(sl_sidewalk_command_id_t command)
{
  (void)(command);
}

void sl_sidewalk_cmd_executor_init(void)
{
  recieve_queue = xQueueCreate(
    SL_SIDEWALK_UTILS_MAX_STORED_COMMANDS_NUM,
    SL_SIDEWALK_UTILS_MAX_COMMAND_LENGTH_CHAR);
}

void sl_sidewalk_cmd_executor_execute(void)
{
  char command_buffer[SL_SIDEWALK_UTILS_MAX_COMMAND_LENGTH_CHAR];
  char *command_in_buffer;
  char *payload_start;
  size_t payload_size;

  memset(command_buffer, 0, sizeof(command_buffer));

  if (xQueueReceive(recieve_queue, &command_buffer, 0) == pdTRUE) {
    for (sl_sidewalk_command_id_t i = (sl_sidewalk_command_id_t) 0; i < SIDEWALK_COMMAND_ID_END; ++i) {
      command_in_buffer = strstr(command_buffer, SL_SIDEWALK_COMMANDS[i].command);
      if (command_in_buffer != NULL) {
        if (SL_SIDEWALK_COMMANDS[i].callback != NULL) {
          payload_start = command_in_buffer + strlen(SL_SIDEWALK_COMMANDS[i].command);
          payload_size = strlen(payload_start);
          SL_SIDEWALK_COMMANDS[i].callback(payload_start, payload_size);
        }
        sl_sidewalk_cmd_executor_common_cb(i);
        break;
      }
    }
  }
}

bool sl_sidewalk_cmd_executor_recieve(char *message, size_t message_length)
{
  char command_buffer[SL_SIDEWALK_UTILS_MAX_COMMAND_LENGTH_CHAR];

  if (!message || message_length == 0) {
    return false;
  }

  memset(command_buffer, 0, sizeof(command_buffer));

  // Copy as many message bytes to the buffer as possible
  uint8_t max_copy_length = message_length <= (SL_SIDEWALK_UTILS_MAX_COMMAND_LENGTH_CHAR - 1)
                            ? (uint8_t)message_length : (SL_SIDEWALK_UTILS_MAX_COMMAND_LENGTH_CHAR - 1);
  memcpy(command_buffer, message, max_copy_length);

  bool result = (pdTRUE == xQueueSend(recieve_queue, (void *)command_buffer, (TickType_t)0)) ? true : false;
  return result;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

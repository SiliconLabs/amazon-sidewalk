/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_sender.c
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

#include <printf.h>
#include <stdint.h>
#include <string.h>

#include "app_log.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "sl_sidewalk_sender.h"
#include "sl_sidewalk_utils.h"
#include "sl_sidewalk_utils_config.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define SIDEWALK_SENDER_MESSAGE_MAX_LENGTH_BYTES (255)

typedef struct {
  char msg[SIDEWALK_SENDER_MESSAGE_MAX_LENGTH_BYTES];
  size_t len;
  uint16_t id;
  sid_error_t error;
  TickType_t last_try;
} sidewalk_sender_msg_t;

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

static uint16_t put_message(struct sid_handle *sidewalk_handle, char* payload, uint16_t payload_length);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static QueueHandle_t sender_queues[SL_SIDEWALK_SENDER_TYPE_END];

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sl_sidewalk_sender_init(void)
{
  for (uint8_t queue_ix = 0; queue_ix < SL_SIDEWALK_SENDER_TYPE_END; queue_ix++) {
    sender_queues[queue_ix] = xQueueCreate(SL_SIDEWALK_UTILS_MAX_PENDING_MESSAGES_NUM, sizeof(sidewalk_sender_msg_t));
  }
}

void sl_sidewalk_sender_send(struct sid_handle *sidewalk_handle)
{
  static sidewalk_sender_msg_t message_to_send;
  static TickType_t current_time;

  // for(uint8_t queue_ix = SL_SIDEWALK_SENDER_TYPE_PRIORITY_HIGH; queue_ix >= 0 ; queue_ix--)
  for (int8_t queue_ix = SL_SIDEWALK_SENDER_TYPE_PRIORITY_HIGH; queue_ix >= 0; queue_ix--) {
    if (xQueueReceive(sender_queues[queue_ix], &message_to_send, 0) == pdTRUE) {
      current_time = xTaskGetTickCount();

      if (message_to_send.last_try == 0) {
        app_log_info("###############################");
        app_log_info("            FIRST TRY          ");
        app_log_info("            PRIO: %d           ", queue_ix);
        app_log_info("###############################");
        message_to_send.id = put_message(sidewalk_handle, message_to_send.msg, message_to_send.len);
        message_to_send.last_try = xTaskGetTickCount();
      } else {
        if (current_time - message_to_send.last_try >= pdMS_TO_TICKS(SL_SIDEWALK_UTILS_MSG_TIMEOUT_MS)) {
          app_log_info("###############################");
          app_log_info("              RETRY            ");
          app_log_info("            PRIO: %d           ", queue_ix);
          app_log_info("###############################");
          message_to_send.id = put_message(sidewalk_handle, message_to_send.msg, message_to_send.len);
          message_to_send.last_try = xTaskGetTickCount();
        }
      }

      xQueueSendToFront(sender_queues[queue_ix], &message_to_send, (TickType_t)0);
      break;
    }
  }
}

void sl_sidewalk_sender_sent_handler(uint16_t id, sid_error_t error)
{
  sidewalk_sender_msg_t message;

  for (uint8_t queue_ix = 0; queue_ix < SL_SIDEWALK_SENDER_TYPE_END; queue_ix++) {
    if (pdTRUE == xQueuePeek(sender_queues[queue_ix], &message, (TickType_t)0)) {
      if (message.id == id) {
        if (error == SID_ERROR_NONE) {
          // Remove from the queue, sent successfully
          xQueueReceive(sender_queues[queue_ix], &message, (TickType_t)100);
        } else {
          if (pdTRUE == xQueueReceive(sender_queues[queue_ix], &message, (TickType_t)100)) {
            message.error = error;
            message.last_try = 0;
            xQueueSendToFront(sender_queues[queue_ix], &message, (TickType_t)100);
          }
        }
        break;
      }
    }
  }
}

bool sl_sidewalk_sender_queue_message(char *message, size_t message_length, sl_sidewalk_sender_priority_type_t priority)
{
  sidewalk_sender_msg_t message_to_send;
  memset((void*) &message_to_send, 0, sizeof(message_to_send));

  strcpy(message_to_send.msg, message);
  message_to_send.len = message_length;
  message_to_send.last_try = 0;

  if (pdTRUE == xQueueSend(sender_queues[priority], (void*) &message_to_send, (TickType_t)0)) {
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static uint16_t put_message(struct sid_handle *sidewalk_handle, char *payload, uint16_t payload_length)
{
  app_log_info("###############################");
  app_log_info("        SENDING MESSAGE        ");
  app_log_info("###############################");
  app_log_info("sending %d bytes", payload_length);
  app_log_info("sending %s", payload);
  app_log_info("###############################");
  app_log_info("###############################");

  struct sid_msg msg = {
    .data = payload,
    .size = strlen(payload)
  };

  // The descriptor is cleared and then only partially initialized which is
  // intentional
  struct sid_msg_desc desc;
  memset(&desc, 0, sizeof(desc));
  desc.type = SID_MSG_TYPE_NOTIFY;
  desc.link_type = SID_LINK_TYPE_ANY;
  desc.link_mode = SID_LINK_MODE_CLOUD;

  sid_error_t ret = sid_put_msg(sidewalk_handle, &msg, &desc);

  if (ret != SID_ERROR_NONE) {
    app_log_error("failed queuing data: %d", (int)ret);
  } else {
    app_log_info("queued data message id: %u", desc.id);
  }

  return desc.id;
}

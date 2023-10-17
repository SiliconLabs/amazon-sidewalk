/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_display.c
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
#include <stdlib.h>
#include <string.h>

#include "dmd.h"
#include "em_gpio.h"
#include "FreeRTOS.h"
#include "glib.h"
#include "queue.h"
#include "sl_sidewalk_display.h"
#include "sl_sidewalk_display_config.h"
#include "sli_sidewalk_qr_code.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

typedef enum {
  DISPLAY_MSG_TYPE_NORMAL = 0,
  DISPLAY_MSG_TYPE_QR,
  DISPLAY_MSG_TYPE_STATUS,
} display_msg_type_t;

typedef struct {
  QueueHandle_t queue;
  char buffer[SL_SIDEWALK_DISPLAY_CHAR_H_PX][SL_SIDEWALK_DISPLAY_CHAR_W_PX];
  GLIB_Context_t glibContext;
} display_t;

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

static void draw_qr(display_t *display_handler, sli_sidewalk_qr_code_qr_t *qr_buffer);
static void handle_status_msg(display_t *display_handler, sl_sidewalk_display_msg_t *message);
static void handle_qr_msg(display_t *display_handler, sl_sidewalk_display_msg_t *message);
static void handle_normal_msg(display_t *display_handler, sl_sidewalk_display_msg_t *message);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static display_t display_handler;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sl_sidewalk_display_init(void)
{
  display_handler.queue = xQueueCreate(
    SL_SIDEWALK_DISPLAY_STRING_PENDING_NUM_MAX,
    sizeof(sl_sidewalk_display_msg_t));
  memset(display_handler.buffer, 0, sizeof(display_handler.buffer));

  GPIO_PinModeSet(SL_BOARD_ENABLE_DISPLAY_PORT, SL_BOARD_ENABLE_DISPLAY_PIN, gpioModePushPull, 0);
  GPIO_PinOutSet(SL_BOARD_ENABLE_DISPLAY_PORT, SL_BOARD_ENABLE_DISPLAY_PIN);

  DMD_init(0);

  GLIB_contextInit(&display_handler.glibContext);
  GLIB_setFont(&display_handler.glibContext, (GLIB_Font_t *)&GLIB_FontNormal8x8);
  display_handler.glibContext.backgroundColor = White;
  display_handler.glibContext.foregroundColor = Black;
}

void sl_sidewalk_display_update(void)
{
  sl_sidewalk_display_msg_t message;

  if (pdTRUE == xQueueReceive(display_handler.queue, &message, (TickType_t)0)) {
    GLIB_clear(&display_handler.glibContext);

    // Call the display handler which suits the message type
    switch (message.type) {
      case DISPLAY_MSG_TYPE_NORMAL:
        handle_normal_msg(&display_handler, &message);
        break;

      case DISPLAY_MSG_TYPE_QR:
        handle_qr_msg(&display_handler, &message);
        break;

      case DISPLAY_MSG_TYPE_STATUS:
        handle_status_msg(&display_handler, &message);
        break;

      default:
        break;
    }

    // Write textual (non-qr) message characters on display lines
    if (message.type != DISPLAY_MSG_TYPE_QR) {
      for (uint8_t line = 0; line < SL_SIDEWALK_DISPLAY_CHAR_H_PX; line++) {
        GLIB_drawStringOnLine(&display_handler.glibContext,
                              &display_handler.buffer[line][0],
                              line,
                              GLIB_ALIGN_LEFT,
                              0,
                              0,
                              false);
      }

      DMD_updateDisplay();
    }
  }
}

void sl_sidewalk_display_stats(sl_sidewalk_display_statistics_t *statistics)
{
  sl_sidewalk_display_msg_t message;
  memset(&message, 0, sizeof(sl_sidewalk_display_msg_t));

  message.type = DISPLAY_MSG_TYPE_STATUS;
  memcpy(&message.stats, statistics, sizeof(sl_sidewalk_display_statistics_t));

  xQueueSend(display_handler.queue, (void*) &message, (TickType_t)0);
}

void sl_sidewalk_display_message(char *payload)
{
  sl_sidewalk_display_msg_t message;
  memset(&message, 0, sizeof(sl_sidewalk_display_msg_t));

  message.type = DISPLAY_MSG_TYPE_NORMAL;
  memcpy(&message.text, payload, strlen(payload));

  xQueueSend(display_handler.queue, (void*) &message, (TickType_t)0);
}

void sl_sidewalk_display_qr(char *payload)
{
  sl_sidewalk_display_msg_t message;
  memset(&message, 0, sizeof(sl_sidewalk_display_msg_t));

  message.type = DISPLAY_MSG_TYPE_QR;
  memcpy(&message.text, payload, strlen(payload));

  xQueueSend(display_handler.queue, (void*) &message, (TickType_t)0);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static void draw_qr(display_t *display_handler, sli_sidewalk_qr_code_qr_t *qr_buffer)
{
  uint8_t scale = SL_SIDEWALK_DISPLAY_W_PX / qr_buffer->width;
  GLIB_Rectangle_t rect = {
    .xMin = 0,
    .xMax = 0,
    .yMin = 0,
    .yMax = 0,
  };

  GLIB_clear(&display_handler->glibContext);

  for (uint8_t x = 0; x < qr_buffer->width; x++) {
    for (uint8_t y = 0; y < qr_buffer->width; y++) {
      rect.xMin =  x * scale + SL_SIDEWALK_DISPLAY_BORDER_PX;
      rect.xMax = (x * scale) + scale + SL_SIDEWALK_DISPLAY_BORDER_PX;

      rect.yMin =  y * scale + SL_SIDEWALK_DISPLAY_BORDER_PX;
      rect.yMax = (y * scale) + scale + SL_SIDEWALK_DISPLAY_BORDER_PX;

      if (sli_sidewalk_qr_code_is_module_pixel_dark(qr_buffer, x, y)) {
        GLIB_drawRectFilled(&display_handler->glibContext, &rect);
      }
    }
  }

  DMD_updateDisplay();
}

static void handle_status_msg(display_t *display_handler, sl_sidewalk_display_msg_t *message)
{
  char status_str_buffer[24];
  memset(status_str_buffer, 0, sizeof(status_str_buffer));

  sprintf(status_str_buffer, "registration: %d", message->stats.is_registered);
  memset(&display_handler->buffer[SL_SIDEWALK_DISPLAY_STATUS_START_LINE][0], 0, SL_SIDEWALK_DISPLAY_CHAR_W_PX);
  memcpy(&display_handler->buffer[SL_SIDEWALK_DISPLAY_STATUS_START_LINE][0], status_str_buffer, strlen(status_str_buffer));

  sprintf(status_str_buffer, "time sync: %d", message->stats.is_time_synced);
  memset(&display_handler->buffer[SL_SIDEWALK_DISPLAY_STATUS_START_LINE + 1][0], 0, SL_SIDEWALK_DISPLAY_CHAR_W_PX);
  memcpy(&display_handler->buffer[SL_SIDEWALK_DISPLAY_STATUS_START_LINE + 1][0], status_str_buffer, strlen(status_str_buffer));

  sprintf(status_str_buffer, "link type: %d", message->stats.link);
  memset(&display_handler->buffer[SL_SIDEWALK_DISPLAY_STATUS_START_LINE + 2][0], 0, SL_SIDEWALK_DISPLAY_CHAR_W_PX);
  memcpy(&display_handler->buffer[SL_SIDEWALK_DISPLAY_STATUS_START_LINE + 2][0], status_str_buffer, strlen(status_str_buffer));

  sprintf(status_str_buffer, "last tx msg id: %d", message->stats.last_successful_tx_seq_num);
  memset(&display_handler->buffer[SL_SIDEWALK_DISPLAY_STATUS_START_LINE + 3][0], 0, SL_SIDEWALK_DISPLAY_CHAR_W_PX);
  memcpy(&display_handler->buffer[SL_SIDEWALK_DISPLAY_STATUS_START_LINE + 3][0], status_str_buffer, strlen(status_str_buffer));
}

static void handle_normal_msg(display_t *display_handler, sl_sidewalk_display_msg_t *message)
{
  uint16_t msg_ix = 0;
  int16_t msg_len = strlen(message->text);
  int16_t current_line = SL_SIDEWALK_DISPLAY_MESSAGE_START_LINE;

  // Clear the display handler buffer first
  for (uint8_t start_line = SL_SIDEWALK_DISPLAY_MESSAGE_START_LINE; start_line < SL_SIDEWALK_DISPLAY_STATUS_START_LINE; start_line++) {
    memset(&display_handler->buffer[start_line][0], 0, SL_SIDEWALK_DISPLAY_CHAR_W_PX);
  }

  // Put the meassage into the display buffer line by line, until the whole message is buffered or all lines are full
  while (msg_len > 0) {
    uint16_t chunk_to_copy = (msg_len >= SL_SIDEWALK_DISPLAY_CHAR_W_PX) ? SL_SIDEWALK_DISPLAY_CHAR_W_PX : msg_len;

    memcpy(&display_handler->buffer[current_line][0], &message->text[msg_ix], chunk_to_copy);
    msg_ix += chunk_to_copy;
    msg_len -= chunk_to_copy;

    current_line++;
    if (current_line == SL_SIDEWALK_DISPLAY_STATUS_START_LINE) {
      break;
    }
  }
}

static void handle_qr_msg(display_t *display_handler, sl_sidewalk_display_msg_t *message)
{
  // Too big for stack, we place it to heap.
  static sli_sidewalk_qr_code_qr_t qr;
  memset(&qr, 0, sizeof(sli_sidewalk_qr_code_qr_t));

  sli_sidewalk_qr_code_create_qr_from_str(&qr, message->text);
  draw_qr(display_handler, &qr);
}

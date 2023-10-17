/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_sender.h
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
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SL_SIDEWALK_MESSAGE_SENDER_H
#define SL_SIDEWALK_MESSAGE_SENDER_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdbool.h>

#include "sid_api.h"
#include "sid_error.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
typedef enum {
  SL_SIDEWALK_SENDER_TYPE_PRIORITY_LOW = 0,
  SL_SIDEWALK_SENDER_TYPE_PRIORITY_MEDIUM,
  SL_SIDEWALK_SENDER_TYPE_PRIORITY_HIGH,
  SL_SIDEWALK_SENDER_TYPE_END
} sl_sidewalk_sender_priority_type_t;
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * Initializer of the sidewalk_sender component
 *****************************************************************************/
void sl_sidewalk_sender_init(void);

void sl_sidewalk_sender_send(struct sid_handle *sidewalk_handle);
void sl_sidewalk_sender_sent_handler(uint16_t id, sid_error_t error);
bool sl_sidewalk_sender_queue_message(char *message, size_t message_length, sl_sidewalk_sender_priority_type_t priority);

#endif // SL_SIDEWALK_MESSAGE_SENDER_H

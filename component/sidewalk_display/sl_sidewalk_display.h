/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_display.h
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

#ifndef SL_SIDEWALK_DISPLAY_H
#define SL_SIDEWALK_DISPLAY_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdbool.h>

#include "sl_board_control_config.h"
#include "sl_sidewalk_display_types.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define SL_SIDEWALK_DISPLAY_STRING_PENDING_NUM_MAX 4

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * This function performs intitalization of the display.
 *
 * This includes GPIO, DMD setup and other GLIB initialization.
 *****************************************************************************/
void sl_sidewalk_display_init(void);

/**************************************************************************//**
 * Getting the next received and queued message and displaying it.
 *
 * This function is a dispatcher which calls further display handlers based on
 * the message type and also GLIB and DMD functions.
 *****************************************************************************/
void sl_sidewalk_display_update(void);

/**************************************************************************//**
 * This function puts the incoming statistic data to a message with a matching
 * type and queues the message for future display.
 *
 * @param statistics Statistic data (registration, time sync, etc.) to be
 *                   displayed
 *****************************************************************************/
void sl_sidewalk_display_stats(sl_sidewalk_display_statistics_t *statistics);

/**************************************************************************//**
 * This function puts the incoming string payload to a normal message and queues
 * the message for future display.
 *
 * @param payload The string payload to be displayed
 *****************************************************************************/
void sl_sidewalk_display_message(char *payload);

/**************************************************************************//**
 * This function puts the incoming QR code (received as a simple string
 * argument) to a qr-type message and queues the message for future display.
 *
 * @param payload The QR-code to be displayed
 *****************************************************************************/
void sl_sidewalk_display_qr(char *payload);

#endif // SL_SIDEWALK_DISPLAY_H

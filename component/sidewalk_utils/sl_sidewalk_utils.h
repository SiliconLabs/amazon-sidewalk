/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_utils.h
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

#ifndef SL_SIDEWALK_UTILS_H
#define SL_SIDEWALK_UTILS_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

#include "sl_sidewalk_utils_types.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// The SMSN size as string length: each bytes contains two chars and the end of
// string character is added also.
#define SL_SIDEWALK_UTILS_SMSN_STR_LENGTH ((SID_PAL_MFG_STORE_SMSN_SIZE * 2) + 1)
#define SL_SIDEWALK_UTILS_CAPABILITIES_STR_MAX_LENGTH (255)
#define SL_SIDEWALK_UTILS_MAX_STORED_COMMANDS_NUM 5
#define SL_SIDEWALK_UTILS_MAX_COMMAND_LENGTH_CHAR 255
#define SL_SIDEWALK_UTILS_MAX_PENDING_MESSAGES_NUM 5

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * This function initializes the sidewalk utils component
 *****************************************************************************/
void sl_sidewalk_utils_init(void);

/**************************************************************************//**
 * This function provides the SMSN value as a string.
 *
 * @param smsn_buffer The output buffer for the SMSN as string
 * @param smsn_buffer_length Length of the output buffer
 *****************************************************************************/
void sl_sidewalk_utils_get_smsn_as_str(char *smsn_buffer, uint16_t smsn_buffer_length);

/**************************************************************************//**
 * This function provides the enabled capabilities as a string.
 *
 * @param my_param The output buffer for the capabilities as string
 * @param my_param Length of the output buffer
 *****************************************************************************/
void sl_sidewalk_utils_get_capabilities_str(char *capablities_str, uint8_t capablities_str_length);

#endif // SL_SIDEWALK_UTILS_H

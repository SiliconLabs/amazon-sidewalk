/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_display_types.h
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

#ifndef SL_SIDEWALK_DISPLAY_TYPES_H
#define SL_SIDEWALK_DISPLAY_TYPES_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define SL_SIDEWALK_DISPLAY_MAX_STR_LENGTH (128)

typedef struct {
  bool is_registered;
  bool is_time_synced;
  uint8_t link;
  uint16_t last_successful_tx_seq_num;
} sl_sidewalk_display_statistics_t;

typedef struct {
  uint32_t type;
  char text[SL_SIDEWALK_DISPLAY_MAX_STR_LENGTH];
  sl_sidewalk_display_statistics_t stats;
} sl_sidewalk_display_msg_t;

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

#endif // SL_SIDEWALK_DISPLAY_TYPES_H

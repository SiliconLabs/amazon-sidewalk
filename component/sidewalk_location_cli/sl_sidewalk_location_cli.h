/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_location_cli.h
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SIDEWALK_LOCATION_CLI_H
#define SL_SIDEWALK_LOCATION_CLI_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <sid_api.h>
#include <sid_location.h>
#if defined(SL_LOCATION_FULL)
#include <sid_pal_gnss_ifc.h>
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#if defined(SL_LOCATION_FULL)

// Size of the location buffer. It is expected to not exceed this size.
#define LOCATION_BUFFER_SIZE (50U)

// Simple location buffer type
typedef struct {
  uint8_t buffer[LOCATION_BUFFER_SIZE];
  size_t size;
} sl_sidewalk_location_cli_buffer_t;

#endif // defined(SL_LOCATION_FULL)

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

void sl_sidewalk_location_cli_callback(
  const struct sid_location_result *const result,
  void *context);

#ifdef __cplusplus
}
#endif

#endif // SL_SIDEWALK_LOCATION_CLI_H

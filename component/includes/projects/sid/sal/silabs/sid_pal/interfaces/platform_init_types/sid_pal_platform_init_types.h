/***************************************************************************//**
 * @file
 * @brief sid_pal_platform_init_types.h
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SID_PAL_PLATFORM_INIT_TYPES_H
#define SID_PAL_PLATFORM_INIT_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
#if defined(SL_RADIO_EXTERNAL)
#include <sx126x_config.h>
#elif defined(SL_RADIO_NATIVE)
#include <efr32xgxx_config.h>
#endif
#endif
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
typedef struct {
//place holder for platform specific init parameters
#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
#if defined(SL_RADIO_EXTERNAL)
  radio_sx126x_device_config_t *radio_cfg;
#elif defined(SL_RADIO_NATIVE)
  radio_efr32xgxx_device_config_t *radio_cfg;
#endif
#endif
} platform_specific_init_parameters_t;
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif  // SID_PAL_PLATFORM_INIT_TYPES_H

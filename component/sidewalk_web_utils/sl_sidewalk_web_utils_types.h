/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_web_utils_types.h
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
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SL_SIDEWALK_WEB_UTILS_TYPES_H
#define SL_SIDEWALK_WEB_UTILS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stddef.h>

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define SL_SIDEWALK_WEB_UTILS_C_BUTTON0_S_PRESS_DSC "+sbv+Button 0 Short Press"
#define SL_SIDEWALK_WEB_UTILS_C_BUTTON0_S_COUNTER_DSC "+siv+Button 0 Short Counter"
#define SL_SIDEWALK_WEB_UTILS_C_BUTTON0_M_PRESS_DSC "+sbv+Button 0 Mid Press"
#define SL_SIDEWALK_WEB_UTILS_C_BUTTON0_M_COUNTER_DSC "+siv+Button 0 Mid Counter"
#define SL_SIDEWALK_WEB_UTILS_C_BUTTON0_L_PRESS_DSC "+sbv+Button 0 Long Press"
#define SL_SIDEWALK_WEB_UTILS_C_BUTTON0_L_COUNTER_DSC "+siv+Button 0 Long Counter"
#define SL_SIDEWALK_WEB_UTILS_C_BUTTON1_S_PRESS_DSC "+sbv+Button 1 Short Press"
#define SL_SIDEWALK_WEB_UTILS_C_BUTTON1_S_COUNTER_DSC "+siv+Button 1 Short Counter"
#define SL_SIDEWALK_WEB_UTILS_C_BUTTON1_M_PRESS_DSC "+sbv+Button 1 Mid Press"
#define SL_SIDEWALK_WEB_UTILS_C_BUTTON1_M_COUNTER_DSC "+siv+Button 1 Mid Counter"
#define SL_SIDEWALK_WEB_UTILS_C_BUTTON1_L_PRESS_DSC "+sbv+Button 1 Long Press"
#define SL_SIDEWALK_WEB_UTILS_C_BUTTON1_L_COUNTER_DSC "+siv+Button 1 Long Counter"
#define SL_SIDEWALK_WEB_UTILS_C_CURRENT_LINK_REPORT_DSC "+st+Current Link"
#define SL_SIDEWALK_WEB_UTILS_C_ROOM_TEMPERATURE_DSC "+sic+Room Temperature"
#define SL_SIDEWALK_WEB_UTILS_C_CORE_TEMPERATURE_DSC "+sic+Core Temperature"
#define SL_SIDEWALK_WEB_UTILS_C_ROOM_TEMPERATURE_INTERVAL_DSC "+st+Timer Interval for Room Temperature [s]"
#define SL_SIDEWALK_WEB_UTILS_C_CORE_TEMPERATURE_INTERVAL_DSC "+st+Timer Interval for Core Temperature [s]"
#define SL_SIDEWALK_WEB_UTILS_C_LED0_SET_DSC "+ab+LED 0 Set"
#define SL_SIDEWALK_WEB_UTILS_C_LED0_TOGGLE_DSC "+am+LED 0 Toggle"
#define SL_SIDEWALK_WEB_UTILS_C_LED1_SET_DSC "+ab+LED 1 Set"
#define SL_SIDEWALK_WEB_UTILS_C_LED1_TOGGLE_DSC "+am+LED 1 Toggle"
#define SL_SIDEWALK_WEB_UTILS_C_LINK_SWITCH_TO_NEXT_DSC "+am+Switch to Next Link"
#define SL_SIDEWALK_WEB_UTILS_C_DISPLAY_TEXT_DSC "+at+Display Text"

typedef struct {
  char * id;
  char * dsc;
} sl_sidewalk_web_utils_capability_str_t;

#ifdef __cplusplus
}
#endif

#endif // SL_SIDEWALK_WEB_UTILS_TYPES_H

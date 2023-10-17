/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_sensor.h
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

#ifndef SL_SIDEWALK_SENSOR_H
#define SL_SIDEWALK_SENSOR_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

#include "sl_board_control_config.h"
#include "sl_sidewalk_sensor_types.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define SL_SIDEWALK_SENSOR_REPORT_MSG_MAX_SIZE_BYTES 64

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * Initializer of the sidewalk_sensor component
 *****************************************************************************/
void sl_sidewalk_sensor_init(void);

/**************************************************************************//**
 * Function reporting (queuing a message with) an information coming from a
 * sensor
 *
 * @param sensor Type of the sensor whose information is to be sent (e.g.,
 *               temperature, button)
 * @param value The information to be sent from the sensor
 *****************************************************************************/
void sl_sidewalk_sensor_report(sl_sidewalk_sensor_type_t sensor, char *value, sl_sidewalk_sender_priority_type_t priority);

#endif // SL_SIDEWALK_SENSOR_H

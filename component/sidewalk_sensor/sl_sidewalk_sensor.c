/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_sensor.c
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sl_sidewalk_sender.h"
#include "sl_sidewalk_sensor.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define MAX_REPORT_LENGTH_CHAR (64)

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static const char * SENSOR_PREFIXES[] = {
  [SL_SIDEWALK_SENSOR_TYPE_BUTTON0] = ":button0=",
  [SL_SIDEWALK_SENSOR_TYPE_BUTTON1] = ":button1=",
  [SL_SIDEWALK_SENSOR_TYPE_TEMPERATURE] = ":temperature=",
  [SL_SIDEWALK_SENSOR_TYPE_MESSAGE] = ":message=",
  [SL_SIDEWALK_SENSOR_TYPE_TONK] = ":tonk "
};

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sl_sidewalk_sensor_init(void)
{
  // Empty initializer kept to meet the sidewalk component pattern
}

void sl_sidewalk_sensor_report(sl_sidewalk_sensor_type_t sensor, char *value, sl_sidewalk_sender_priority_type_t priority)
{
  if ((sensor >= 0) && (sensor < SL_SIDEWALK_SENSOR_TYPE_END)) {
    if (value != NULL) {
      char msg_buffer[MAX_REPORT_LENGTH_CHAR];
      memset(msg_buffer, 0, sizeof(msg_buffer));

      strcat(msg_buffer, SENSOR_PREFIXES[sensor]);
      strcat(msg_buffer, value);
      sl_sidewalk_sender_queue_message(msg_buffer, strlen(msg_buffer), priority);
    }
  }
}

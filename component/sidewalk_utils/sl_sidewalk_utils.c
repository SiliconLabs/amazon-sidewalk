/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_utils.c
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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sid_pal_mfg_store_ifc.h"
#include "sl_sidewalk_utils.h"
#include "sl_component_catalog.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

static void smsn_to_text(uint8_t *smsn, char *text);
static bool get_raw_smsn(uint8_t *buffer, uint8_t buffer_len);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static uint8_t device_capabilities;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sl_sidewalk_utils_init(void)
{
  memset(&device_capabilities, 0, sizeof(device_capabilities));
}

void sl_sidewalk_utils_get_smsn_as_str(char *smsn_buffer, uint16_t smsn_buffer_length)
{
  if (smsn_buffer != NULL && smsn_buffer_length >= SL_SIDEWALK_UTILS_SMSN_STR_LENGTH) {
    uint8_t smsn_raw_buffer[SID_PAL_MFG_STORE_SMSN_SIZE];

    memset(smsn_buffer, 0, smsn_buffer_length);

    if (true == get_raw_smsn(smsn_raw_buffer, sizeof(smsn_raw_buffer))) {
      smsn_to_text(smsn_raw_buffer, smsn_buffer);
    }
  }
}

void sl_sidewalk_utils_get_capabilities_str(char *capablities_str, uint8_t capablities_str_length)
{
  if (capablities_str != NULL) {
    if (capablities_str_length >= SL_SIDEWALK_UTILS_CAPABILITIES_STR_MAX_LENGTH) {
      memset(capablities_str, 0, capablities_str_length);

      // TODO: read these versions from the current software instead of hardcoding
      strcat(capablities_str, "|prot+v1");
      strcat(capablities_str, "|sidewalk+1.14");
      strcat(capablities_str, "|gsdk+4.2.0");

#if defined(SL_CATALOG_BTN0_PRESENT)
      strcat(capablities_str, "|button0+sb+Push Button 0");
#endif

#if defined(SL_CATALOG_BTN1_PRESENT)
      strcat(capablities_str, "|button1+sb+Push Button 1");
#endif

#if defined(SL_CATALOG_LED0_PRESENT)
      strcat(capablities_str, "|led0+ab+LED 0");
#endif

#if defined(SL_CATALOG_LED1_PRESENT)
      strcat(capablities_str, "|led1+ab+LED 1");
#endif

#if defined(SL_TEMPERATURE_SENSOR_EXTERNAL)
      strcat(capablities_str, "|temperature+sic+Room Temperature");
#elif defined(SL_TEMPERATURE_SENSOR_INTERNAL)
      strcat(capablities_str, "|temperature+sic+Core Temperature");
#endif

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
      strcat(capablities_str, "|display+at+Display");
      strcat(capablities_str, "|message+st+Message");
#endif
    }
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static void smsn_to_text(uint8_t *smsn, char *text)
{
  if (smsn != NULL && text != NULL) {
    for (uint8_t i = 0; i < SID_PAL_MFG_STORE_SMSN_SIZE; i++) {
      sprintf(text + (i << 1), "%02X", smsn[i]);
    }
  }
}

static bool get_raw_smsn(uint8_t *buffer, uint8_t buffer_len)
{
  bool retval = false;
  uint8_t zero_buffer[SID_PAL_MFG_STORE_SMSN_SIZE];
  memset(zero_buffer, 0, sizeof(zero_buffer));

  if (buffer != NULL) {
    if (buffer_len >= SID_PAL_MFG_STORE_SMSN_SIZE) {
      sid_pal_mfg_store_read(SID_PAL_MFG_STORE_SMSN,
                             buffer,
                             SID_PAL_MFG_STORE_SMSN_SIZE);

      if (memcmp(buffer, zero_buffer, SID_PAL_MFG_STORE_SMSN_SIZE)) {
        retval = true;
      }
    }
  }

  return retval;
}

/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_utils.c
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <ctype.h>

#include "sid_pal_mfg_store_ifc.h"
#include "sid_pal_storage_kv_ifc.h"
#include "sid_pal_storage_kv_internal_group_ids.h"
#include "sl_sidewalk_utils.h"
#include "sl_component_catalog.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

static void smsn_to_text(const uint8_t *smsn, char *text);
static void sidewalk_id_to_text(const uint8_t *sidewalk_id, char *text);
static bool get_raw_smsn(uint8_t *buffer, uint8_t buffer_len);
static bool get_raw_sidewalk_id(uint8_t *buffer, uint8_t buffer_len);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static uint8_t device_capabilities = 0;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sl_sidewalk_utils_init(void)
{
  memset(&device_capabilities, 0, sizeof(device_capabilities));
}

bool sl_sidewalk_utils_is_data_ascii(const char *data, uint16_t data_len)
{
  if (data == NULL) {
    return false;
  }
  for (uint16_t i = 0; i < data_len; ++i) {
    if (!(isalnum((unsigned char)data[i])
          || isspace((unsigned char)data[i])
          || ispunct((unsigned char)data[i]))) {
      return false; // Found a character that is not alphanumeric or punctuation
    }
  }
  return true; // All characters are alphanumeric or punctuation
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

void sl_sidewalk_utils_get_sidewalk_id_as_str(char *sidewalk_id_buffer, uint16_t sidewalk_id_buffer_length)
{
  if (sidewalk_id_buffer != NULL && sidewalk_id_buffer_length >= SL_SIDEWALK_UTILS_SIDEWALK_ID_STR_LENGTH) {
    uint8_t sidewalk_id_raw_buffer[SID_PAL_MFG_STORE_DEVID_SIZE];

    memset(sidewalk_id_buffer, 0, sidewalk_id_buffer_length);

    if (true == get_raw_sidewalk_id(sidewalk_id_raw_buffer, sizeof(sidewalk_id_raw_buffer))) {
      sidewalk_id_to_text(sidewalk_id_raw_buffer, sidewalk_id_buffer);
    }
  }
}

void sl_sidewalk_utils_get_capabilities_str(char *capablities_str, uint8_t capablities_str_length)
{
  if (capablities_str == NULL) {
    return;
  }

  if (capablities_str_length >= SL_SIDEWALK_UTILS_CAPABILITIES_STR_MAX_LENGTH) {
    memset(capablities_str, 0, capablities_str_length);

    // TODO: read these versions from the current software instead of hardcoding
    strcat(capablities_str, "|prot+v1");
    strcat(capablities_str, "|sidewalk+1.16");
    strcat(capablities_str, "|gsdk+4.4.1");

#if defined(SL_CATALOG_BTN0_PRESENT)
    strcat(capablities_str, "|message+st+Message");
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
#endif
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static void smsn_to_text(const uint8_t *smsn, char *text)
{
  if (smsn != NULL && text != NULL) {
    for (uint8_t i = 0; i < SID_PAL_MFG_STORE_SMSN_SIZE; i++) {
      sprintf(text + (i << 1), "%02X", smsn[i]);
    }
  }
}

static void sidewalk_id_to_text(const uint8_t *sidewalk_id, char *text)
{
  if (sidewalk_id != NULL && text != NULL) {
    for (uint8_t i = 0; i < SID_PAL_MFG_STORE_DEVID_SIZE; i++) {
      sprintf(text + (i << 1), "%02X", sidewalk_id[i]);
    }
  }
}

static bool get_raw_smsn(uint8_t *buffer, uint8_t buffer_len)
{
  bool retval = false;
  uint8_t zero_buffer[SID_PAL_MFG_STORE_SMSN_SIZE];

  if (buffer == NULL) {
    return false;
  }

  memset(zero_buffer, 0, sizeof(zero_buffer));

  if (buffer_len >= SID_PAL_MFG_STORE_SMSN_SIZE) {
    sid_pal_mfg_store_read(SID_PAL_MFG_STORE_SMSN,
                           buffer,
                           SID_PAL_MFG_STORE_SMSN_SIZE);

    if (memcmp(buffer, zero_buffer, SID_PAL_MFG_STORE_SMSN_SIZE)) {
      retval = true;
    }
  }

  return retval;
}

static bool get_raw_sidewalk_id(uint8_t *buffer, uint8_t buffer_len)
{
  bool retval = false;
  uint8_t zero_buffer[SID_PAL_MFG_STORE_DEVID_SIZE];

  if (buffer == NULL) {
    return false;
  }

  memset(zero_buffer, 0, sizeof(zero_buffer));

  if (buffer_len >= SID_PAL_MFG_STORE_DEVID_SIZE
      && !sid_pal_mfg_store_dev_id_get(buffer)) {
    sid_error_t ret = sid_pal_storage_kv_record_get(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID,
                                                    43, // todo: get rid of magic number once the key is publicly defined
                                                    buffer,
                                                    SID_PAL_MFG_STORE_DEVID_SIZE);
    if (ret == SID_ERROR_NONE && memcmp(buffer, zero_buffer, SID_PAL_MFG_STORE_DEVID_SIZE)) {
      retval = true;
    }
  }

  return retval;
}

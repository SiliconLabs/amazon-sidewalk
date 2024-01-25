/***************************************************************************//**
 * @file
 * @brief mfg_store.c
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
 *  claim that you wrote the original software. If you use this software
 *  in a product, an acknowledgment in the product documentation would be
 *  appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *  misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <sid_pal_mfg_store_ifc.h>
#include <sid_pal_log_ifc.h>
#include <stdalign.h>
#include <stdint.h>
#include <string.h>
#include "nvm3_manager.h"
#include "em_system.h" // for SYSTEM_GetUnique
#include "sl_malloc.h"

/* Manufacturing store write capability is not enabled by default. It
 * is currently required for internal diagnostic apps and for SWAT
 */
#if (defined (HALO_ENABLE_DIAGNOSTICS) && HALO_ENABLE_DIAGNOSTICS) || defined(SWAT_DEVICE_TYPE) \
  || (defined(SL_SID_PDP_FEATURE_ON) && SL_SID_PDP_FEATURE_ON)
#define ENABLE_MFG_STORE_WRITE
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define MFG_VERSION_1_VAL                   0x01000000
#define MFG_VERSION_2_VAL                   0x2

#define ENCODED_DEV_ID_SIZE_5_BYTES_MASK    0xA0
#define DEV_ID_MSB_MASK                     0x1F

#if defined(__LITTLE_ENDIAN__) || (defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN) \
  || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
// Converts from network (32-bit) order to host byte order.
#define SLI_NTOHL(netlong) __builtin_bswap32(netlong)
#else
#define SLI_NTOHL(netlong) (netlong)
#endif

enum mfg_store_error_status {
  MFG_STORE_ERROR_ST_SUCCESS = 0,
  MFG_STORE_ERROR_ST_WRONG_KEY = -1,
  MFG_STORE_ERROR_ST_WRONG_INPUT_ARGS = -2,
  MFG_STORE_ERROR_ST_WRITE_ERROR = -3,
  MFG_STORE_ERROR_ST_REPACK_ERROR = -4,
  MFG_STORE_ERROR_ST_DELETE_ERROR = -5,
  MFG_STORE_ERROR_ST_OUT_OF_MEMORY = -6,
  MFG_STORE_ERROR_ST_WRITE_NOT_ACTIVATED = -7,
  MFG_STORE_ERROR_ST_ERASE_NOT_ACTIVATED = -8,
};

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
static const uint32_t MFG_WORD_SIZE = 4;  // in bytes

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sid_pal_mfg_store_init(sid_pal_mfg_store_region_t mfg_store_region)
{
  (void)mfg_store_region;

  if (!nvm3_defaultHandle->hasBeenOpened) {
    Ecode_t status = nvm3_initDefault();
    if (ECODE_NVM3_OK != status) {
      SID_PAL_LOG_ERROR("pal: mfg store init err: %d", status);
      return;
    }
  }

  uint16_t obj_cnt = (uint16_t)nvm3_enumObjects(nvm3_defaultHandle, NULL, 0, SLI_SID_NVM3_KEY_MIN_MFG, SLI_SID_NVM3_KEY_MAX_MFG);
  SID_PAL_LOG_INFO("pal: mfg store opened with %d object(s)", obj_cnt);
}

void sid_pal_mfg_store_deinit(void)
{
  // do not deinit default nvm3 instance as it is also used by gsdk
}

int32_t sid_pal_mfg_store_write(uint16_t value, const uint8_t *buffer, uint16_t length)
{
#ifdef ENABLE_MFG_STORE_WRITE
  if (!SLI_SID_NVM3_VALIDATE_KEY(MFG, value)) {
    SID_PAL_LOG_ERROR("pal: mfg write, key 0x%.5x not in range (0x%.5x - 0x%.5x)", value, SLI_SID_NVM3_KEY_MIN_MFG_REL, SLI_SID_NVM3_KEY_MAX_MFG_REL);
    return MFG_STORE_ERROR_ST_WRONG_KEY;
  }

  if (!buffer || length == 0) {
    SID_PAL_LOG_ERROR("pal: mfg write, wrong input args");
    return MFG_STORE_ERROR_ST_WRONG_INPUT_ARGS;
  }

  Ecode_t status = nvm3_writeData(nvm3_defaultHandle, SLI_SID_NVM3_MAP_KEY(MFG, value), buffer, (size_t)length);
  if (status != ECODE_NVM3_OK) {
    SID_PAL_LOG_ERROR("pal: mfg write, write err: %d", status);
    return MFG_STORE_ERROR_ST_WRITE_ERROR;
  }

  if (nvm3_repackNeeded(nvm3_defaultHandle)) {
    status = nvm3_repack(nvm3_defaultHandle);
    if (status != ECODE_NVM3_OK) {
      SID_PAL_LOG_ERROR("pal: mfg write, repack err: %d", status);
      return MFG_STORE_ERROR_ST_REPACK_ERROR;
    }
  }

  return MFG_STORE_ERROR_ST_SUCCESS;
#else
  (void)value;
  (void)buffer;
  (void)length;

  SID_PAL_LOG_WARNING("pal: mfg write, write not activated");

  return MFG_STORE_ERROR_ST_WRITE_NOT_ACTIVATED;
#endif
}

void sid_pal_mfg_store_read(uint16_t value, uint8_t *buffer, uint16_t length)
{
  uint32_t object_type;
  size_t object_length;
  uint32_t mapped_key = SLI_SID_NVM3_MAP_KEY(MFG, value);

  if (!SLI_SID_NVM3_VALIDATE_KEY(MFG, value)) {
    SID_PAL_LOG_ERROR("pal: mfg read, key 0x%.5x not in range (0x%.5x - 0x%.5x)", value, SLI_SID_NVM3_KEY_MIN_MFG_REL, SLI_SID_NVM3_KEY_MAX_MFG_REL);
    return;
  }

  if (!buffer || length == 0) {
    SID_PAL_LOG_ERROR("pal: mfg read, wrong input args");
    return;
  }

  nvm3_getObjectInfo(nvm3_defaultHandle, mapped_key, &object_type, &object_length);

  if (object_type == NVM3_OBJECTTYPE_DATA) {
    nvm3_readData(nvm3_defaultHandle, mapped_key, buffer, (size_t)length);
  }
}

uint16_t sid_pal_mfg_store_get_length_for_value(uint16_t value)
{
  uint32_t object_type;
  size_t object_length = 0;

  if (!SLI_SID_NVM3_VALIDATE_KEY(MFG, value)) {
    SID_PAL_LOG_ERROR("pal: mfg get len for value, key 0x%.5x not in range (0x%.5x - 0x%.5x)", value, SLI_SID_NVM3_KEY_MIN_MFG_REL, SLI_SID_NVM3_KEY_MAX_MFG_REL);
    return object_length;
  }

  nvm3_getObjectInfo(nvm3_defaultHandle, SLI_SID_NVM3_MAP_KEY(MFG, value), &object_type, &object_length);

  return (object_type == NVM3_OBJECTTYPE_DATA) ? object_length : 0;
}

int32_t sid_pal_mfg_store_erase(void)
{
#ifdef ENABLE_MFG_STORE_WRITE
  uint32_t obj_cnt;
  nvm3_ObjectKey_t *key_list = NULL;
  Ecode_t status = ECODE_NVM3_OK;

  obj_cnt = nvm3_enumObjects(nvm3_defaultHandle, NULL, 0, SLI_SID_NVM3_KEY_MIN_MFG, SLI_SID_NVM3_KEY_MAX_MFG);
  if (obj_cnt == 0) {
    SID_PAL_LOG_INFO("pal: mfg erase, nothing to erase");
    return MFG_STORE_ERROR_ST_SUCCESS;
  }

  key_list = (nvm3_ObjectKey_t *)sl_calloc(obj_cnt, sizeof(nvm3_ObjectKey_t));
  if (!key_list) {
    SID_PAL_LOG_ERROR("pal: mfg erase, out of memory");
    return MFG_STORE_ERROR_ST_OUT_OF_MEMORY;
  }

  nvm3_enumObjects(nvm3_defaultHandle, key_list, obj_cnt, SLI_SID_NVM3_KEY_MIN_MFG, SLI_SID_NVM3_KEY_MAX_MFG);
  for (uint32_t i = 0; i < obj_cnt; i++) {
    status = nvm3_deleteObject(nvm3_defaultHandle, key_list[i]);
    if (ECODE_NVM3_OK != status) {
      SID_PAL_LOG_ERROR("pal: mfg erase, erase err: %d", status);
      sl_free(key_list);
      return MFG_STORE_ERROR_ST_DELETE_ERROR;
    }
  }

  sl_free(key_list);
  key_list = NULL;

  return MFG_STORE_ERROR_ST_SUCCESS;
#else
  SID_PAL_LOG_WARNING("pal: mfg erase, erase not activated");

  return MFG_STORE_ERROR_ST_ERASE_NOT_ACTIVATED;
#endif
}

bool sid_pal_mfg_store_is_tlv_support(void)
{
  return true;
}

uint32_t sid_pal_mfg_store_get_version(void)
{
  uint32_t version;

  sid_pal_mfg_store_read(SID_PAL_MFG_STORE_VERSION, (uint8_t *)&version, SID_PAL_MFG_STORE_VERSION_SIZE);
  // Assuming that we keep this behavior for both 1P & 3P
  return SLI_NTOHL(version);
}

bool sid_pal_mfg_store_dev_id_get(uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE])
{
  bool error_code = false;
  uint8_t buffer[SID_PAL_MFG_STORE_DEVID_SIZE] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  const uint8_t unset_dev_id[SID_PAL_MFG_STORE_DEVID_SIZE] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DEVID, buffer, SID_PAL_MFG_STORE_DEVID_SIZE);

  if (memcmp(buffer, unset_dev_id, SID_PAL_MFG_STORE_DEVID_SIZE) == 0) {
    uint64_t full_uniq = SYSTEM_GetUnique();
    uint32_t low = full_uniq & 0x0000FFFF;
    buffer[0] = 0xBF;
    buffer[1] = 0xFF;
    buffer[2] = 0xFF;
    buffer[3] = (low >> 8) & 0xFF;
    buffer[4] = low & 0xFF;
  } else {
    const uint32_t version = sid_pal_mfg_store_get_version();

    if ((MFG_VERSION_1_VAL == version) || (0x1 == version)) {
      // Correct dev_id for mfg version 1
      // For devices with mfg version 1, the device Id is stored as two words
      // in network endian format.
      // To read the device Id two words at SID_PAL_MFG_STORE_DEVID has to be
      // read and each word needs to be changed to host endian format.
      uint8_t dev_id_buffer[2 * MFG_WORD_SIZE];
      uint32_t val = 0;
      sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DEVID, dev_id_buffer, sizeof(dev_id_buffer));
      memcpy(&val, &dev_id_buffer[0], sizeof(val));
      val = SLI_NTOHL(val);
      memcpy(&dev_id_buffer[0], &val, sizeof(val));
      memcpy(&val, &dev_id_buffer[MFG_WORD_SIZE], sizeof(val));
      val = SLI_NTOHL(val);
      memcpy(&dev_id_buffer[MFG_WORD_SIZE], &val, sizeof(val));
      // Encode the size in the first 3 bits in MSB of the devId
      dev_id_buffer[0] = (dev_id_buffer[0] & DEV_ID_MSB_MASK) | ENCODED_DEV_ID_SIZE_5_BYTES_MASK;
      memcpy(buffer, dev_id_buffer, SID_PAL_MFG_STORE_DEVID_SIZE);
    }

    error_code = true;
  }

  memcpy(dev_id, buffer, SID_PAL_MFG_STORE_DEVID_SIZE);

  return error_code;
}

bool sid_pal_mfg_store_serial_num_get(uint8_t serial_num[SID_PAL_MFG_STORE_SERIAL_NUM_SIZE])
{
  uint32_t buffer[(SID_PAL_MFG_STORE_SERIAL_NUM_SIZE + (MFG_WORD_SIZE - 1)) / MFG_WORD_SIZE];

  sid_pal_mfg_store_read(SID_PAL_MFG_STORE_SERIAL_NUM, (uint8_t *)buffer, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);

  static const uint8_t unset_serial_num[SID_PAL_MFG_STORE_SERIAL_NUM_SIZE] =
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };

  if (memcmp(buffer, unset_serial_num, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE) == 0) {
    return false;
  }

  const uint32_t version = sid_pal_mfg_store_get_version();

  // TODO: HALO-5169
  if ((MFG_VERSION_1_VAL == version) || (0x1 == version)) {
    for (unsigned int i = 0; i < sizeof(buffer) / sizeof(buffer[0]); ++i) {
      buffer[i] = SLI_NTOHL(buffer[i]);
    }
  }

  memcpy(serial_num, buffer, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);

  return true;
}

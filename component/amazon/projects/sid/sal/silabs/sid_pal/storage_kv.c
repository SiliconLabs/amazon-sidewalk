/***************************************************************************//**
 * @file
 * @brief storage_kv.c
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

#include <sid_pal_storage_kv_ifc.h>
#if defined(SL_SIDEWALK_UNIT_TEST)
#include "sl_sidewalk_log_pal_mock.h"
#else
#include "sl_sidewalk_log_pal.h"
#endif
#include <sid_pal_assert_ifc.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "nvm3_manager.h"
#include "sl_memory_manager.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

struct storage_kv_record_header {
  uint32_t data_size;
  uint16_t key;
};

#define STORAGE_KV_REC_HDR_SIZE (sizeof(struct storage_kv_record_header))

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

sid_error_t sid_pal_storage_kv_init(void)
{
  uint16_t obj_cnt = 0U;

  if (!nvm3_defaultHandle->hasBeenOpened
      && ECODE_NVM3_OK != nvm3_initDefault()) {
    SL_SID_LOG_PAL_ERROR("pal kv-storage: kv store init failed");
    return SID_ERROR_GENERIC;
  }

  obj_cnt = (uint16_t)nvm3_enumObjects(nvm3_defaultHandle,
                                       NULL,
                                       0,
                                       SLI_SID_NVM3_KEY_MIN_KV,
                                       SLI_SID_NVM3_KEY_MAX_KV);
  SL_SID_LOG_PAL_INFO("pal kv-storage: kv store opened with %d object(s)", obj_cnt);

  return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_deinit(void)
{
  // do not deinit default nvm3 instance as it is also used by gsdk

  return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_record_get(uint16_t group,
                                          uint16_t key,
                                          void *p_data,
                                          uint32_t len)
{
  Ecode_t status = ECODE_NVM3_OK;
  struct storage_kv_record_header record_header;
  size_t offset_in_object = 0;
  uint32_t object_type = 0U;
  size_t object_length = 0;
  uint32_t mapped_key = SLI_SID_NVM3_MAP_KEY(KV, group);

  if (!SLI_SID_NVM3_VALIDATE_KEY(KV, group)) {
    SL_SID_LOG_PAL_ERROR("pal kv-storage: kv record get, key 0x%.5x not in range (0x%.5x - 0x%.5x)",
                         group, SLI_SID_NVM3_KEY_MIN_KV_REL, SLI_SID_NVM3_KEY_MAX_KV_REL);
    return SID_ERROR_PARAM_OUT_OF_RANGE;
  }

  if (!p_data) {
    SL_SID_LOG_PAL_ERROR("pal kv-storage: kv record get, null ptr");
    return SID_ERROR_NULL_POINTER;
  }

  (void)nvm3_getObjectInfo(nvm3_defaultHandle,
                           mapped_key,
                           &object_type,
                           &object_length);

  if (object_length > 0) {
    while (object_length > offset_in_object) {
      status = nvm3_readPartialData(nvm3_defaultHandle,
                                    mapped_key,
                                    &record_header,
                                    offset_in_object,
                                    STORAGE_KV_REC_HDR_SIZE);
      if (status != ECODE_NVM3_OK) {
        break;
      }

      if (record_header.key == key) {
        // Record found
        status = nvm3_readPartialData(nvm3_defaultHandle,
                                      mapped_key,
                                      p_data,
                                      offset_in_object + STORAGE_KV_REC_HDR_SIZE,
                                      len);
        break;
      }

      offset_in_object += STORAGE_KV_REC_HDR_SIZE + record_header.data_size;
      if (object_length <= offset_in_object) {
        status = ECODE_NVM3_ERR_KEY_NOT_FOUND;
        break;
      }
    }

    if (object_length <= offset_in_object) {
      status = ECODE_NVM3_ERR_KEY_NOT_FOUND;
    }
  } else {
    status = ECODE_NVM3_ERR_KEY_NOT_FOUND;
  }

  return sli_sid_nvm3_convert_ecode_to_sid_error(status);
}

sid_error_t sid_pal_storage_kv_record_get_len(uint16_t group,
                                              uint16_t key,
                                              uint32_t *p_len)
{
  Ecode_t status = ECODE_NVM3_OK;
  struct storage_kv_record_header record_header;
  size_t offset_in_object = 0;
  uint32_t object_type;
  size_t object_length = 0;
  uint32_t mapped_key = SLI_SID_NVM3_MAP_KEY(KV, group);

  if (!SLI_SID_NVM3_VALIDATE_KEY(KV, group)) {
    SL_SID_LOG_PAL_ERROR("pal kv-storage: kv record get len, key 0x%.5x not in range (0x%.5x - 0x%.5x)",
                         group, SLI_SID_NVM3_KEY_MIN_KV_REL, SLI_SID_NVM3_KEY_MAX_KV_REL);
    return SID_ERROR_PARAM_OUT_OF_RANGE;
  }

  if (!p_len) {
    SL_SID_LOG_PAL_ERROR("pal kv-storage: kv record get len, null ptr");
    return SID_ERROR_NULL_POINTER;
  }

  (void)nvm3_getObjectInfo(nvm3_defaultHandle,
                           mapped_key,
                           &object_type,
                           &object_length);

  if (object_length > 0) {
    while (object_length > offset_in_object) {
      status = nvm3_readPartialData(nvm3_defaultHandle,
                                    mapped_key,
                                    &record_header,
                                    offset_in_object,
                                    STORAGE_KV_REC_HDR_SIZE);
      if (status != ECODE_NVM3_OK) {
        break;
      }

      if (record_header.key == key) {
        // Record found
        *p_len = record_header.data_size;
        break;
      }
      offset_in_object += STORAGE_KV_REC_HDR_SIZE + record_header.data_size;
    }

    if (object_length <= offset_in_object) {
      status = ECODE_NVM3_ERR_KEY_NOT_FOUND;
    }
  } else {
    status = ECODE_NVM3_ERR_KEY_NOT_FOUND;
  }

  return sli_sid_nvm3_convert_ecode_to_sid_error(status);
}

// "[Record] data must be aligned to a 4 byte boundary"
// TODO:HALO-7063 - Return workaround rfds_wr_buf[256] to rfds_wr_buf[64]
// alignas(4) static uint8_t rfds_wr_buf[256];
sid_error_t sid_pal_storage_kv_record_set(uint16_t group,
                                          uint16_t key,
                                          void const *p_data,
                                          uint32_t len)
{
  static uint8_t read_buf[SID_PAL_KV_STORE_MAX_LENGTH_BYTES] = { 0U };
  sid_error_t is_record_exist = SID_ERROR_NONE;
  Ecode_t status = ECODE_NVM3_OK;
  uint32_t object_type = 0;
  size_t data_len = 0;
  size_t new_object_size = 0;
  uint8_t *raw_file_buffer = NULL;
  uint32_t mapped_key = 0U;

  if (!len) {
    return SID_ERROR_INVALID_ARGS;
  }

  SID_PAL_ASSERT(len <= SID_PAL_KV_STORE_MAX_LENGTH_BYTES);

  if (!p_data) {
    SL_SID_LOG_PAL_ERROR("pal kv-storage: kv record set, null ptr");
    return SID_ERROR_NULL_POINTER;
  }

  if (sid_pal_storage_kv_record_get(group,
                                    key,
                                    read_buf,
                                    len) == SID_ERROR_NONE) {
    // When the value to be written is already present, do nothing and return success.
    if (memcmp(read_buf, p_data, len) == 0) {
      return SID_ERROR_NONE;
    }
    // Record with the same key but with different data is already in the object
  } else {
    // Record not present in the object
    is_record_exist = SID_ERROR_NOT_FOUND;
  }

  struct storage_kv_record_header new_record_header = {
    .key       = key,
    .data_size = len
  };
  mapped_key = SLI_SID_NVM3_MAP_KEY(KV, group);

  if (!SLI_SID_NVM3_VALIDATE_KEY(KV, group)) {
    SL_SID_LOG_PAL_ERROR("pal kv-storage: kv record set, key 0x%.5x not in range (0x%.5x - 0x%.5x)",
                         group, SLI_SID_NVM3_KEY_MIN_KV_REL, SLI_SID_NVM3_KEY_MAX_KV_REL);
    return SID_ERROR_PARAM_OUT_OF_RANGE;
  }

  status = nvm3_getObjectInfo(nvm3_defaultHandle,
                              mapped_key,
                              &object_type,
                              &data_len);

  if (status == ECODE_NVM3_ERR_KEY_NOT_FOUND) {
    // First item in the group
    new_object_size = STORAGE_KV_REC_HDR_SIZE + len;

    // This part is needed because NVM3 does NOT support object append at the moment!
    raw_file_buffer = (uint8_t *)sl_calloc(new_object_size, sizeof(uint8_t));
    if (raw_file_buffer == NULL) {
      status = ECODE_NVM3_ERR_INT_SIZE_ERROR;
    } else {
      // Copy the new entry header into the buffer
      memcpy(raw_file_buffer, &new_record_header, STORAGE_KV_REC_HDR_SIZE);
      // Copy the actual data after the header
      memcpy(&raw_file_buffer[STORAGE_KV_REC_HDR_SIZE], p_data, len);

      status = nvm3_writeData(nvm3_defaultHandle,
                              mapped_key,
                              raw_file_buffer,
                              new_object_size);
      sl_free(raw_file_buffer);
    }
  } else if (status == ECODE_NVM3_OK) {
    do {
      if (data_len > 0 && is_record_exist == SID_ERROR_NONE) {
        status = sid_pal_storage_kv_record_delete(group, key);
        if (status != SID_ERROR_NONE) {
          break;
        }

        status = nvm3_getObjectInfo(nvm3_defaultHandle,
                                    mapped_key,
                                    &object_type,
                                    &data_len);
        if (status != ECODE_NVM3_OK) {
          break;
        }
      }
      new_object_size = data_len + len + STORAGE_KV_REC_HDR_SIZE;
      raw_file_buffer = (uint8_t *)sl_calloc(new_object_size, sizeof(uint8_t));
      if (raw_file_buffer == NULL) {
        status = ECODE_NVM3_ERR_INT_SIZE_ERROR;
        break;
      }

      if (data_len > 0) {
        status = nvm3_readData(nvm3_defaultHandle, mapped_key, raw_file_buffer, data_len);
        if (status != ECODE_NVM3_OK) {
          break;
        }
      }

      // Copy the new entry header into the buffer
      memcpy(&raw_file_buffer[data_len], &new_record_header, STORAGE_KV_REC_HDR_SIZE);
      // Copy the actual data after the header
      memcpy(&raw_file_buffer[data_len + STORAGE_KV_REC_HDR_SIZE], p_data, len);

      status = nvm3_writeData(nvm3_defaultHandle,
                              mapped_key,
                              raw_file_buffer,
                              new_object_size);
      if (status == ECODE_NVM3_OK
          && nvm3_repackNeeded(nvm3_defaultHandle)) {
        status = nvm3_repack(nvm3_defaultHandle);
      }
    } while (0);

    sl_free(raw_file_buffer);
  }

  return sli_sid_nvm3_convert_ecode_to_sid_error(status);
}

sid_error_t sid_pal_storage_kv_group_delete(uint16_t group)
{
  Ecode_t status = ECODE_NVM3_OK;

  if (!SLI_SID_NVM3_VALIDATE_KEY(KV, group)) {
    SL_SID_LOG_PAL_ERROR("pal kv-storage: kv group delete, key 0x%.5x not in range (0x%.5x - 0x%.5x)",
                         group, SLI_SID_NVM3_KEY_MIN_KV_REL, SLI_SID_NVM3_KEY_MAX_KV_REL);
    return SID_ERROR_PARAM_OUT_OF_RANGE;
  }

  status = nvm3_deleteObject(nvm3_defaultHandle, SLI_SID_NVM3_MAP_KEY(KV, group));
  if (status == ECODE_NVM3_ERR_KEY_NOT_FOUND) {
    status = ECODE_NVM3_OK;
  }

  return sli_sid_nvm3_convert_ecode_to_sid_error(status);
}

sid_error_t sid_pal_storage_kv_record_delete(uint16_t group, uint16_t key)
{
  Ecode_t status = ECODE_NVM3_OK;
  struct storage_kv_record_header record_header;
  uint32_t object_type = 0;
  size_t data_len = 0;
  size_t offset_in_object = 0;
  uint8_t *raw_file_buffer = NULL;
  uint32_t mapped_key = SLI_SID_NVM3_MAP_KEY(KV, group);

  if (!SLI_SID_NVM3_VALIDATE_KEY(KV, group)) {
    SL_SID_LOG_PAL_ERROR("pal kv-storage: kv record delete, key 0x%.5x not in range (0x%.5x - 0x%.5x)",
                         group, SLI_SID_NVM3_KEY_MIN_KV_REL, SLI_SID_NVM3_KEY_MAX_KV_REL);
    return SID_ERROR_PARAM_OUT_OF_RANGE;
  }

  do {
    status = nvm3_getObjectInfo(nvm3_defaultHandle,
                                mapped_key,
                                &object_type,
                                &data_len);

    if ((status != ECODE_NVM3_OK) && (status != ECODE_NVM3_ERR_KEY_NOT_FOUND)) {
      break;
    }
    raw_file_buffer = (uint8_t *)sl_calloc(data_len, sizeof(uint8_t));

    if (raw_file_buffer == NULL) {
      status = ECODE_NVM3_ERR_INT_SIZE_ERROR;
      break;
    }

    while (data_len > offset_in_object) {
      status = nvm3_readPartialData(nvm3_defaultHandle,
                                    mapped_key,
                                    &record_header,
                                    offset_in_object,
                                    STORAGE_KV_REC_HDR_SIZE);

      if (status != ECODE_NVM3_OK) {
        break;
      }

      if (record_header.key == key) {
        // Record found
        status = nvm3_readPartialData(nvm3_defaultHandle,
                                      mapped_key,
                                      raw_file_buffer,
                                      0,
                                      offset_in_object);

        if (status != ECODE_NVM3_OK) {
          break;
        }
        status = nvm3_readPartialData(nvm3_defaultHandle,
                                      mapped_key,
                                      &raw_file_buffer[offset_in_object],
                                      offset_in_object + record_header.data_size + STORAGE_KV_REC_HDR_SIZE,
                                      data_len - offset_in_object - record_header.data_size - STORAGE_KV_REC_HDR_SIZE);

        if (status != ECODE_NVM3_OK) {
          break;
        }

        status = nvm3_writeData(nvm3_defaultHandle,
                                mapped_key,
                                raw_file_buffer,
                                data_len - record_header.data_size - STORAGE_KV_REC_HDR_SIZE);

        /* status check is not neccessary, break command is issued anyway */
        break;
      }

      offset_in_object += STORAGE_KV_REC_HDR_SIZE + record_header.data_size;
    }

    if (data_len <= offset_in_object) {
      status = ECODE_NVM3_ERR_KEY_NOT_FOUND;
    }
  } while (0);

  sl_free(raw_file_buffer);

  return sli_sid_nvm3_convert_ecode_to_sid_error(status);
}

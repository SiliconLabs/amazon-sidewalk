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
#include <sid_pal_log_ifc.h>
#include <sid_pal_assert_ifc.h>

#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <nvm3_manager.h>

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
static nvm3_Handle_t *kv_nvm3_handle;

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static sid_error_t sid_pal_silabs_storage_kv_is_record_exist_in_group(uint16_t group,
                                                                      uint16_t key,
                                                                      size_t *record_offset);

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
sid_error_t sid_pal_storage_kv_init()
{
  sid_error_t retval = SID_ERROR_NONE;

  if (is_kv_nvm3_initialized() == false) {
    nvm3_manager_init();
  }
  kv_nvm3_handle = get_kv_nvm3_handle();
  return retval;
}

sid_error_t sid_pal_storage_kv_record_get(uint16_t group,
                                          uint16_t key,
                                          void *p_data,
                                          uint32_t len)
{
  Ecode_t res = ECODE_NVM3_OK;
  struct storage_kv_record_header record_header;
  size_t offset_in_object = 0;
  uint32_t type;
  size_t obj_len = 0;

  nvm3_getObjectInfo(kv_nvm3_handle, group, &type, &obj_len);

  if (obj_len > 0) {
    while (obj_len > offset_in_object) {
      res = nvm3_readPartialData(kv_nvm3_handle, group, &record_header, offset_in_object, STORAGE_KV_REC_HDR_SIZE);

      if (res != ECODE_NVM3_OK) {
        break;
      }

      if (record_header.key == key) {
        // Record found
        res = nvm3_readPartialData(kv_nvm3_handle, group, p_data, offset_in_object + STORAGE_KV_REC_HDR_SIZE, len);
        break;
      }
      offset_in_object += STORAGE_KV_REC_HDR_SIZE + record_header.data_size;

      if (obj_len <= offset_in_object) {
        res = ECODE_NVM3_ERR_KEY_NOT_FOUND;
        break;
      }
    }
    if (obj_len <= offset_in_object) {
      res = ECODE_NVM3_ERR_KEY_NOT_FOUND;
    }
  } else {
    res = ECODE_NVM3_ERR_KEY_NOT_FOUND;
  }
  return nvm3_manager_nvm3_to_sid_error_code_translation(res);
}

sid_error_t sid_pal_storage_kv_record_get_len(uint16_t group,
                                              uint16_t key,
                                              uint32_t * p_len)
{
  Ecode_t res = ECODE_NVM3_OK;
  struct storage_kv_record_header record_header;
  size_t offset_in_object = 0;

  uint32_t type;
  size_t obj_len = 0;

  nvm3_getObjectInfo(kv_nvm3_handle, group, &type, &obj_len);

  if (obj_len > 0) {
    while (obj_len > offset_in_object) {
      res = nvm3_readPartialData(kv_nvm3_handle, group, &record_header, offset_in_object, STORAGE_KV_REC_HDR_SIZE);

      if (res != ECODE_NVM3_OK) {
        break;
      }

      if (record_header.key == key) {
        // Record found
        *p_len = record_header.data_size;
        break;
      }
      offset_in_object += STORAGE_KV_REC_HDR_SIZE + record_header.data_size;
    }
    if (obj_len <= offset_in_object) {
      res = ECODE_NVM3_ERR_KEY_NOT_FOUND;
    }
  } else {
    res = ECODE_NVM3_ERR_KEY_NOT_FOUND;
  }

  return nvm3_manager_nvm3_to_sid_error_code_translation(res);
}

// "[Record] data must be aligned to a 4 byte boundary"
// TODO:HALO-7063 - Return workaround rfds_wr_buf[256] to rfds_wr_buf[64]
// alignas(4) static uint8_t rfds_wr_buf[256];
sid_error_t sid_pal_storage_kv_record_set(uint16_t group,
                                          uint16_t key,
                                          void const * p_data,
                                          uint32_t len)
{
  SID_PAL_ASSERT(len <= SID_PAL_KV_STORE_MAX_LENGTH_BYTES);

  static uint8_t read_buf[SID_PAL_KV_STORE_MAX_LENGTH_BYTES];
  memset(read_buf, 0, sizeof(read_buf));
  sid_error_t sid_ret = sid_pal_storage_kv_record_get(group, key, read_buf, len);
  // When the value to be written is already present, do nothing and return success.
  if (sid_ret == SID_ERROR_NONE && memcmp(read_buf, p_data, len) == 0) {
    return SID_ERROR_NONE;
  }

  Ecode_t res = ECODE_NVM3_OK;
  sid_error_t is_record_exist = SID_ERROR_NONE;
  uint32_t objectType = 0;
  size_t data_len = 0;
  size_t new_object_size = 0;
  size_t offset_in_group = 0;
  uint8_t * raw_file_buffer = NULL;
  struct storage_kv_record_header new_record_header = {
    .key       = key,
    .data_size = len
  };

  res = nvm3_getObjectInfo(kv_nvm3_handle, group, &objectType, &data_len);

  if (res == ECODE_NVM3_ERR_KEY_NOT_FOUND) {
    // First item in the group
    new_object_size = STORAGE_KV_REC_HDR_SIZE + len;

    // This part is needed because NVM3 does NOT support object append at the moment!
    raw_file_buffer = (uint8_t *)calloc(new_object_size, sizeof(uint8_t));
    if (raw_file_buffer == NULL) {
      res = ECODE_NVM3_ERR_INT_SIZE_ERROR;
    } else {
      // Copy the new entry header into the buffer
      memcpy(raw_file_buffer, &new_record_header, STORAGE_KV_REC_HDR_SIZE);
      // Copy the actual data after the header
      memcpy(&raw_file_buffer[STORAGE_KV_REC_HDR_SIZE], p_data, len);

      res = nvm3_writeData(kv_nvm3_handle, group, raw_file_buffer, new_object_size);

      free(raw_file_buffer);
    }
  } else if (res == ECODE_NVM3_OK) {
    if (data_len > 0) {
      is_record_exist = sid_pal_silabs_storage_kv_is_record_exist_in_group(group, key, &offset_in_group);

      if (is_record_exist == SID_ERROR_NONE) {
        sid_pal_storage_kv_record_delete(group, key);
        nvm3_getObjectInfo(kv_nvm3_handle, group, &objectType, &data_len);
      }
    }
    new_object_size = data_len + len + STORAGE_KV_REC_HDR_SIZE;
    raw_file_buffer = (uint8_t *)calloc(new_object_size, sizeof(uint8_t));

    if (data_len > 0) {
      res = nvm3_readData(kv_nvm3_handle, group, raw_file_buffer, data_len);
    }

    // Copy the new entry header into the buffer
    memcpy(&raw_file_buffer[data_len], &new_record_header, STORAGE_KV_REC_HDR_SIZE);
    // Copy the actual data after the header
    memcpy(&raw_file_buffer[data_len + STORAGE_KV_REC_HDR_SIZE], p_data, len);

    res = nvm3_writeData(kv_nvm3_handle, group, raw_file_buffer, new_object_size);

    if (res == ECODE_OK) {
      if (nvm3_repackNeeded(kv_nvm3_handle)) {
        res = nvm3_repack(kv_nvm3_handle);
      }
    }

    free(raw_file_buffer);
  }

  return nvm3_manager_nvm3_to_sid_error_code_translation(res);
}

sid_error_t sid_pal_storage_kv_group_delete(uint16_t group)
{
  Ecode_t res = ECODE_NVM3_OK;

  res = nvm3_deleteObject(kv_nvm3_handle, group);

  if (res == ECODE_NVM3_ERR_KEY_NOT_FOUND) {
    res = ECODE_NVM3_OK;
  }
  return nvm3_manager_nvm3_to_sid_error_code_translation(res);
}

sid_error_t sid_pal_storage_kv_record_delete(uint16_t group,
                                             uint16_t key)
{
  Ecode_t res = ECODE_NVM3_OK;
  struct storage_kv_record_header record_header;
  uint32_t objectType = 0;
  size_t data_len = 0;
  size_t offset_in_object = 0;
  uint8_t * raw_file_buffer = NULL;

  do {
    res = nvm3_getObjectInfo(kv_nvm3_handle, group, &objectType, &data_len);

    if ((res != ECODE_NVM3_OK) && (res != ECODE_NVM3_ERR_KEY_NOT_FOUND)) {
      break;
    }
    raw_file_buffer = (uint8_t *)calloc(data_len, sizeof(uint8_t));

    if (raw_file_buffer == NULL) {
      res = ECODE_NVM3_ERR_INT_SIZE_ERROR;
      break;
    }

    while (data_len > offset_in_object) {
      res = nvm3_readPartialData(kv_nvm3_handle, group, &record_header, offset_in_object, STORAGE_KV_REC_HDR_SIZE);

      if (res != ECODE_NVM3_OK) {
        break;
      }

      if (record_header.key == key) {
        // Record found
        res = nvm3_readPartialData(kv_nvm3_handle, group, raw_file_buffer, 0, offset_in_object);

        if (res != ECODE_NVM3_OK) {
          break;
        }
        res = nvm3_readPartialData(kv_nvm3_handle,
                                   group,
                                   &raw_file_buffer[offset_in_object],
                                   offset_in_object + record_header.data_size + STORAGE_KV_REC_HDR_SIZE,
                                   data_len - offset_in_object - record_header.data_size - STORAGE_KV_REC_HDR_SIZE);

        if (res != ECODE_NVM3_OK) {
          break;
        }
        res = nvm3_deleteObject(kv_nvm3_handle, group);

        if (res != ECODE_NVM3_OK) {
          break;
        }
        res = nvm3_writeData(kv_nvm3_handle,
                             group,
                             raw_file_buffer,
                             data_len - record_header.data_size - STORAGE_KV_REC_HDR_SIZE);

        if (res != ECODE_NVM3_OK) {
          break;
        }
        break;
      }
      offset_in_object += STORAGE_KV_REC_HDR_SIZE + record_header.data_size;
    }
    if (data_len <= offset_in_object) {
      res = ECODE_NVM3_ERR_KEY_NOT_FOUND;
    }
  } while (0);

  free(raw_file_buffer);

  return nvm3_manager_nvm3_to_sid_error_code_translation(res);
}
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static sid_error_t sid_pal_silabs_storage_kv_is_record_exist_in_group(uint16_t group,
                                                                      uint16_t key,
                                                                      size_t * record_offset)
{
  Ecode_t res = ECODE_NVM3_OK;
  struct storage_kv_record_header record_header;
  size_t offset_in_object = 0;
  uint32_t objectType;
  size_t data_len = 0;

  nvm3_getObjectInfo(kv_nvm3_handle, group, &objectType, &data_len);

  if (data_len > 0) {
    while (data_len > offset_in_object) {
      res = nvm3_readPartialData(kv_nvm3_handle, group, &record_header, offset_in_object, STORAGE_KV_REC_HDR_SIZE);

      if (res != ECODE_NVM3_OK) {
        break;
      }

      if (record_header.key == key) {
        // Record found
        *record_offset = offset_in_object;
        break;
      }
      offset_in_object += STORAGE_KV_REC_HDR_SIZE + record_header.data_size;
    }

    if (data_len <= offset_in_object) {
      res = ECODE_NVM3_ERR_KEY_NOT_FOUND;
    }
  } else {
    res = ECODE_NVM3_ERR_KEY_NOT_FOUND;
  }

  return nvm3_manager_nvm3_to_sid_error_code_translation(res);
}

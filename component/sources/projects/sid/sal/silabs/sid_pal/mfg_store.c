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
#include <nvm3_manager.h>
#include "em_system.h"
#include "assert.h"

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
static nvm3_Handle_t * mfg_nvm3_handle = NULL;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/*******************************************************************************
 *  Prepare the manufacturing store for use. Must be called before
 *  any of the other sid_pal_mfg_store functions.
 *
 * @param[in]  mfg_store_region Structure containing start and end addresses
 *                              of the manufacturing store.
 ******************************************************************************/
void sid_pal_mfg_store_init(sid_pal_mfg_store_region_t mfg_store_region)
{
  (void)mfg_store_region;
  size_t numberOfObjects;

  if (is_mfg_nvm3_initialized() == false) {
    nvm3_manager_init();
  }
  mfg_nvm3_handle = get_mfg_nvm3_handle();
  // Get the number of valid keys already in NVM3
  numberOfObjects = nvm3_countObjects(mfg_nvm3_handle);
  SID_PAL_LOG(SID_PAL_LOG_SEVERITY_INFO, "MFG Store opened with %d objects\n", numberOfObjects);
}

/*******************************************************************************
 * Write to mfg store.
 *
 *  @param[in]  value  Enum constant for the desired value. Use values from
 *                     sid_pal_mfg_store_value_t or application defined values
 *                     here.
 *  @param[in]  buffer Buffer containing the value to be stored.
 *  @param[in]  length Length of the value in bytes. Use values from
 *                     sid_pal_mfg_store_value_size_t here.
 *
 *  @return  0 on success, negative value on failure.
 ******************************************************************************/
int32_t sid_pal_mfg_store_write(uint16_t value,
                                const uint8_t * buffer,
                                uint16_t length)
{
  Ecode_t status = nvm3_writeData(mfg_nvm3_handle, value, buffer, (size_t)length);
  if (status != ECODE_NVM3_OK) {
    SID_PAL_LOG(SID_PAL_LOG_SEVERITY_ERROR, "MFG Store write error: %d\n", status);
    return -1;
  }

  /* Do repacking if needed */
  if (nvm3_repackNeeded(mfg_nvm3_handle)) {
    status = nvm3_repack(mfg_nvm3_handle);
    if (status != ECODE_NVM3_OK) {
      SID_PAL_LOG(SID_PAL_LOG_SEVERITY_ERROR, "MFG Store repack error: %d\n", status);
      return -1;
    }
  }

  return 0;
}

/*******************************************************************************
 *  Read from mfg store.
 *
 *  @param[in]  value  Enum constant for the desired value. Use values from
 *                     sid_pal_mfg_store_value_t or application defined values
 *                     here.
 *  @param[out] buffer Buffer to which the value will be copied.
 *  @param[in]  length Length of the value in bytes. Use values from
 *                     sid_pal_mfg_store_value_size_t here.
 *
 *
 ******************************************************************************/
void sid_pal_mfg_store_read(uint16_t value,
                            uint8_t * buffer,
                            uint16_t length)
{
  uint32_t objectType;
  size_t dataLen1;

  // Find size of data for object with key identifier 1 and 2 and read out
  nvm3_getObjectInfo(mfg_nvm3_handle, value, &objectType, &dataLen1);

  if (objectType == NVM3_OBJECTTYPE_DATA) {
    nvm3_readData(mfg_nvm3_handle, value, buffer, (size_t)length);
  }
}

/*******************************************************************************
 *  Get length of a tag ID.
 *
 *  @param[in]  value  Enum constant for the desired value. Use values from
 *                     sid_pal_mfg_store_value_t or application defined values
 *                     here.
 *
 *  @return  Length of the value in bytes for the tag that is requested on success,
 *           0 on failure (not found)
 ******************************************************************************/
uint16_t sid_pal_mfg_store_get_length_for_value(uint16_t value)
{
  uint32_t object_type;
  size_t length;

  /* Find size of data for object with key identifier */
  nvm3_getObjectInfo(mfg_nvm3_handle, value, &object_type, &length);

  return (object_type == NVM3_OBJECTTYPE_DATA) ? length : 0;
}

/*******************************************************************************
 *  Erase the manufacturing store.
 *  Because the manufacturing store is backed by flash memory, and flash memory
 *  can only be erased in large chunks (pages), this interface only supports
 *  erasing the entire manufacturing store.
 *
 *  @return  0 on success, negative value on failure.
 ******************************************************************************/
int32_t sid_pal_mfg_store_erase(void)
{
#if defined(HALO_ENABLE_DIAGNOSTICS) && HALO_ENABLE_DIAGNOSTICS
  Ecode_t status = 0;
  status = nvm3_eraseAll(mfg_nvm3_handle);
  return (int32_t) status;
#else
#if defined(SL_SID_DDP_FEATURE_ON) && SL_SID_DDP_FEATURE_ON
  return 0;
#else
  return -1;
#endif /* SL_SID_DDP_FEATURE_ON */
#endif /* HALO_ENABLE_DIAGNOSTICS */
}

/*******************************************************************************
 *  Check if the manufacturing store supports TLV based storage.
 *
 *  @return   true if the manufacturing store supports TLV based storage.
 ******************************************************************************/
bool sid_pal_mfg_store_is_tlv_support(void)
{
  return true;
}

/*******************************************************************************
 *  Get version of values stored in mfg store.
 *  The version of the mfg values is stored along with all the values
 *  in mfg store. This API retrieves the value by reading the
 *  address at which the version is stored.
 *
 *  @return   version of mfg store.
 ******************************************************************************/
uint32_t sid_pal_mfg_store_get_version(void)
{
  uint32_t version;

  sid_pal_mfg_store_read(SID_PAL_MFG_STORE_VERSION,
                         (uint8_t *)&version, SID_PAL_MFG_STORE_VERSION_SIZE);
  // Assuming that we keep this behavior for both 1P & 3P
  return SLI_NTOHL(version);
}

/*******************************************************************************
 *  Get the device ID from the mfg store.
 *
 *  @param[out] dev_id The device ID
 *
 *  @return true if the device ID could be found
 ******************************************************************************/
bool sid_pal_mfg_store_dev_id_get(uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE])
{
  bool error_code = false;
  uint8_t buffer[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  _Static_assert(sizeof(buffer) == SID_PAL_MFG_STORE_DEVID_SIZE, "dev ID buffer wrong size");

  sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DEVID,
                         buffer, SID_PAL_MFG_STORE_DEVID_SIZE);

  static const uint8_t UNSET_DEV_ID[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  _Static_assert(sizeof(UNSET_DEV_ID) == SID_PAL_MFG_STORE_DEVID_SIZE, "Unset dev ID wrong size");

  if (memcmp(buffer, UNSET_DEV_ID, SID_PAL_MFG_STORE_DEVID_SIZE) == 0) {
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

/*******************************************************************************
 *  Get the device serial number from the mfg store.
 *
 *  @param[out] serial_num The device serial number
 *
 *  @return true if the device serial number could be found
 ******************************************************************************/
bool sid_pal_mfg_store_serial_num_get(uint8_t serial_num[SID_PAL_MFG_STORE_SERIAL_NUM_SIZE])
{
  uint32_t buffer[(SID_PAL_MFG_STORE_SERIAL_NUM_SIZE + (MFG_WORD_SIZE - 1)) / MFG_WORD_SIZE];

  sid_pal_mfg_store_read(SID_PAL_MFG_STORE_SERIAL_NUM,
                         (uint8_t *)buffer, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);

  static const uint8_t UNSET_SERIAL_NUM[] =
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };
  _Static_assert(sizeof(UNSET_SERIAL_NUM) == SID_PAL_MFG_STORE_SERIAL_NUM_SIZE, "Unset serial num wrong size");

  if (memcmp(buffer, UNSET_SERIAL_NUM, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE) == 0) {
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

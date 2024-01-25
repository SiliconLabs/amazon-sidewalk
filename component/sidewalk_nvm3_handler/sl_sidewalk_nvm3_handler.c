/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_nvm3_handler.c
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "sl_sidewalk_nvm3_handler.h"
#include "sl_sidewalk_nvm3_handler_config.h"
#include "nvm3_generic.h"
#include "nvm3_hal_flash.h"
#include "nvm3_default.h"
#include "nvm3_default_config.h"
#include "app_log.h"
#include "nvm3_manager.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define RETURN_ERR_IF_KEY_NOT_IN_RANGE(key)                                 \
  do {                                                                      \
    if (key < SLI_SID_NVM3_KEY_MIN_APP || key > SLI_SID_NVM3_KEY_MAX_APP) { \
      return SL_SIDEWALK_NVM3_INVALID_KEY_SPACE_REGION;                     \
    }                                                                       \
  } while (0)                                                               \

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Initializes the nvm3 part to be able to used by NVM3 storage
 ******************************************************************************/
void sl_sidewalk_nvm3_handler_init(void)
{
  // Open nvm3 part for read/write
  if (nvm3_open(nvm3_defaultHandle, nvm3_defaultInit) != ECODE_NVM3_OK) {
    app_log_warning("NVM3 init failed");
  }
}

/*******************************************************************************
 * Write NVM3 object
 *
 *  @param[in]  value   A 20-bit object identifier
 *  @param[in]  buffer  Buffer containing the value to be stored
 *  @param[in]  length  Length of the value in bytes
 *
 *  @return 0 on success, negative value on failure
 ******************************************************************************/
int32_t sl_sidewalk_nvm3_write(uint32_t value,
                               const uint8_t *buffer,
                               uint16_t length)
{
  RETURN_ERR_IF_KEY_NOT_IN_RANGE(value);

  Ecode_t status = nvm3_writeData(nvm3_defaultHandle, value, buffer, (size_t)length);
  if (status != ECODE_NVM3_OK) {
    return SL_SIDEWALK_NVM3_DATA_WRITE_FAILED;
  }

  // Do repacking if needed
  if (nvm3_repackNeeded(nvm3_defaultHandle)) {
    status = nvm3_repack(nvm3_defaultHandle);
    if (status != ECODE_NVM3_OK) {
      return SL_SIDEWALK_NVM3_DATA_REPACK_FAILED;
    }
  }

  return 0;
}

/*******************************************************************************
 *  Read NVM3 object
 *
 *  @param[in]  value  A 20-bit object identifier
 *  @param[out] buffer Buffer to which the value will be copied
 *
 *  @return 0 on success, other value on failure
 ******************************************************************************/
int32_t sl_sidewalk_nvm3_read(uint32_t value,
                              uint8_t *buffer)
{
  int32_t retVal = 0;
  uint32_t object_type;
  size_t dataLen;

  RETURN_ERR_IF_KEY_NOT_IN_RANGE(value);

  // Find size of data for object with key identifier and read out
  Ecode_t ret = nvm3_getObjectInfo(nvm3_defaultHandle, value, &object_type, &dataLen);

  if ((ret == ECODE_NVM3_OK) && (object_type == NVM3_OBJECTTYPE_DATA)) {
    if (nvm3_readData(nvm3_defaultHandle, value, buffer, dataLen) != ECODE_NVM3_OK) {
      retVal = SL_SIDEWALK_NVM3_DATA_READ_FAILED;
    }
  } else {
    retVal = SL_SIDEWALK_NVM3_GET_OBJECT_INFO_FAILED;
  }

  return retVal;
}

/*******************************************************************************
 *  Delete an object in NVM3
 *
 *  @param[in]  value  A 20-bit object identifier
 *
 *  @return 0 on success, negative value on failure
 ******************************************************************************/
int32_t sl_sidewalk_nvm3_delete_object(uint32_t value)
{
  RETURN_ERR_IF_KEY_NOT_IN_RANGE(value);

  return (int32_t)nvm3_deleteObject(nvm3_defaultHandle, value);
}

/*******************************************************************************
 *  Get the number of valid keys already in NVM3
 *  @return Number of valid objects in NVM3
 ******************************************************************************/
uint32_t sl_sidewalk_nvm3_get_valid_object_number(void)
{
  return (uint32_t)nvm3_countObjects(nvm3_defaultHandle);
}

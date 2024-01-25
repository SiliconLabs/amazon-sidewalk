/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_nvm3_handler.h
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

#ifndef SL_SIDEWALK_NVM3_HANDLER_H
#define SL_SIDEWALK_NVM3_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "nvm3_generic.h"
#include "nvm3_manager.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
enum sl_sidewalk_nvm3_hdl_error_status {
  SL_SIDEWALK_NVM3_SUCCESS = 0,
  SL_SIDEWALK_NVM3_DATA_READ_FAILED = -1,
  SL_SIDEWALK_NVM3_DATA_WRITE_FAILED = -2,
  SL_SIDEWALK_NVM3_DATA_REPACK_FAILED = -3,
  SL_SIDEWALK_NVM3_GET_OBJECT_INFO_FAILED = -4,
  SL_SIDEWALK_NVM3_INVALID_KEY_SPACE_REGION = -5
};

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Initializes the nvm3 part to be able to used by NVM3 storage
 ******************************************************************************/
void sl_sidewalk_nvm3_handler_init(void);

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
                               uint16_t length);

/*******************************************************************************
 *  Read NVM3 object
 *
 *  @param[in]  value  A 20-bit object identifier
 *  @param[out] buffer Buffer to which the value will be copied
 *
 *  @return 0 on success, other value on failure
 ******************************************************************************/
int32_t sl_sidewalk_nvm3_read(uint32_t value,
                              uint8_t *buffer);

/*******************************************************************************
 *  Delete an object in NVM3
 *
 *  @param[in]  value  A 20-bit object identifier
 *
 *  @return 0 on success, negative value on failure
 ******************************************************************************/
int32_t sl_sidewalk_nvm3_delete_object(uint32_t value);

/*******************************************************************************
 *  Get the number of valid keys already in NVM3
 *  @return Number of valid objects in NVM3
 ******************************************************************************/
uint32_t sl_sidewalk_nvm3_get_valid_object_number(void);

#ifdef __cplusplus
}
#endif

#endif // SL_SIDEWALK_NVM3_HANDLER_H

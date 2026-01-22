/***************************************************************************//**
 * @file
 * @brief nvm3_manager.c
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

#include "nvm3_manager.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

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
//                          Static Function Definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

sid_error_t sli_sid_nvm3_convert_ecode_to_sid_error(Ecode_t nvm3_return_code)
{
  sid_error_t sid_error_code;

  switch (nvm3_return_code) {
    case ECODE_NVM3_ERR_KEY_NOT_FOUND:
      sid_error_code = SID_ERROR_NOT_FOUND;
      break;

    case ECODE_NVM3_ERR_READ_DATA_SIZE:
    case ECODE_NVM3_ERR_WRITE_DATA_SIZE:
      sid_error_code = SID_ERROR_INCOMPATIBLE_PARAMS;
      break;

    case ECODE_NVM3_ERR_STORAGE_FULL:
      sid_error_code = SID_ERROR_STORAGE_FULL;
      break;

    case ECODE_NVM3_ERR_ERASE_FAILED:
      sid_error_code = SID_ERROR_STORAGE_ERASE_FAIL;
      break;

    case ECODE_NVM3_ERR_WRITE_FAILED:
      sid_error_code = SID_ERROR_STORAGE_WRITE_FAIL;
      break;

    case ECODE_NVM3_ERR_READ_FAILED:
      sid_error_code = SID_ERROR_STORAGE_READ_FAIL;
      break;

    case ECODE_OK:
      sid_error_code = SID_ERROR_NONE;
      break;

    default:
      sid_error_code = SID_ERROR_GENERIC;
      break;
  }

  return sid_error_code;
}

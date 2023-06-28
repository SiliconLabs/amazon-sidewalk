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
#include <sid_pal_log_ifc.h>
#include <sid_pal_assert_ifc.h>
#include <nvm3_manager.h>
#include "nvm3_default.h"
#include "nvm3_default_config.h"
#include "assert.h"
#include "nvm3_hal_flash.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define MFG_NUM_FLASH_PAGE                  (3)
#define MFG_NVM3_CACHE_SIZE                 (50UL)
#define MFG_NVM3_EXT_NVM_BASE_ADDRESS       (0)
#define MFG_NVM3_DEFAULT_SIZE               (FLASH_PAGE_SIZE)*(MFG_NUM_FLASH_PAGE)
#define MFG_NVM3_MAX_DELETE_CNT             (10)
#define MFG_NVM3_MAX_OBJECT_SIZE            NVM3_MAX_OBJECT_SIZE_DEFAULT
#define MFG_NVM3_DEFAULT_REPACK_HEADROOM    (0)

#define KV_NUM_FLASH_PAGE                   (3)
#define KV_NVM3_CACHE_SIZE                  (200UL)
#define KV_NVM3_EXT_NVM_BASE_ADDRESS        (0)
#define KV_NVM3_DEFAULT_SIZE                (FLASH_PAGE_SIZE)*(MFG_NUM_FLASH_PAGE)
#define KV_NVM3_MAX_DELETE_CNT              (10)
#define KV_NVM3_MAX_OBJECT_SIZE             NVM3_MAX_OBJECT_SIZE_DEFAULT
#define KV_NVM3_DEFAULT_REPACK_HEADROOM     (0)

#if defined(__ICCARM__)
__root uint8_t mfg_nvm3_default_storage[MFG_NVM3_DEFAULT_SIZE] @"SIMEE";
__root uint8_t kv_nvm3_default_storage[KV_NVM3_DEFAULT_SIZE] @"SIMEE";
    #define NVM3_BASE    (mfg_nvm3_default_storage)
    #define NVM3_SIZE    ((uint32_t)&mfg_nvm3_default_storage)
#elif defined(__GNUC__)
extern char linker_nvm_begin;
__attribute__((used)) uint8_t mfg_nvm3_default_storage[MFG_NVM3_DEFAULT_SIZE] __attribute__((section(".simee")));
__attribute__((used)) uint8_t kv_nvm3_default_storage[KV_NVM3_DEFAULT_SIZE] __attribute__((section(".simee")));
    #define NVM3_BASE    ( &linker_nvm_begin)
    #define NVM3_SIZE    ((uint32_t) &linker_nvm_size)
#else // if defined( __ICCARM__ )
    #error "Unsupported toolchain"
#endif  // if defined( __ICCARM__ )

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Decode the return value of the nvm3 commands
 * @param sta return value of a function
 * @return String of readable message
 ******************************************************************************/
static char * cmd_nvm3_ecode_to_str(Ecode_t sta);
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
static nvm3_Handle_t mfg_nvm3_handleData;
static nvm3_Handle_t * mfg_nvm3_handle = &mfg_nvm3_handleData;

static nvm3_Handle_t kv_nvm3_handleData;
static nvm3_Handle_t * kv_nvm3_handle = &kv_nvm3_handleData;

static nvm3_CacheEntry_t mfg_defaultCache[MFG_NVM3_CACHE_SIZE];
static nvm3_CacheEntry_t kv_defaultCache[KV_NVM3_CACHE_SIZE];

static nvm3_Init_t mfg_nvm3_InitData =
{
  (nvm3_HalPtr_t)(NVM3_BASE + NVM3_DEFAULT_NVM_SIZE),
  MFG_NVM3_DEFAULT_SIZE,
  mfg_defaultCache,
  MFG_NVM3_CACHE_SIZE,
  MFG_NVM3_MAX_OBJECT_SIZE,
  MFG_NVM3_DEFAULT_REPACK_HEADROOM,
  &nvm3_halFlashHandle,
};

static nvm3_Init_t kv_nvm3_InitData =
{
  (nvm3_HalPtr_t)(NVM3_BASE + NVM3_DEFAULT_NVM_SIZE + MFG_NVM3_DEFAULT_SIZE),
  KV_NVM3_DEFAULT_SIZE,
  kv_defaultCache,
  KV_NVM3_CACHE_SIZE,
  KV_NVM3_MAX_OBJECT_SIZE,
  KV_NVM3_DEFAULT_REPACK_HEADROOM,
  &nvm3_halFlashHandle,
};

static nvm3_Init_t * mfg_nvm3_Init = &mfg_nvm3_InitData;
static nvm3_Init_t * kv_nvm3_Init = &kv_nvm3_InitData;

static bool mfg_nvm3_init = false;
static bool kv_nvm3_init = false;
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Initializes the nvm3 part to be able to used by MFG and KV Storage
 ******************************************************************************/
void nvm3_manager_init(void)
{
  Ecode_t sta;

  // Open both nvm3 part for read write
  sta = nvm3_open(mfg_nvm3_handle, mfg_nvm3_Init);
  SID_PAL_LOG(SID_PAL_LOG_SEVERITY_INFO, "MFG NVM3 start info:\n %s\n", cmd_nvm3_ecode_to_str(sta));

  if (sta == ECODE_NVM3_OK) {
    mfg_nvm3_init = true;
  }
  sta = nvm3_open(kv_nvm3_handle, kv_nvm3_Init);
  SID_PAL_LOG(SID_PAL_LOG_SEVERITY_INFO, "KV NVM3 start info:\n %s\n", cmd_nvm3_ecode_to_str(sta));

  if (sta == ECODE_NVM3_OK) {
    kv_nvm3_init = true;
  }
}

/*******************************************************************************
 * Get the nvm3 handle for the MFG Store
 * @return pointer to the MFG nvm3 handle
 ******************************************************************************/
nvm3_Handle_t * get_mfg_nvm3_handle(void)
{
  return &mfg_nvm3_handleData;
}

/*******************************************************************************
 * Check if the init was successful
 * @return true is init was successful
 ******************************************************************************/
bool is_mfg_nvm3_initialized(void)
{
  return mfg_nvm3_init;
}

/*******************************************************************************
 * Get the nvm3 handle for the KV Store
 * @return pointer to the KV nvm3 handle
 ******************************************************************************/
nvm3_Handle_t * get_kv_nvm3_handle(void)
{
  return &kv_nvm3_handleData;
}

/*******************************************************************************
 * Check if the init was successful
 * @return true is init was successful
 ******************************************************************************/
bool is_kv_nvm3_initialized(void)
{
  return kv_nvm3_init;
}

// ----------------------------------------------------------------------------- */
//                          Static Function Definitions */
// ----------------------------------------------------------------------------- */

/*******************************************************************************
 * Decode the return value of the nvm3 commands
 * @param sta return value of a function
 * @return String of readable message
 ******************************************************************************/
char * cmd_nvm3_ecode_to_str(Ecode_t sta)
{
  switch (sta) {
    case ECODE_NVM3_OK:
      return "OK";

      break;

    case ECODE_NVM3_ERR_ALIGNMENT_INVALID:
      return "Invalid data alignment";

      break;

    case ECODE_NVM3_ERR_SIZE_TOO_SMALL:
      return "Not enough NVM memory specified";

      break;

    case ECODE_NVM3_ERR_NO_VALID_PAGES:
      return "No valid pages found";

      break;

    case ECODE_NVM3_ERR_PAGE_SIZE_NOT_SUPPORTED:
      return "The page size is not supported";

      break;

    case ECODE_NVM3_ERR_OBJECT_SIZE_NOT_SUPPORTED:
      return "The object size is not supported";

      break;

    case ECODE_NVM3_ERR_STORAGE_FULL:
      return "No more space available";

      break;

    case ECODE_NVM3_ERR_NOT_OPENED:
      return "The module has not been sucessfully opened";

      break;

    case ECODE_NVM3_ERR_OPENED_WITH_OTHER_PARAMETERS:
      return "NVM3 is already opened with other parameters";

      break;

    case ECODE_NVM3_ERR_PARAMETER:
      return "Error in parameter";

      break;

    case ECODE_NVM3_ERR_KEY_INVALID:
      return "Invalid key";

      break;

    case ECODE_NVM3_ERR_KEY_NOT_FOUND:
      return "Key not found";

      break;

    case ECODE_NVM3_ERR_OBJECT_IS_NOT_DATA:
      return "The object is not data";

      break;

    case ECODE_NVM3_ERR_OBJECT_IS_NOT_A_COUNTER:
      return "The object is not a counter";

      break;

    case ECODE_NVM3_ERR_ERASE_FAILED:
      return "Erase failed";

      break;

    case ECODE_NVM3_ERR_WRITE_DATA_SIZE:
      return "The object is too large";

      break;

    case ECODE_NVM3_ERR_WRITE_FAILED:
      return "Error in the write operation";

      break;

    case ECODE_NVM3_ERR_READ_DATA_SIZE:
      return "Read length mismatch";

      break;

    case ECODE_NVM3_ERR_INIT_WITH_FULL_NVM:
      return "Open with a full NVM";

      break;

    case ECODE_NVM3_ERR_RESIZE_PARAMETER:
      return "Resize parameter error";

      break;

    case ECODE_NVM3_ERR_RESIZE_NOT_ENOUGH_SPACE:
      return "Not enough space to resize";

      break;

    case ECODE_NVM3_ERR_INT_WRITE_TO_NOT_ERASED:
      return "Internal Error: Write to memory that is not erased";

      break;

    case ECODE_NVM3_ERR_INT_ADDR_INVALID:
      return "Internal Error: Accessing invalid memory";

      break;

    case ECODE_NVM3_ERR_INT_KEY_MISMATCH:
      return "Internal Error: Key validaton failure";

      break;

    case ECODE_NVM3_ERR_INT_SIZE_ERROR:
      return "Internal Error: Size mismatch error";

      break;

    case ECODE_NVM3_ERR_INT_EMULATOR:
      return "Internal Error: Emulator error";

      break;

    case ECODE_NVM3_ERR_INT_TEST:
      return "Internal Error: Test error";

      break;

    default:
      return "NVM3 unknown error: 0x%x.";

      break;
  }
}

sid_error_t nvm3_manager_nvm3_to_sid_error_code_translation(Ecode_t nvm3_return_code)
{
  sid_error_t sid_error_code;

  switch (nvm3_return_code) {
    case ECODE_NVM3_ERR_KEY_NOT_FOUND:
      sid_error_code = SID_ERROR_NOT_FOUND;
      break;

    case ECODE_NVM3_ERR_READ_DATA_SIZE:
      sid_error_code = SID_ERROR_INCOMPATIBLE_PARAMS;
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

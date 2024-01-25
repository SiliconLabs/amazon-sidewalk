/***************************************************************************//**
 * @file
 * @brief nvm3_manager.h
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

#ifndef NVM3_MANAGER_H
#define NVM3_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "nvm3.h"
#include "sid_error.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -- DO NOT MODIFY START --

#define SLI_SID_NVM3_KEY_BASE         0xA0000 // reserved for sidewalk in gsdk

#define SLI_SID_NVM3_KEY_MIN_APP_REL  0x0
#define SLI_SID_NVM3_KEY_MAX_APP_REL  0x1FFF
#define SLI_SID_NVM3_KEY_MIN_KV_REL   0x0     // defined in sid_pal_storage_kv_internal_group_ids.h
#define SLI_SID_NVM3_KEY_MAX_KV_REL   0x6FFF  // defined in sid_pal_storage_kv_internal_group_ids.h
#define SLI_SID_NVM3_KEY_MIN_MFG_REL  0x0     // defined in sid_pal_mfg_store_ifc.h
#define SLI_SID_NVM3_KEY_MAX_MFG_REL  0x6FFF  // defined in sid_pal_mfg_store_ifc.h

#define SLI_SID_NVM3_KEY_MIN_APP      (SLI_SID_NVM3_KEY_BASE + SLI_SID_NVM3_KEY_MIN_APP_REL)        // 0xA0000 - 0xA1FFF
#define SLI_SID_NVM3_KEY_MAX_APP      (SLI_SID_NVM3_KEY_BASE + SLI_SID_NVM3_KEY_MAX_APP_REL)
#define SLI_SID_NVM3_KEY_MIN_KV       (SLI_SID_NVM3_KEY_MAX_APP + 1)                                // 0xA2000 - 0xA8FFF
#define SLI_SID_NVM3_KEY_MAX_KV       (SLI_SID_NVM3_KEY_MAX_APP + 1 + SLI_SID_NVM3_KEY_MAX_KV_REL)
#define SLI_SID_NVM3_KEY_MIN_MFG      (SLI_SID_NVM3_KEY_MAX_KV + 1)
#define SLI_SID_NVM3_KEY_MAX_MFG      (SLI_SID_NVM3_KEY_MAX_KV + 1 + SLI_SID_NVM3_KEY_MAX_MFG_REL)  // 0xA9000 - 0xAFFFF

#define SLI_SID_NVM3_KEY_BASE_APP     SLI_SID_NVM3_KEY_MIN_APP
#define SLI_SID_NVM3_KEY_BASE_KV      SLI_SID_NVM3_KEY_MIN_KV
#define SLI_SID_NVM3_KEY_BASE_MFG     SLI_SID_NVM3_KEY_MIN_MFG

// -- DO NOT MODIFY END --

#define SLI_SID_NVM3_VALIDATE_KEY(region, key)    ((uint32_t)key <= SLI_SID_NVM3_KEY_MAX_##region##_REL)
#define SLI_SID_NVM3_MAP_KEY(region, key)         (SLI_SID_NVM3_KEY_BASE_##region + (uint32_t)key)

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Translates Ecode_t type error codes to sid_error_t type
 * @param[in] Ecode_t type error code from nvm3
 * @return translated error code to sid_error_t
 ******************************************************************************/
sid_error_t sli_sid_nvm3_convert_ecode_to_sid_error(Ecode_t nvm3_return_code);

#ifdef __cplusplus
}
#endif

#endif /* NVM3_MANAGER_H */

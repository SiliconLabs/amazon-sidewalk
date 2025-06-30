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

/**
 * \addtogroup sid_pal
 * @{
 */
/**
 * \addtogroup sid_pal_nvm3_mngr
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "nvm3.h"
#include "sid_error.h"
#include "sid_pal_mfg_store_ifc.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @addtogroup sid_pal_nvm3_mngr_types Type definitions
 * @ingroup sid_pal_nvm3_mngr
 * @{
 *****************************************************************************/

// -- DO NOT MODIFY START --

#define SLI_SID_NVM3_KEY_BASE         0xA0000 /*!< reserved for sidewalk in gsdk */

#define SLI_SID_NVM3_KEY_MIN_APP_REL  0x0     /*!< Minimum relative key value for application-specific NVM3 keys. */
#define SLI_SID_NVM3_KEY_MAX_APP_REL  0x1FFF  /*!< Maximum relative key value for application-specific NVM3 keys. */
#define SLI_SID_NVM3_KEY_MIN_KV_REL   0x0     /*!< defined in @ref sid_pal_kv_ifc */
#define SLI_SID_NVM3_KEY_MAX_KV_REL   0x6FFF  /*!< defined in @ref sid_pal_kv_ifc */
#define SLI_SID_NVM3_KEY_MIN_MFG_REL  0x0     /*!< defined in @ref sid_pal_mfg_ifc */
#define SLI_SID_NVM3_KEY_MAX_MFG_REL  0x6FFF  /*!< defined in @ref sid_pal_mfg_ifc */

#define SLI_SID_NVM3_KEY_MIN_APP      (SLI_SID_NVM3_KEY_BASE + SLI_SID_NVM3_KEY_MIN_APP_REL)        /*!< Minimum key value for application-specific NVM3 keys. Range: 0xA0000 - 0xA1FFF */
#define SLI_SID_NVM3_KEY_MAX_APP      (SLI_SID_NVM3_KEY_BASE + SLI_SID_NVM3_KEY_MAX_APP_REL)        /*!< Maximum key value for application-specific NVM3 keys. */
#define SLI_SID_NVM3_KEY_MIN_KV       (SLI_SID_NVM3_KEY_MAX_APP + 1)                                /*!< Minimum key value for key-value storage NVM3 keys. Range: 0xA2000 - 0xA8FFF */
#define SLI_SID_NVM3_KEY_MAX_KV       (SLI_SID_NVM3_KEY_MAX_APP + 1 + SLI_SID_NVM3_KEY_MAX_KV_REL)  /*!< Maximum key value for key-value storage NVM3 keys. */
#define SLI_SID_NVM3_KEY_MIN_MFG      (SLI_SID_NVM3_KEY_MAX_KV + 1)                                 /*!< Minimum key value for manufacturer-specific NVM3 keys. Range: 0xA9000 - 0xAFFFF */
#define SLI_SID_NVM3_KEY_MAX_MFG      (SLI_SID_NVM3_KEY_MAX_KV + 1 + SLI_SID_NVM3_KEY_MAX_MFG_REL)  /*!< Maximum key value for manufacturer-specific NVM3 keys. */

#define SLI_SID_NVM3_KEY_BASE_APP     SLI_SID_NVM3_KEY_MIN_APP      /*!< Base key value for application-specific NVM3 keys. */
#define SLI_SID_NVM3_KEY_BASE_KV      SLI_SID_NVM3_KEY_MIN_KV       /*!< Base key value for key-value storage NVM3 keys. */
#define SLI_SID_NVM3_KEY_BASE_MFG     SLI_SID_NVM3_KEY_MIN_MFG      /*!< Base key value for manufacturer-specific NVM3 keys. */

/**
 * @brief SiLabs-specific MFG key for the NVM3 version MFG object.
 * The range of application-specific keys starts after SID_PAL_MFG_STORE_CORE_VALUE_MAX (= 4000) according to
 * @ref sid_pal_mfg_ifc, this is simply the first freely usable key (= 4001).
 */
#define SID_PAL_MFG_STORE_SL_NVM3_VERSION (SID_PAL_MFG_STORE_CORE_VALUE_MAX + 1)

/**
 * @brief Size of the NVM3 version in bytes.
 */
#define SID_PAL_MFG_STORE_SL_NVM3_VERSION_SIZE  4

// -- DO NOT MODIFY END --

/**
 * @brief Validate if a key is within the specified region's range.
 *
 * This macro checks if the given key is within the valid range for the specified region.
 *
 * @param[in] region The region to validate the key against.
 * @param[in] key The key to be validated.
 *
 * @return True if the key is within the valid range, false otherwise.
 */
#define SLI_SID_NVM3_VALIDATE_KEY(region, key)    ((uint32_t)key <= SLI_SID_NVM3_KEY_MAX_##region##_REL)
/**
 * @brief Map a key to the specified region's base key.
 *
 * This macro maps the given key to the base key of the specified region.
 *
 * @param[in] region The region to map the key to.
 * @param[in] key The key to be mapped.
 *
 * @return The mapped key value.
 */
#define SLI_SID_NVM3_MAP_KEY(region, key)         (SLI_SID_NVM3_KEY_BASE_##region + (uint32_t)key)

/** @} (end sid_pal_nvm3_mngr_types) */

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**
 * @brief Translates Ecode_t type error codes to sid_error_t type
 * @param[in] nvm3_return_code type error code from nvm3
 * @return translated error code to sid_error_t
 */
sid_error_t sli_sid_nvm3_convert_ecode_to_sid_error(Ecode_t nvm3_return_code);

#ifdef __cplusplus
}
#endif

#endif /* NVM3_MANAGER_H */

/** @} */ // end of sid_pal group
/** @} */ // end of sid_pal_nvm3_mngr group
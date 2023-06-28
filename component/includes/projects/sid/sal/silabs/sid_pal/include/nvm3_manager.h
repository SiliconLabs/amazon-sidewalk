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

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Initializes the nvm3 part to be able to used by MFG and KV Storage
 ******************************************************************************/
void nvm3_manager_init(void);

/*******************************************************************************
 * Get the nvm3 handle for the MFG Store
 * @return pointer to the MFG nvm3 handle
 ******************************************************************************/
nvm3_Handle_t * get_mfg_nvm3_handle(void);

/*******************************************************************************
 * Get the nvm3 handle for the KV Store
 * @return pointer to the KV nvm3 handle
 ******************************************************************************/
nvm3_Handle_t * get_kv_nvm3_handle(void);

/*******************************************************************************
 * Check if the init was successful
 * @return true is init was successful
 ******************************************************************************/
bool is_mfg_nvm3_initialized(void);

/*******************************************************************************
 * Check if the init was successful
 * @return true is init was successful
 ******************************************************************************/
bool is_kv_nvm3_initialized(void);

/*******************************************************************************
 * Translates Ecode_t type error codes from nvm3 to sid_error_t type
 * @param[in] Ecode_t type error code from nvm3
 * @return translated error code to sid_error_t
 ******************************************************************************/
sid_error_t nvm3_manager_nvm3_to_sid_error_code_translation(Ecode_t nvm3_return_code);

#ifdef __cplusplus
}
#endif

#endif /* NVM3_MANAGER_H */

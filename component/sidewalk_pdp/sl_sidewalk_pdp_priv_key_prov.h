/***************************************************************************//**
 * @file sl_sidewalk_pdp_priv_key_prov.h
 * @brief
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

#ifndef SL_SIDEWALK_PDP_PRIV_KEY_PROV_H
#define SL_SIDEWALK_PDP_PRIV_KEY_PROV_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include "sl_common.h"
#include "sl_sidewalk_pdp_common.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define SL_SID_PDP_PRIV_KEY_PROV_MIN_WRITE_NVM3_REQ_LEN (7)
#define SL_SID_PDP_PRIV_KEY_PROV_MIN_INJECT_KEY_REQ_LEN (30)

SL_PACK_START(1)
typedef struct {
  uint32_t key;
  uint16_t data_len;
  const uint8_t *data;
} SL_ATTRIBUTE_PACKED sl_sid_pdp_priv_key_prov_write_nvm3_req_t;
SL_PACK_END()

SL_PACK_START(1)
typedef struct {
  uint32_t lifetime;
  uint32_t location;
  uint32_t usage_flags;
  uint32_t bits;
  uint32_t algo;
  uint8_t type;
} SL_ATTRIBUTE_PACKED psa_key_attr_t;
SL_PACK_END()

SL_PACK_START(1)
typedef struct {
  psa_key_attr_t key_attr;
  uint32_t key_id;
  uint32_t key_len;
  const uint8_t *key;
} SL_ATTRIBUTE_PACKED sl_sid_pdp_priv_key_prov_inject_key_req_t;
SL_PACK_END()

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief Writes data to a given NVM3 instance.
 *
 * @warning
 *   `in` buffer should not be overwritten because request data is not copied
 *   into another buffer but kept in `in` buffer for memory optimisation.
 *
 * @warning
 *   Response is not required for this request.
 *
 * @param[in] in Request
 * @param[in] in_len Length of the request
 * @param[out] out Response buffer
 * @param[out] out_size Size of the response buffer
 * @param[out] out_len Length of the response if any
 *
 * @return Status code
 * @retval SL_SID_PDP_STATUS_ERR_IN_ARGS_NOT_VALID One or more input arguments are not valid
 * @retval SL_SID_PDP_STATUS_ERR_NVM3_OPEN NVM3 open failed
 * @retval SL_SID_PDP_STATUS_ERR_NVM3_WRITE NVM3 write failed
 * @retval SL_SID_PDP_STATUS_ERR_NVM3_REPACK NVM3 repack failed
 * @retval SL_SID_PDP_STATUS_ERR_NVM3_CLOSE NVM3 close failed
 * @retval SL_SID_PDP_STATUS_SUCCESS Success
 ******************************************************************************/
sl_sid_pdp_status_t sl_sid_pdp_priv_key_prov_write_nvm3(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len);

/***************************************************************************//**
 * @brief Injects a key into the secure vault.
 *
 * @warning
 *   `in` buffer should not be overwritten because request data is not copied
 *   into another buffer but kept in `in` buffer for memory optimisation.
 *
 * @warning
 *   Response is not required for this request.
 *
 * @param[in] in Request
 * @param[in] in_len Length of the request
 * @param[out] out Response buffer
 * @param[out] out_size Size of the response buffer
 * @param[out] out_len Length of the response if any
 *
 * @return Status code
 * @retval SL_SID_PDP_STATUS_ERR_IN_ARGS_NOT_VALID One or more input arguments are not valid
 * @retval SL_SID_PDP_STATUS_ERR_PSA_CRYPTO_INIT PSA crypto initialize failed
 * @retval SL_SID_PDP_STATUS_ERR_PSA_IMPORT_KEY Import key into PSA crypto failed
 * @retval SL_SID_PDP_STATUS_ERR_PSA_SIGN_MESSAGE Message signing failed
 * @retval SL_SID_PDP_STATUS_SUCCESS Success
 ******************************************************************************/
sl_sid_pdp_status_t sl_sid_pdp_priv_key_prov_inject_key(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len);

#endif // SL_SIDEWALK_PDP_PRIV_KEY_PROV_H

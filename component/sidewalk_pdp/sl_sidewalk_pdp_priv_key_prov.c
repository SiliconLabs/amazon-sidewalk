/***************************************************************************//**
 * @file sl_sidewalk_pdp_priv_key_proc.c
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#ifdef DEBUG_PDP
#include <stdio.h>
#endif /* DEBUG_PDP */
#include <string.h>
#include "nvm3.h"
#include "nvm3_hal_flash.h"
#include "sl_sidewalk_pdp_priv_key_prov.h"
#include "psa/crypto.h"

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

sl_sid_pdp_status_t sl_sid_pdp_priv_key_prov_write_nvm3(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len)
{
  (void)out;
  (void)out_len;
  (void)out_size;

  if (in == NULL || in_len < SL_SID_PDP_PRIV_KEY_PROV_MIN_WRITE_NVM3_REQ_LEN) {
    return SL_SID_PDP_STATUS_ERR_IN_ARGS_NOT_VALID;
  }

  Ecode_t ret;

  sl_sid_pdp_priv_key_prov_write_nvm3_req_t req = *(sl_sid_pdp_priv_key_prov_write_nvm3_req_t *)in;
  req.data = &in[(SL_SID_PDP_PRIV_KEY_PROV_MIN_WRITE_NVM3_REQ_LEN - 1)];

  ret = nvm3_writeData(nvm3_defaultHandle, req.key, req.data, req.data_len);

  if (ret != ECODE_NVM3_OK) {
    return SL_SID_PDP_STATUS_ERR_NVM3_WRITE;
  }

  if (nvm3_repackNeeded(nvm3_defaultHandle)) {
    ret = nvm3_repack(nvm3_defaultHandle);
    if (ret != ECODE_NVM3_OK) {
      return SL_SID_PDP_STATUS_ERR_NVM3_REPACK;
    }
  }

  return SL_SID_PDP_STATUS_SUCCESS;
}

sl_sid_pdp_status_t sl_sid_pdp_priv_key_prov_inject_key(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len)
{
  (void)out;
  (void)out_len;
  (void)out_size;

  if (in == NULL || in_len < SL_SID_PDP_PRIV_KEY_PROV_MIN_INJECT_KEY_REQ_LEN) {
    return SL_SID_PDP_STATUS_ERR_IN_ARGS_NOT_VALID;
  }

  psa_key_attributes_t attr;
  psa_status_t ret;
  sl_sid_pdp_priv_key_prov_inject_key_req_t req = *(sl_sid_pdp_priv_key_prov_inject_key_req_t *)in;
  req.key = &in[29];

  ret = psa_crypto_init();
  if (ret != PSA_SUCCESS) {
    return SL_SID_PDP_STATUS_ERR_PSA_CRYPTO_INIT;
  }

  attr = psa_key_attributes_init();
  psa_set_key_lifetime(&attr, PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION((psa_key_lifetime_t)req.key_attr.lifetime, (psa_key_location_t)req.key_attr.location));
  psa_set_key_id(&attr, (mbedtls_svc_key_id_t)req.key_id);
  psa_set_key_usage_flags(&attr, (psa_key_usage_t)req.key_attr.usage_flags);
  psa_set_key_bits(&attr, (size_t)req.key_attr.bits);
  psa_set_key_type(&attr, PSA_KEY_TYPE_ECC_KEY_PAIR((psa_ecc_family_t)req.key_attr.type));
  psa_set_key_algorithm(&attr, (psa_algorithm_t)req.key_attr.algo);
  mbedtls_svc_key_id_t key_id;
  ret = psa_import_key(&attr, req.key, req.key_len, &key_id);
  if (ret != PSA_SUCCESS) {
    return SL_SID_PDP_STATUS_ERR_PSA_IMPORT_KEY;
  }
  req.key_id = key_id;

#ifdef DEBUG_PDP
  uint8_t hash[64];
  uint8_t sign[64];
  size_t sign_len;
  ret = psa_sign_message((mbedtls_svc_key_id_t)req.key_id, (psa_algorithm_t)req.key_attr.algo, hash, sizeof(hash), sign, sizeof(sign), &sign_len);
  if (ret != PSA_SUCCESS) {
    return SL_SID_PDP_STATUS_ERR_PSA_SIGN_MESSAGE;
  }
#endif /* DEBUG_PDP */

  return SL_SID_PDP_STATUS_SUCCESS;
}

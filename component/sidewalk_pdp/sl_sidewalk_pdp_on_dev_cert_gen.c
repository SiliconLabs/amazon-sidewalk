/***************************************************************************//**
 * @file sl_sidewalk_pdp_on_dev_cert_gen.c
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
#include <sid_pal_crypto_ifc.h>
#include <sid_pal_mfg_store_ifc.h>
#include <string.h>
#include "sl_sidewalk_pdp_on_dev_cert_gen.h"

// -----------------------------------------------------------------------------
//                          Public Function Prototypes
// -----------------------------------------------------------------------------
extern void silabs_crypto_enable_sv(void);

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

sl_sid_pdp_status_t sl_sid_pdp_on_dev_cert_gen_init(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len)
{
  (void)in;
  (void)in_len;
  (void)out_size;

  silabs_crypto_enable_sv();
  sid_error_t sid_ret = sid_pal_crypto_init();
  if (sid_ret != SID_ERROR_NONE) {
    goto cleanup;
  }

  sid_pal_mfg_store_region_t mfg_store_region;
  sid_pal_mfg_store_init(mfg_store_region);

  sid_ret = sid_on_dev_cert_init();
  if (sid_ret != SID_ERROR_NONE) {
    goto cleanup;
  }

  cleanup:
  if (sid_ret != SID_ERROR_NONE) {
    memcpy(&out[0], (uint8_t *)&sid_ret, sizeof(sid_error_t));
    *out_len = sizeof(sid_error_t); // internal error info in case of failure
    return SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_INIT;
  }

  return SL_SID_PDP_STATUS_SUCCESS;
}

sl_sid_pdp_status_t sl_sid_pdp_on_dev_cert_gen_gen_smsn(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len)
{
  (void)out_size;

  sid_error_t sid_ret;

  if (in == NULL || in_len < SL_SID_PDP_ON_DEV_CERT_GEN_MIN_GEN_SMSN_REQ_LEN) {
    return SL_SID_PDP_STATUS_ERR_IN_ARGS_NOT_VALID;
  }

  if (out == NULL || out_len == NULL) {
    return SL_SID_PDP_STATUS_ERR_OUT_ARGS_NOT_VALID;
  }

  sl_sid_pdp_on_dev_cert_gen_gen_smsn_req_t req = *(sl_sid_pdp_on_dev_cert_gen_gen_smsn_req_t *)in;
  req.dev_type = &in[8];
  req.dsn = &in[8 + req.dev_type_len];
  req.apid = &in[8 + req.dev_type_len + req.dsn_len];
  if (req.board_id_len) {
    req.board_id = &in[8 + req.dev_type_len + req.dsn_len + req.apid_len];
  }

  // Note on device-type. It gets transformed into "Amazon-id" which is <device-type>-PRODUCTION
  uint16_t dev_type_total_len = req.dev_type_len + strlen(SL_SID_PDP_CERT_DEV_TYPE_SUFFIX) + 1;
  char dev_type_production[dev_type_total_len];
  memset(dev_type_production, 0, dev_type_total_len);
  memcpy(&dev_type_production[0], (const char *)req.dev_type, req.dev_type_len);
  memcpy(&dev_type_production[req.dev_type_len], SL_SID_PDP_CERT_DEV_TYPE_SUFFIX, strlen(SL_SID_PDP_CERT_DEV_TYPE_SUFFIX));

  uint16_t dsn_total_len = req.dsn_len + 1;
  char dsn[dsn_total_len];
  memset(dsn, 0, dsn_total_len);
  memcpy(dsn, (const char *)req.dsn, req.dsn_len);

  uint16_t apid_total_len = req.apid_len + 1;
  char apid[apid_total_len];
  memset(apid, 0, apid_total_len);
  memcpy(apid, (const char *)req.apid, req.apid_len);

  uint16_t board_id_total_len = req.board_id_len + 1;
  char board_id[board_id_total_len];
  memset(board_id, 0, board_id_total_len);
  memcpy(board_id, (const char *)req.board_id, req.board_id_len);

  const struct sid_on_dev_cert_info dev_info = {
    .dev_type = dev_type_production,
    .dsn = dsn,
    .apid = apid,
    .board_id = req.board_id_len != 0 ? board_id : NULL
  };

  uint8_t smsn[SID_ODC_SMSN_SIZE];
  memset(smsn, 0xFF, SID_ODC_SMSN_SIZE);
  if ((sid_ret = sid_on_dev_cert_generate_smsn(&dev_info, smsn)) != SID_ERROR_NONE) {
    goto cleanup;
  }

  cleanup:
  if (sid_ret != SID_ERROR_NONE) {
    memcpy(&out[0], (uint8_t *)&sid_ret, sizeof(sid_error_t));
    *out_len = sizeof(sid_error_t); // internal error info in case of failure
    return SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_GEN_SMSN;
  }

  uint32_t smsn_len = SID_ODC_SMSN_SIZE;
  memcpy(&out[0], (uint8_t *)&smsn_len, sizeof(smsn_len));
  memcpy(&out[4], (uint8_t *)smsn, SID_ODC_SMSN_SIZE);
  *out_len = sizeof(smsn_len) + smsn_len;

  return SL_SID_PDP_STATUS_SUCCESS;
}

sl_sid_pdp_status_t sl_sid_pdp_on_dev_cert_gen_gen_csr(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len)
{
  (void)out_size;

  sid_error_t sid_ret;

  if (in == NULL || in_len != SL_SID_PDP_ON_DEV_CERT_GEN_GEN_CSR_REQ_LEN) {
    return SL_SID_PDP_STATUS_ERR_IN_ARGS_NOT_VALID;
  }

  if (out == NULL || out_len == NULL) {
    return SL_SID_PDP_STATUS_ERR_OUT_ARGS_NOT_VALID;
  }

  sl_sid_pdp_on_dev_cert_gen_gen_csr_req_t req = *(sl_sid_pdp_on_dev_cert_gen_gen_csr_req_t *)in;

  uint8_t csr[SID_ODC_CSR_MAX_SIZE];
  size_t csr_size = SID_ODC_CSR_MAX_SIZE;

  if ((sid_ret = sid_on_dev_cert_generate_csr(req.algo, csr, &csr_size)) != SID_ERROR_NONE) {
    goto cleanup;
  }

  uint32_t csr_len = csr_size;
  memcpy(&out[0], (uint8_t *)&csr_len, sizeof(csr_len));
  memcpy(&out[4], (uint8_t *)csr, csr_len);
  *out_len = sizeof(csr_len) + csr_len;

  cleanup:
  if (sid_ret != SID_ERROR_NONE) {
    memcpy(&out[0], (uint8_t *)&sid_ret, sizeof(sid_error_t));
    *out_len = sizeof(sid_error_t); // internal error info in case of failure
    return SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_GEN_CSR;
  }

  return SL_SID_PDP_STATUS_SUCCESS;
}

sl_sid_pdp_status_t sl_sid_pdp_on_dev_cert_gen_write_cert_chain(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len)
{
  (void)out_size;

  if (in == NULL || in_len > SL_SID_PDP_ON_DEV_CERT_GEN_WRITE_CERT_CHAIN_REQ_LEN) {
    return SL_SID_PDP_STATUS_ERR_IN_ARGS_NOT_VALID;
  }

  if (out == NULL || out_len == NULL) {
    return SL_SID_PDP_STATUS_ERR_OUT_ARGS_NOT_VALID;
  }

  sl_sid_pdp_on_dev_cert_gen_write_cert_chain_req_t req = *(sl_sid_pdp_on_dev_cert_gen_write_cert_chain_req_t *)in;
  req.cert_chain = &in[3];

  struct sid_on_dev_cert_chain_params cert_chain_params = {
    .algo = req.algo,
    .cert_chain = (uint8_t *)req.cert_chain,
    .cert_chain_size = req.cert_chain_len,
  };
  sid_error_t sid_ret;

  if ((sid_ret = sid_on_dev_cert_write_cert_chain(&cert_chain_params)) != SID_ERROR_NONE) {
    goto cleanup;
  }

  cleanup:
  if (sid_ret != SID_ERROR_NONE) {
    memcpy(&out[0], (uint8_t *)&sid_ret, sizeof(sid_error_t));
    *out_len = sizeof(sid_error_t); // internal error info in case of failure
    return SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_WRITE_CERT_CHAIN;
  }

  return SL_SID_PDP_STATUS_SUCCESS;
}

sl_sid_pdp_status_t sl_sid_pdp_on_dev_cert_gen_write_app_key(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len)
{
  (void)out_size;

  if (in == NULL || in_len != SL_SID_PDP_ON_DEV_CERT_GEN_WRITE_APP_KEY_REQ_LEN) {
    return SL_SID_PDP_STATUS_ERR_IN_ARGS_NOT_VALID;
  }

  if (out == NULL || out_len == NULL) {
    return SL_SID_PDP_STATUS_ERR_OUT_ARGS_NOT_VALID;
  }

  sl_sid_pdp_on_dev_cert_gen_write_app_key_req_t req = *(sl_sid_pdp_on_dev_cert_gen_write_app_key_req_t *)in;
  req.app_key = &in[2];

  sid_error_t sid_ret;

  if ((sid_ret = sid_on_dev_cert_write_app_server_key((uint8_t *)req.app_key)) != SID_ERROR_NONE) {
    goto cleanup;
  }

  cleanup:
  if (sid_ret != SID_ERROR_NONE) {
    memcpy(&out[0], (uint8_t *)&sid_ret, sizeof(sid_error_t));
    *out_len = sizeof(sid_error_t); // internal error info in case of failure
    return SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_WRITE_APP_KEY;
  }

  return SL_SID_PDP_STATUS_SUCCESS;
}

sl_sid_pdp_status_t sl_sid_pdp_on_dev_cert_gen_commit(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len)
{
  (void)in;
  (void)in_len;
  (void)out_size;

  sid_error_t sid_ret;

  sid_ret = sid_on_dev_cert_verify_and_store();
  if (sid_ret != SID_ERROR_NONE) {
    goto cleanup;
  }

  cleanup:
  if (sid_ret != SID_ERROR_NONE) {
    memcpy(&out[0], (uint8_t *)&sid_ret, sizeof(sid_error_t));
    *out_len = sizeof(sid_error_t); // internal error info in case of failure
    return SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_COMMIT;
  }

  return SL_SID_PDP_STATUS_SUCCESS;
}

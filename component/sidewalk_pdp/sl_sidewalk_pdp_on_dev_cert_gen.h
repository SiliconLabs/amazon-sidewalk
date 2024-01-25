/***************************************************************************//**
 * @file sl_sidewalk_pdp_on_dev_cert_gen.h
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

#ifndef SL_SIDEWALK_PDP_ON_DEV_CERT_GEN_H
#define SL_SIDEWALK_PDP_ON_DEV_CERT_GEN_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include <sid_on_dev_cert.h>
#include "sl_common.h"
#include "sl_sidewalk_pdp_common.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define SL_SID_PDP_ON_DEV_CERT_GEN_MIN_GEN_SMSN_REQ_LEN (32)
#define SL_SID_PDP_ON_DEV_CERT_GEN_GEN_CSR_REQ_LEN (1)
#define SL_SID_PDP_ON_DEV_CERT_GEN_WRITE_CERT_CHAIN_REQ_LEN (SID_ODC_SCC_MAX_SIZE + 3)
#define SL_SID_PDP_ON_DEV_CERT_GEN_WRITE_APP_KEY_REQ_LEN (SID_ODC_ED25519_PUK_SIZE + 2)
#define SL_SID_PDP_CERT_DEV_TYPE_SUFFIX  ("-PRODUCTION")

SL_PACK_START(1)
typedef struct {
  uint16_t dev_type_len;
  uint16_t dsn_len;
  uint16_t apid_len;
  uint16_t board_id_len;
  const uint8_t *dev_type;
  const uint8_t *dsn;
  const uint8_t *apid;
  const uint8_t *board_id;
} SL_ATTRIBUTE_PACKED sl_sid_pdp_on_dev_cert_gen_gen_smsn_req_t;
SL_PACK_END()

SL_PACK_START(1)
typedef struct {
  uint8_t algo;
} SL_ATTRIBUTE_PACKED sl_sid_pdp_on_dev_cert_gen_gen_csr_req_t;
SL_PACK_END()

SL_PACK_START(1)
typedef struct {
  uint8_t algo;
  uint16_t cert_chain_len;
  const uint8_t *cert_chain;
} SL_ATTRIBUTE_PACKED sl_sid_pdp_on_dev_cert_gen_write_cert_chain_req_t;
SL_PACK_END()

SL_PACK_START(1)
typedef struct {
  uint16_t app_key_len;
  const uint8_t *app_key;
} SL_ATTRIBUTE_PACKED sl_sid_pdp_on_dev_cert_gen_write_app_key_req_t;
SL_PACK_END()

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief Initializes Amazon on-device certificate module.
 *
 * @warning
 *   `in` buffer should not be overwritten because request data is not copied
 *   into another buffer but kept in `in` buffer for memory optimisation.
 *
 * @warning
 *   Response buffer contains extra error information encoded in 4 bytes when
 *   SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_INIT is returned.
 *
 * @param[in] in Request
 * @param[in] in_len Length of the request
 * @param[out] out Response buffer
 * @param[out] out_size Size of the response buffer
 * @param[out] out_len Length of the response if any
 *
 * @return Status code
 * @retval SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_INIT Amazon API call failed
 * @retval SL_SID_PDP_STATUS_SUCCESS Success
 ******************************************************************************/
sl_sid_pdp_status_t sl_sid_pdp_on_dev_cert_gen_init(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len);

/***************************************************************************//**
 * @brief Generates SMSN.
 *
 * @warning
 *   `in` buffer should not be overwritten because request data is not copied
 *   into another buffer but kept in `in` buffer for memory optimisation.
 *
 * @warning
 *   Response buffer contains extra error information encoded in 4 bytes when
 *   SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_GEN_SMSN is returned. In case of success
 *   SMSN length encoded in 4 bytes concatenated with generated SMSN is returned.
 *
 * @param[in] in Request
 * @param[in] in_len Length of the request
 * @param[out] out Response buffer
 * @param[out] out_size Size of the response buffer
 * @param[out] out_len Length of the response if any
 *
 * @return Status code
 * @retval SL_SID_PDP_STATUS_ERR_IN_ARGS_NOT_VALID One or more input arguments are not valid
 * @retval SL_SID_PDP_STATUS_ERR_OUT_ARGS_NOT_VALID One or more output arguments are not valid
 * @retval SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_GEN_SMSN Amazon API call failed
 * @retval SL_SID_PDP_STATUS_SUCCESS Success
 ******************************************************************************/
sl_sid_pdp_status_t sl_sid_pdp_on_dev_cert_gen_gen_smsn(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len);

/***************************************************************************//**
 * @brief Generates CSR.
 *
 * @warning
 *   `in` buffer should not be overwritten because request data is not copied
 *   into another buffer but kept in `in` buffer for memory optimisation.
 *
 * @warning
 *   Response buffer contains extra error information encoded in 4 bytes when
 *   SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_GEN_CSR is returned. In case of success
 *   CSR length encoded in 4 bytes concatenated with generated CSR is returned.
 *
 * @param[in] in Request
 * @param[in] in_len Length of the request
 * @param[out] out Response buffer
 * @param[out] out_size Size of the response buffer
 * @param[out] out_len Length of the response if any
 *
 * @return Status code
 * @retval SL_SID_PDP_STATUS_ERR_IN_ARGS_NOT_VALID One or more input arguments are not valid
 * @retval SL_SID_PDP_STATUS_ERR_OUT_ARGS_NOT_VALID One or more output arguments are not valid
 * @retval SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_GEN_CSR Amazon API call failed
 * @retval SL_SID_PDP_STATUS_SUCCESS Success
 ******************************************************************************/
sl_sid_pdp_status_t sl_sid_pdp_on_dev_cert_gen_gen_csr(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len);

/***************************************************************************//**
 * @brief Writes certificate chain.
 *
 * @warning
 *   `in` buffer should not be overwritten because request data is not copied
 *   into another buffer but kept in `in` buffer for memory optimisation.
 *
 * @warning
 *   Response buffer contains extra error information encoded in 4 bytes when
 *   SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_WRITE_CERT_CHAIN is returned.
 *
 * @param[in] in Request
 * @param[in] in_len Length of the request
 * @param[out] out Response buffer
 * @param[out] out_size Size of the response buffer
 * @param[out] out_len Length of the response if any
 *
 * @return Status code
 * @retval SL_SID_PDP_STATUS_ERR_IN_ARGS_NOT_VALID One or more input arguments are not valid
 * @retval SL_SID_PDP_STATUS_ERR_OUT_ARGS_NOT_VALID One or more output arguments are not valid
 * @retval SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_WRITE_CERT_CHAIN Amazon API call failed
 * @retval SL_SID_PDP_STATUS_SUCCESS Success
 ******************************************************************************/
sl_sid_pdp_status_t sl_sid_pdp_on_dev_cert_gen_write_cert_chain(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len);

/***************************************************************************//**
 * @brief Writes application public key.
 *
 * @warning
 *   `in` buffer should not be overwritten because request data is not copied
 *   into another buffer but kept in `in` buffer for memory optimisation.
 *
 * @warning
 *   Response buffer contains extra error information encoded in 4 bytes when
 *   SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_WRITE_APP_KEY is returned.
 *
 * @param[in] in Request
 * @param[in] in_len Length of the request
 * @param[out] out Response buffer
 * @param[out] out_size Size of the response buffer
 * @param[out] out_len Length of the response if any
 *
 * @return Status code
 * @retval SL_SID_PDP_STATUS_ERR_IN_ARGS_NOT_VALID One or more input arguments are not valid
 * @retval SL_SID_PDP_STATUS_ERR_OUT_ARGS_NOT_VALID One or more output arguments are not valid
 * @retval SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_WRITE_APP_KEY Amazon API call failed
 * @retval SL_SID_PDP_STATUS_SUCCESS Success
 ******************************************************************************/
sl_sid_pdp_status_t sl_sid_pdp_on_dev_cert_gen_write_app_key(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len);
sl_sid_pdp_status_t sl_sid_pdp_on_dev_cert_gen_commit(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len);

#endif // SL_SIDEWALK_PDP_ON_DEV_CERT_GEN_H

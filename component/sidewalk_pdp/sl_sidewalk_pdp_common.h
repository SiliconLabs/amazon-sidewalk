/***************************************************************************//**
 * @file sl_sidewalk_pdp_common.h
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

#ifndef SL_PDP_SIDEWALK_COMMON_H
#define SL_PDP_SIDEWALK_COMMON_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include "sl_common.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

typedef enum {
  // Private key provisioning commands
  SL_SID_PDP_CMD_PRIV_KEY_PROV_WRITE_NVM3 = 0,
  SL_SID_PDP_CMD_PRIV_KEY_PROV_INJECT_KEY = 1,
  // On-device certificate generation commands
  SL_SID_PDP_CMD_ON_DEV_CERT_GEN_INIT = 2,
  SL_SID_PDP_CMD_ON_DEV_CERT_GEN_GEN_SMSN = 3,
  SL_SID_PDP_CMD_ON_DEV_CERT_GEN_GEN_CSR = 4,
  SL_SID_PDP_CMD_ON_DEV_CERT_GEN_WRITE_CERT_CHAIN = 5,
  SL_SID_PDP_CMD_ON_DEV_CERT_GEN_WRITE_APP_KEY = 6,
  SL_SID_PDP_CMD_ON_DEV_CERT_GEN_COMMIT = 7,
  // Unknown commands
  SL_SID_PDP_CMD_UNKNOWN
} sl_sid_pdp_cmd_t;

typedef enum {
  // Generic status
  SL_SID_PDP_STATUS_SUCCESS = 0,
  SL_SID_PDP_STATUS_ERR_IN_ARGS_NOT_VALID,
  SL_SID_PDP_STATUS_ERR_OUT_ARGS_NOT_VALID,
  // Parser status
  SL_SID_PDP_STATUS_ERR_PKT_LEN_TOO_SMALL,
  SL_SID_PDP_STATUS_ERR_CMD_UNKNOWN,
  // Silabs platform related status
  SL_SID_PDP_STATUS_ERR_NVM3_OPEN,
  SL_SID_PDP_STATUS_ERR_NVM3_WRITE,
  SL_SID_PDP_STATUS_ERR_NVM3_REPACK,
  SL_SID_PDP_STATUS_ERR_NVM3_CLOSE,
  SL_SID_PDP_STATUS_ERR_PSA_CRYPTO_INIT,
  SL_SID_PDP_STATUS_ERR_PSA_IMPORT_KEY,
  SL_SID_PDP_STATUS_ERR_PSA_SIGN_MESSAGE,
  // On-device certificate generation API status
  SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_INIT,
  SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_GEN_SMSN,
  SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_GEN_CSR,
  SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_WRITE_CERT_CHAIN,
  SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_WRITE_APP_KEY,
  SL_SID_PDP_STATUS_ERR_ON_DEV_CERT_GEN_COMMIT
} sl_sid_pdp_status_t;

#endif // SL_PDP_SIDEWALK_COMMON_H

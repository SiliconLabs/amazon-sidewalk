/***************************************************************************//**
 * @file sl_sidewalk_pdp_parser.h
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

#ifndef SL_SIDEWALK_PDP_PARSER_H
#define SL_SIDEWALK_PDP_PARSER_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include "sl_sidewalk_pdp_common.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define SL_SID_PDP_MIN_REQ_PACKET_LEN (4)

typedef struct {
  uint16_t cmd;
  uint16_t data_len;
  const uint8_t *data;
} sl_sid_pdp_req_packet_t;

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief Parses received packet.
 *
 * @warning
 *   `in` buffer should not be overwritten because request data is not copied
 *   into another buffer but kept in `in` buffer for memory optimisation.
 *
 * @param[in] in Received packet
 * @param[in] in_len Length of the received packet
 * @param[out] out Request buffer
 * @param[out] out_len Length of the request
 * @param[out] cmd Command
 *
 * @return Status code
 * @retval SL_SID_PDP_STATUS_ERR_IN_ARGS_NOT_VALID One or more input arguments are not valid
 * @retval SL_SID_PDP_STATUS_ERR_OUT_ARGS_NOT_VALID One or more output arguments are not valid
 * @retval SL_SID_PDP_STATUS_ERR_PKT_LEN_TOO_SMALL Received packet is too small
 * @retval SL_SID_PDP_STATUS_ERR_CMD_UNKNOWN Command is not defined
 * @retval SL_SID_PDP_STATUS_SUCCESS Success
 ******************************************************************************/
sl_sid_pdp_status_t sl_sid_pdp_parse_req_packet(const uint8_t * const in, uint16_t in_len, const uint8_t **out, uint16_t * const out_len, uint8_t * const cmd);

#endif // SL_SIDEWALK_PDP_PARSER_H

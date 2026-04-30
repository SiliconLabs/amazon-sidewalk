/***************************************************************************//**
 * @file sl_sidewalk_pdp_parser.c
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

#include <string.h>
#include "sl_iostream.h"
#include "sl_sidewalk_pdp_parser.h"

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

sl_sid_pdp_status_t sl_sid_pdp_receive_packet(uint8_t *rx_buf, uint16_t rx_buf_size, int16_t *rx_cnt)
{
  uint16_t data_len = 0;
  uint16_t rx_len = 0;
  *rx_cnt = 0;
  sl_sid_pdp_status_t status = SL_SID_PDP_STATUS_SUCCESS;

  while (1) {
    // check if the buffer is full
    if (*rx_cnt >= rx_buf_size) {
      return SL_SID_PDP_STATUS_ERR_BUF_SIZE_TOO_SMALL;
    }

    // read 1-byte at a time and wait for the whole packet to be received
    sl_status_t sl_st = sl_iostream_read(sl_iostream_get_default(),
                                         (void *)&rx_buf[*rx_cnt],
                                         SL_SID_PDP_IOSTREAM_READ_GRANULARITY,
                                         (size_t *)&rx_len);
    if (sl_st == SL_STATUS_OK && rx_len == SL_SID_PDP_IOSTREAM_READ_GRANULARITY) {
      (*rx_cnt)++;
      if (*rx_cnt == SL_SID_PDP_MIN_REQ_PACKET_LEN) {
        // command and data_len are received
        data_len = *((uint16_t *)&rx_buf[SL_SID_PDP_DATA_LEN_START_IDX]);
      }
      if (*rx_cnt == data_len + SL_SID_PDP_MIN_REQ_PACKET_LEN) {
        // the whole packet is received
        break;
      }
    }
  }

  return status;
}

sl_sid_pdp_status_t sl_sid_pdp_parse_req_packet(const uint8_t * const in,
                                                uint16_t in_len,
                                                const uint8_t **out,
                                                uint16_t * const out_len,
                                                uint8_t * const cmd)
{
  if (in == NULL || in_len == 0) {
    return SL_SID_PDP_STATUS_ERR_IN_ARGS_NOT_VALID;
  }

  if (out == NULL || out_len == NULL || cmd == NULL) {
    return SL_SID_PDP_STATUS_ERR_OUT_ARGS_NOT_VALID;
  }

  uint16_t idx = 0;

  // check if the packet has at least cmd and data_len fields
  // there may be packets without data
  if (in_len < SL_SID_PDP_MIN_REQ_PACKET_LEN) {
    return SL_SID_PDP_STATUS_ERR_PKT_LEN_TOO_SMALL;
  }

  *cmd = *((uint16_t *)&in[idx]);
  if (*cmd >= SL_SID_PDP_CMD_UNKNOWN) {
    return SL_SID_PDP_STATUS_ERR_CMD_UNKNOWN;
  }
  idx += sizeof(uint16_t);
  *out_len = *((uint16_t *)&in[idx]);
  idx += sizeof(uint16_t);
  *out = &in[idx];

  return SL_SID_PDP_STATUS_SUCCESS;
}

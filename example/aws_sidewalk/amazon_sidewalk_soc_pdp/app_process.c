/***************************************************************************//**
 * @file app_process.c
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
#include <stdint.h>
#include "app_process.h"
#include "sl_iostream_rtt.h"
#include "sl_sidewalk_pdp_common.h"
#include "sl_sidewalk_pdp_parser.h"
#include "sl_sidewalk_pdp_priv_key_prov.h"
#include "sl_sidewalk_pdp_on_dev_cert_gen.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

typedef sl_sid_pdp_status_t (*process_req_t)(const uint8_t * const in, uint16_t in_len, uint8_t * const out, uint16_t out_size, uint16_t * const out_len);

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static uint8_t rx_pkt[SL_SID_PDP_RX_BUF_SIZE];
static uint8_t tx_pkt[SL_SID_PDP_TX_BUF_SIZE];
static process_req_t process_req[] = {
  sl_sid_pdp_priv_key_prov_write_nvm3,
  sl_sid_pdp_priv_key_prov_inject_key,
  sl_sid_pdp_on_dev_cert_gen_init,
  sl_sid_pdp_on_dev_cert_gen_gen_smsn,
  sl_sid_pdp_on_dev_cert_gen_gen_csr,
  sl_sid_pdp_on_dev_cert_gen_write_cert_chain,
  sl_sid_pdp_on_dev_cert_gen_write_app_key,
  sl_sid_pdp_on_dev_cert_gen_commit
};

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

static void send_response(uint32_t status, uint32_t data_len, const uint8_t *data);

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void assertEFM(const char *file, int line)
{
  (void)file;
  (void)line;
  while (1)
    ;
}

void app_process(void)
{
  uint16_t rx_len = 0;
  sl_status_t sl_st;

  sl_st = sl_iostream_read(sl_iostream_rtt_handle, (void *)rx_pkt, SL_SID_PDP_RX_BUF_SIZE, (size_t *)&rx_len);
  if (sl_st == SL_STATUS_OK && rx_len > 0) {
    const uint8_t *req = NULL;
    uint16_t req_len = 0;
    uint16_t tx_len = 0;
    uint8_t cmd;
    sl_sid_pdp_status_t sl_sid_pdp_st;

    // parse received packet into request
    sl_sid_pdp_st = sl_sid_pdp_parse_req_packet(rx_pkt, rx_len, &req, &req_len, &cmd);
    if (sl_sid_pdp_st != SL_SID_PDP_STATUS_SUCCESS) {
      goto cleanup;
    }

    // process request and prepare response if needed
    sl_sid_pdp_st = process_req[cmd](req, req_len, tx_pkt, sizeof(tx_pkt), &tx_len);
    if (sl_sid_pdp_st != SL_SID_PDP_STATUS_SUCCESS) {
      goto cleanup;
    }

    cleanup:
    send_response(sl_sid_pdp_st, tx_len, tx_pkt);
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static void send_response(uint32_t status, uint32_t data_len, const uint8_t *data)
{
  sl_iostream_write(sl_iostream_rtt_handle, (const void *)&status, sizeof(status));
  if (data_len != 0 && data != NULL) {
    sl_iostream_write(sl_iostream_rtt_handle, (const void *)data, data_len);
  }
}

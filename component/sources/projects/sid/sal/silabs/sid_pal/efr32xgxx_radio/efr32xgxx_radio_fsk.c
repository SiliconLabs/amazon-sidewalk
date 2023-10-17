/***************************************************************************//**
 * @file
 * @brief efr32xgxx_radio_fsk.c
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
/**
 * Modulation parameters are plan to remove in HALO-5123.
 * Packet parameters are under discussion to remove or support in the future.
 * So part of functions are place holders here, and will be well handled after discussion.
 */
#include "efr32xgxx_radio.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define RADIO_FSK_PACKET_TYPE_OFFSET             (0)
#define RADIO_FSK_PACKET_LENGTH_OFFSET           (1)
#define RADIO_FSK_FCS_TYPE_BIT                   (4)
#define RADIO_FSK_WHITENING_BIT                  (3)
#define RADIO_FCS_LEN_2_BYTES                    (2)
#define RADIO_FCS_LEN_4_BYTES                    (4)

#define MAX_PAYLOAD_LENGTH_WITH_FCS_TYPE_0       (251)
#define MAX_PAYLOAD_LENGTH_WITH_FCS_TYPE_1       (253)

#define RF_NOISE_FLOOR                           (-90)

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static void radio_mp_to_efr32xgxx_mp(efr32xgxx_mod_params_gfsk_t *fsk_mp,
                                     const sid_pal_radio_fsk_modulation_params_t *mod_params);
static void radio_pp_to_efr32xgxx_pp(efr32xgxx_pkt_params_gfsk_t *fsk_pp,
                                     const sid_pal_radio_fsk_packet_params_t *packet_params);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
int32_t radio_fsk_process_rx_done(const halo_drv_silabs_ctx_t *drv_ctx)
{
  sid_pal_radio_fsk_phy_hdr_t phy_hdr;
  sid_pal_radio_rx_packet_t   *radio_rx_packet                     = drv_ctx->radio_rx_packet;
  int32_t                     err                                  = RADIO_ERROR_NONE;
  uint8_t                     *buffer                              = radio_rx_packet->rcv_payload;
  uint8_t                     phr[EFR32XGXX_PHR_LENGTH]            = { 0 };
  int8_t                      rssi                                 = 0;
  static int8_t               rssi_avg                             = 0;

  radio_rx_packet->payload_len = efr32xgxx_get_rxpacket(phr, buffer, &rssi, PAYLOAD_IS_MSB);
  if (radio_rx_packet->payload_len <= 0) {
    radio_rx_packet->payload_len = 0;
    err = RADIO_ERROR_GENERIC;
    goto ret;
  }

  phy_hdr.fcs_type = (reverse8(phr[RADIO_FSK_PACKET_TYPE_OFFSET]) >> RADIO_FSK_FCS_TYPE_BIT);
  phy_hdr.is_data_whitening_enabled =
    (reverse8(phr[RADIO_FSK_PACKET_TYPE_OFFSET]) >> RADIO_FSK_WHITENING_BIT) ? true : false;

  switch (phy_hdr.fcs_type) {
    case RADIO_FSK_FCS_TYPE_0:
      radio_rx_packet->payload_len -= RADIO_FCS_LEN_4_BYTES;
      break;
    case RADIO_FSK_FCS_TYPE_1:
      radio_rx_packet->payload_len -= RADIO_FCS_LEN_2_BYTES;
      break;
    default:
      radio_rx_packet->payload_len = 0;
      err = RADIO_ERROR_GENERIC;
      goto ret;
  }

  if (err == RADIO_ERROR_NONE) {
    rssi_avg = (rssi_avg == 0) ? rssi : (rssi_avg + rssi) / 2;
    radio_rx_packet->fsk_rx_packet_status.rx_status = rssi - (RF_NOISE_FLOOR);
    radio_rx_packet->fsk_rx_packet_status.rssi_sync = rssi;
    radio_rx_packet->fsk_rx_packet_status.rssi_avg  = rssi_avg;
  }

  ret:
  return err;
}

sid_pal_radio_data_rate_t sid_pal_radio_fsk_mod_params_to_data_rate(const sid_pal_radio_fsk_modulation_params_t *mp)
{
  uint8_t data_rate = SID_PAL_RADIO_DATA_RATE_INVALID;

  // The modulation params are not yet finalized and is tracked in JIRA-5059.
  // These changes here will unblock the testing for GEN2 lights.
  if (mp->bit_rate == RADIO_FSK_BR_50KBPS && mp->freq_dev == RADIO_FSK_FDEV_25KHZ
      && mp->bandwidth == (uint8_t)EFR32XGXX_GFSK_BW_156200) {
    data_rate =  SID_PAL_RADIO_DATA_RATE_50KBPS;
  } else if (mp->bit_rate == RADIO_FSK_BR_150KBPS && mp->freq_dev == RADIO_FSK_FDEV_37_5KHZ
             && mp->bandwidth == (uint8_t)EFR32XGXX_GFSK_BW_312000) {
    data_rate = SID_PAL_RADIO_DATA_RATE_150KBPS;
  } else if (mp->bit_rate == RADIO_FSK_BR_250KBPS && mp->freq_dev == RADIO_FSK_FDEV_62_5KHZ
             && mp->bandwidth == (uint8_t)EFR32XGXX_GFSK_BW_467000) {
    data_rate = SID_PAL_RADIO_DATA_RATE_250KBPS;
  }

  return data_rate;
}

int32_t sid_pal_radio_fsk_data_rate_to_mod_params(sid_pal_radio_fsk_modulation_params_t *mod_params,
                                                  sid_pal_radio_data_rate_t data_rate)
{
  if (mod_params == NULL) {
    return RADIO_ERROR_INVALID_PARAMS;
  }

  switch (data_rate) {
    case SID_PAL_RADIO_DATA_RATE_50KBPS:
      mod_params->bit_rate    = RADIO_FSK_BR_50KBPS;
      mod_params->freq_dev    = RADIO_FSK_FDEV_25KHZ;
      mod_params->bandwidth   = (uint8_t)EFR32XGXX_GFSK_BW_156200;
      mod_params->mod_shaping = (uint8_t)EFR32XGXX_GFSK_MOD_SHAPE_BT_1;
      break;
    case SID_PAL_RADIO_DATA_RATE_150KBPS:
      mod_params->bit_rate    = RADIO_FSK_BR_150KBPS;
      mod_params->freq_dev    = RADIO_FSK_FDEV_37_5KHZ;
      mod_params->bandwidth   = (uint8_t)EFR32XGXX_GFSK_BW_312000;
      mod_params->mod_shaping = (uint8_t)EFR32XGXX_GFSK_MOD_SHAPE_BT_05;
      break;
    case SID_PAL_RADIO_DATA_RATE_250KBPS:
      mod_params->bit_rate    = RADIO_FSK_BR_250KBPS;
      mod_params->freq_dev    = RADIO_FSK_FDEV_62_5KHZ;
      mod_params->bandwidth   = (uint8_t)EFR32XGXX_GFSK_BW_467000;
      mod_params->mod_shaping = (uint8_t)EFR32XGXX_GFSK_MOD_SHAPE_BT_05;
      break;
    default:
      return RADIO_ERROR_INVALID_PARAMS;
  }

  return RADIO_ERROR_NONE;
}

// ringnet-phy will call this function to give default value to packet_params,
// and then call radio_set_fsk_packet_params and radio_set_fsk_sync_word in this
// file. If the parameters are fixed, we can decide to remove them.
// This API lists here is just a place holder for future.
int32_t sid_pal_radio_prepare_fsk_for_rx(sid_pal_radio_fsk_pkt_cfg_t *rx_pkt_cfg)
{
  int32_t err = RADIO_ERROR_INVALID_PARAMS;

  if (rx_pkt_cfg == NULL) {
    goto ret;
  }

  if (rx_pkt_cfg->phy_hdr == NULL || rx_pkt_cfg->packet_params == NULL
      || rx_pkt_cfg->sync_word == NULL) {
    goto ret;
  }

  sid_pal_radio_fsk_packet_params_t *f_pp = rx_pkt_cfg->packet_params;
  sid_pal_radio_fsk_phy_hdr_t       *phr  = rx_pkt_cfg->phy_hdr;
  uint8_t                           *sw   = rx_pkt_cfg->sync_word;

  f_pp->preamble_min_detect       = (uint8_t)EFR32XGXX_GFSK_PBL_DET_16_BITS;
  f_pp->sync_word_length          = EFR32XGXX_FSK_SYNC_WORD_LENGTH_IN_RX;
  f_pp->addr_comp                 = (uint8_t)EFR32XGXX_GFSK_ADDR_CMP_FILT_OFF;
  f_pp->header_type               = (uint8_t)EFR32XGXX_GFSK_PKT_FIX_LEN;
  f_pp->payload_length            = EFR32XGXX_FSK_MAX_PAYLOAD_LENGTH;
  f_pp->crc_type                  = (uint8_t)EFR32XGXX_GFSK_CRC_OFF;
  f_pp->radio_whitening_mode      = (uint8_t)EFR32XGXX_GFSK_DC_FREE_OFF;

  sw[0]                           = 0x55;
  sw[1]                           = phr->is_fec_enabled ? 0x6F : 0x90;
  sw[2]                           = 0x4E;

  err = RADIO_ERROR_NONE;

  ret:
  return err;
}

int32_t sid_pal_radio_prepare_fsk_for_tx(sid_pal_radio_fsk_pkt_cfg_t *tx_pkt_cfg)
{
  int32_t err = RADIO_ERROR_INVALID_PARAMS;

  do {
    if (tx_pkt_cfg == NULL) {
      break;
    }

    if (tx_pkt_cfg->phy_hdr == NULL || tx_pkt_cfg->packet_params == NULL
        || tx_pkt_cfg->sync_word == NULL || tx_pkt_cfg->payload == NULL) {
      break;
    }

    sid_pal_radio_fsk_packet_params_t *f_pp = tx_pkt_cfg->packet_params;
    if (f_pp->payload_length == 0 || f_pp->preamble_length == 0) {
      break;
    }

    sid_pal_radio_fsk_phy_hdr_t *phr = tx_pkt_cfg->phy_hdr;
    if (((phr->fcs_type == RADIO_FSK_FCS_TYPE_0)
         && (f_pp->payload_length > MAX_PAYLOAD_LENGTH_WITH_FCS_TYPE_0))
        || ((phr->fcs_type == RADIO_FSK_FCS_TYPE_1)
            && (f_pp->payload_length > MAX_PAYLOAD_LENGTH_WITH_FCS_TYPE_1))) {
      break;
    }

    uint8_t *sync_word                                  = tx_pkt_cfg->sync_word;
    uint8_t sync_word_length_in_byte                    = 0;
    uint8_t psdu_length                                 = f_pp->payload_length;
    uint8_t tx_buffer[EFR32XGXX_FSK_MAX_PAYLOAD_LENGTH] = { 0 };
    uint32_t crc                                        = 0x00000000;
    uint16_t tx_buffer_length                           = 0;

    if (phr->fcs_type == RADIO_FSK_FCS_TYPE_0) {
      crc = compute_crc32(tx_pkt_cfg->payload, f_pp->payload_length);

      tx_buffer[EFR32XGXX_PHR_LENGTH + psdu_length++] = (uint8_t)(crc >> 24);
      tx_buffer[EFR32XGXX_PHR_LENGTH + psdu_length++] = (uint8_t)(crc >> 16);
      tx_buffer[EFR32XGXX_PHR_LENGTH + psdu_length++] = (uint8_t)(crc >> 8);
      tx_buffer[EFR32XGXX_PHR_LENGTH + psdu_length++] = (uint8_t)(crc >> 0);
    } else if (phr->fcs_type == RADIO_FSK_FCS_TYPE_1) {
      crc = compute_crc16(tx_pkt_cfg->payload, f_pp->payload_length);

      tx_buffer[EFR32XGXX_PHR_LENGTH + psdu_length++] = (uint8_t)(crc >> 8);
      tx_buffer[EFR32XGXX_PHR_LENGTH + psdu_length++] = (uint8_t)(crc >> 0);
    } else {
      err = RADIO_ERROR_NOT_SUPPORTED;
      break;
    }

    tx_buffer_length = efr32xgxx_set_phr(tx_buffer, 0,
                                         (phr->fcs_type == RADIO_FSK_FCS_TYPE_1) ? 1 : 0,
                                         phr->is_data_whitening_enabled, psdu_length);
    tx_buffer_length += EFR32XGXX_PHR_LENGTH;
    memcpy(tx_buffer + EFR32XGXX_PHR_LENGTH, tx_pkt_cfg->payload, f_pp->payload_length);

    sync_word[sync_word_length_in_byte++] = 0x55;      // Added to force the preamble polarity to a real "0x55"
    sync_word[sync_word_length_in_byte++] = (phr->is_fec_enabled == true) ? 0x6F : 0x90;
    sync_word[sync_word_length_in_byte++] = 0x4E;

    f_pp->preamble_min_detect  = (uint8_t)EFR32XGXX_GFSK_PBL_DET_16_BITS;
    f_pp->sync_word_length     = sync_word_length_in_byte;
    f_pp->addr_comp            = (uint8_t)EFR32XGXX_GFSK_ADDR_CMP_FILT_OFF;
    f_pp->header_type          = (uint8_t)EFR32XGXX_GFSK_PKT_FIX_LEN;
    f_pp->payload_length       = EFR32XGXX_PHR_LENGTH + tx_buffer_length;
    f_pp->crc_type             = (uint8_t)EFR32XGXX_GFSK_CRC_OFF;
    f_pp->radio_whitening_mode = (uint8_t)EFR32XGXX_GFSK_DC_FREE_OFF;

    memcpy(tx_pkt_cfg->payload, tx_buffer, f_pp->payload_length);
    err = RADIO_ERROR_NONE;
  } while (0);

  return err;
}

int32_t sid_pal_radio_set_fsk_sync_word(const uint8_t *sync_word, uint8_t sync_word_length)
{
  int32_t err = RADIO_ERROR_NONE;
  if (efr32xgxx_set_gfsk_sync_word(sync_word, sync_word_length) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
  }

  return err;
}

// This API lists here is just a place holder for compilation now.
int32_t sid_pal_radio_set_fsk_whitening_seed(uint16_t seed)
{
  (void)seed;
  return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_fsk_modulation_params(const sid_pal_radio_fsk_modulation_params_t *mod_params)
{
  if (mod_params == NULL) {
    return RADIO_ERROR_INVALID_PARAMS;
  }

  efr32xgxx_mod_params_gfsk_t fsk_mp;
  radio_mp_to_efr32xgxx_mp(&fsk_mp, mod_params);
  if (efr32xgxx_set_gfsk_mod_params(&fsk_mp) != RADIO_ERROR_NONE) {
    return RADIO_ERROR_HARDWARE_ERROR;
  }

  return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_fsk_packet_params(const sid_pal_radio_fsk_packet_params_t *packet_params)
{
  if (packet_params == NULL) {
    return RADIO_ERROR_INVALID_PARAMS;
  }

  efr32xgxx_pkt_params_gfsk_t fsk_pp;
  radio_pp_to_efr32xgxx_pp(&fsk_pp, packet_params);
  if (efr32xgxx_set_gfsk_pkt_params(&fsk_pp) != RADIO_ERROR_NONE) {
    return RADIO_ERROR_HARDWARE_ERROR;
  }

  return RADIO_ERROR_NONE;
}

uint32_t sid_pal_radio_fsk_time_on_air(const sid_pal_radio_fsk_modulation_params_t *mod_params,
                                       const sid_pal_radio_fsk_packet_params_t *packet_params,
                                       uint8_t packetLen)
{
  if (mod_params == NULL || packet_params == NULL) {
    return 0;
  }

  efr32xgxx_pkt_params_gfsk_t fsk_pp;
  efr32xgxx_mod_params_gfsk_t fsk_mp;

  radio_mp_to_efr32xgxx_mp(&fsk_mp, mod_params);
  fsk_pp.pld_len_in_bytes = packetLen;
  radio_pp_to_efr32xgxx_pp(&fsk_pp, packet_params);

  return efr32xgxx_get_gfsk_time_on_air_in_ms(&fsk_pp, &fsk_mp);
}

uint32_t sid_pal_radio_fsk_get_fsk_number_of_symbols(const sid_pal_radio_fsk_modulation_params_t *mod_params,
                                                     uint32_t delay_micro_secs) // TODO: Implement in the generic way
{
  uint32_t num_symb = EFR32XGXX_US_TO_SYMBOLS(delay_micro_secs, mod_params->bit_rate);
  return num_symb;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

// TODO: HALO-5123
// to remove modulation parameters support in radio hal
static void radio_mp_to_efr32xgxx_mp(efr32xgxx_mod_params_gfsk_t *fsk_mp,
                                     const sid_pal_radio_fsk_modulation_params_t *mod_params)
{
  fsk_mp->br_in_bps    = mod_params->bit_rate;
  fsk_mp->fdev_in_hz   = mod_params->freq_dev;
  fsk_mp->bw_dsb_param = (efr32xgxx_gfsk_bw_t)mod_params->bandwidth;
  fsk_mp->mod_shape    = (efr32xgxx_gfsk_mod_shapes_t)mod_params->mod_shaping;
}

// This function is for time on the air computation and for sync word length
// and preamble length modification.
static void radio_pp_to_efr32xgxx_pp(efr32xgxx_pkt_params_gfsk_t *fsk_pp,
                                     const sid_pal_radio_fsk_packet_params_t *packet_params)
{
  // NOTE: sid_diagnostics application sets preamble in bits wheras other sidewalk applications
  // in bytes. Following bytes to bits conversion is not compatible with sid_diagnostics. To make
  // it compatible just use the following statement
  // "fsk_pp->pbl_len_in_bits = packet_params->preamble_length"
  fsk_pp->pbl_len_in_bits       = packet_params->preamble_length << 3;
  fsk_pp->pbl_min_det           = (efr32xgxx_gfsk_pbl_det_t)packet_params->preamble_min_detect;
  fsk_pp->sync_word_len_in_bits = packet_params->sync_word_length << 3;
  fsk_pp->addr_cmp              = (efr32xgxx_gfsk_addr_cmp_t)packet_params->addr_comp;
  fsk_pp->hdr_type              = (efr32xgxx_gfsk_pkt_len_modes_t)packet_params->header_type;
  fsk_pp->pld_len_in_bytes      = packet_params->payload_length;
  fsk_pp->crc_type              = (efr32xgxx_gfsk_crc_types_t)packet_params->crc_type;
  fsk_pp->dc_free               = (efr32xgxx_gfsk_dc_free_t)packet_params->radio_whitening_mode;
}

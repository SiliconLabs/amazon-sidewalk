/***************************************************************************//**
 * @file
 * @brief efr32xgxx.h
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

#ifndef EFR32XGXX_H
#define EFR32XGXX_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>

#include "rail.h"
#include "rail_ieee802154.h"
#include "pa_conversions_efr32.h"
#include "em_emu.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define EFR32XGXX_LOG_INFO(fmt, args ...)            //HALO_LOG_INFO(fmt, ##args)
#define EFR32XGXX_LOG_ERROR(fmt, args ...)           //HALO_LOG_ERROR(fmt, ##args)

#define EFR32XGXX_TIMER_MS2USEC(X) (X * 1000)
#define EFR32XGXX_TIMER_SEC2MS(X)  (X * 1000)

#define PAYLOAD_IS_MSB                           1
#define MS_IN_SEC                                1000U
#define NS_IN_MS                                 1000000U
#define RSSI_ERROR_COUNT                         10

#define EFR32XGXX_PHR_LENGTH                     2
#define EFR32XGXX_FSK_MAX_PAYLOAD_LENGTH         255
#define EFR32XGXX_FSK_SYNC_WORD_LENGTH_IN_RX     3

#define RADIO_FSK_BR_50KBPS                      50000
#define RADIO_FSK_BR_150KBPS                     150000
#define RADIO_FSK_BR_250KBPS                     250000

#define RADIO_FSK_FDEV_19KHZ                     19000
#define RADIO_FSK_FDEV_25KHZ                     25000
#define RADIO_FSK_FDEV_37_5KHZ                   37500
#define RADIO_FSK_FDEV_62_5KHZ                   62500

/**
 * @brief EFR32XGXX GFSK modulation shaping enumeration definition
 */
typedef enum efr32xgxx_gfsk_mod_shapes_e {
  EFR32XGXX_GFSK_MOD_SHAPE_OFF   = 0x00,
  EFR32XGXX_GFSK_MOD_SHAPE_BT_03 = 0x08,
  EFR32XGXX_GFSK_MOD_SHAPE_BT_05 = 0x09,
  EFR32XGXX_GFSK_MOD_SHAPE_BT_07 = 0x0A,
  EFR32XGXX_GFSK_MOD_SHAPE_BT_1  = 0x0B,
} efr32xgxx_gfsk_mod_shapes_t;

/**
 * @brief EFR32XGXX GFSK Rx bandwidth enumeration definition
 */
typedef enum efr32xgxx_gfsk_bw_e {
  EFR32XGXX_GFSK_BW_4800   = 0x1F,
  EFR32XGXX_GFSK_BW_5800   = 0x17,
  EFR32XGXX_GFSK_BW_7300   = 0x0F,
  EFR32XGXX_GFSK_BW_9700   = 0x1E,
  EFR32XGXX_GFSK_BW_11700  = 0x16,
  EFR32XGXX_GFSK_BW_14600  = 0x0E,
  EFR32XGXX_GFSK_BW_19500  = 0x1D,
  EFR32XGXX_GFSK_BW_23400  = 0x15,
  EFR32XGXX_GFSK_BW_29300  = 0x0D,
  EFR32XGXX_GFSK_BW_39000  = 0x1C,
  EFR32XGXX_GFSK_BW_46900  = 0x14,
  EFR32XGXX_GFSK_BW_58600  = 0x0C,
  EFR32XGXX_GFSK_BW_78200  = 0x1B,
  EFR32XGXX_GFSK_BW_93800  = 0x13,
  EFR32XGXX_GFSK_BW_117300 = 0x0B,
  EFR32XGXX_GFSK_BW_156200 = 0x1A,
  EFR32XGXX_GFSK_BW_187200 = 0x12,
  EFR32XGXX_GFSK_BW_234300 = 0x0A,
  EFR32XGXX_GFSK_BW_312000 = 0x19,
  EFR32XGXX_GFSK_BW_373600 = 0x11,
  EFR32XGXX_GFSK_BW_467000 = 0x09,
} efr32xgxx_gfsk_bw_t;

/**
 * @brief EFR32XGXX GFSK modulation parameters structure definition
 */
typedef struct efr32xgxx_mod_params_gfsk_s {
  uint32_t                    br_in_bps;
  uint32_t                    fdev_in_hz;
  efr32xgxx_gfsk_mod_shapes_t mod_shape;
  efr32xgxx_gfsk_bw_t         bw_dsb_param;
} efr32xgxx_mod_params_gfsk_t;

/**
 * @brief EFR32XGXX GFSK preamble length Rx detection size enumeration definition
 */
typedef enum efr32xgxx_gfsk_pbl_det_e {
  EFR32XGXX_GFSK_PBL_DET_OFF     = 0x00,
  EFR32XGXX_GFSK_PBL_DET_08_BITS = 0x04,
  EFR32XGXX_GFSK_PBL_DET_16_BITS = 0x05,
  EFR32XGXX_GFSK_PBL_DET_24_BITS = 0x06,
  EFR32XGXX_GFSK_PBL_DET_32_BITS = 0x07,
} efr32xgxx_gfsk_pbl_det_t;

/**
 * @brief EFR32XGXX GFSK address filtering configuration enumeration definition
 */
typedef enum efr32xgxx_gfsk_addr_cmp_e {
  EFR32XGXX_GFSK_ADDR_CMP_FILT_OFF        = 0x00,
  EFR32XGXX_GFSK_ADDR_CMP_FILT_NODE       = 0x01,
  EFR32XGXX_GFSK_ADDR_CMP_FILT_NODE_BROAD = 0x02,
} efr32xgxx_gfsk_addr_cmp_t;

/**
 * @brief EFR32XGXX GFSK packet length enumeration definition
 */
typedef enum efr32xgxx_gfsk_pkt_len_modes_e {
  EFR32XGXX_GFSK_PKT_FIX_LEN = 0x00,    //!< The packet length is known on both sides, no header included
  EFR32XGXX_GFSK_PKT_VAR_LEN = 0x01,    //!< The packet length is variable, header included
} efr32xgxx_gfsk_pkt_len_modes_t;

/**
 * @brief EFR32XGXX GFSK CRC type enumeration definition
 */
typedef enum efr32xgxx_gfsk_crc_types_e {
  EFR32XGXX_GFSK_CRC_OFF         = 0x01,
  EFR32XGXX_GFSK_CRC_1_BYTE      = 0x00,
  EFR32XGXX_GFSK_CRC_2_BYTES     = 0x02,
  EFR32XGXX_GFSK_CRC_1_BYTE_INV  = 0x04,
  EFR32XGXX_GFSK_CRC_2_BYTES_INV = 0x06,
} efr32xgxx_gfsk_crc_types_t;

/**
 * @brief EFR32XGXX GFSK whitening control enumeration definition
 */
typedef enum efr32xgxx_gfsk_dc_free_e {
  EFR32XGXX_GFSK_DC_FREE_OFF       = 0x00,
  EFR32XGXX_GFSK_DC_FREE_WHITENING = 0x01,
} efr32xgxx_gfsk_dc_free_t;

/**
 * @brief EFR32XGXX GFSK packet parameters structure definition
 */
typedef struct efr32xgxx_pkt_params_gfsk_s {
  uint16_t                       pbl_len_in_bits;          //!< Preamble length in bits
  efr32xgxx_gfsk_pbl_det_t       pbl_min_det;              //!< Preamble detection length
  uint8_t                        sync_word_len_in_bits;    //!< Sync word length in bits
  efr32xgxx_gfsk_addr_cmp_t      addr_cmp;                 //!< Address filtering configuration
  efr32xgxx_gfsk_pkt_len_modes_t hdr_type;                 //!< Header type
  uint8_t                        pld_len_in_bytes;         //!< Payload length in bytes
  efr32xgxx_gfsk_crc_types_t     crc_type;                 //!< CRC type configuration
  efr32xgxx_gfsk_dc_free_t       dc_free;                  //!< Whitening configuration
} efr32xgxx_pkt_params_gfsk_t;

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
uint8_t reverse8(uint8_t n);
uint16_t reverse16(uint16_t n);
RAIL_Handle_t efr32xgxx_get_railhandle(void);
uint16_t efr32xgxx_set_phr(uint8_t *hdr, uint8_t modesw, uint8_t crc, uint8_t whitening, uint16_t len);
uint16_t efr32xgxx_get_rxpacket(uint8_t *phr, uint8_t *payload, int8_t *rssi, bool msb);
void efr32xgxx_radio_irq_process(void);
int32_t efr32xgxx_set_platform(void);
int32_t efr32xgxx_set_radio_init(void);
int32_t efr32xgxx_set_radio_init_hard(void);
void efr32xgxx_radio_irq_process(void);
void efr32xgxx_event_handler(void);
int32_t efr32xgxx_set_txpower(int8_t power);
void efr32xgxx_get_txpower(int8_t *max_tx_power, int8_t *min_tx_power);
int32_t efr32xgxx_set_tx_payload(const uint8_t *buffer, uint8_t size, bool msb);
int32_t efr32xgxx_set_sleep(void);
int32_t efr32xgxx_set_standby(void);
int32_t efr32xgxx_set_tx(const uint32_t timeout);
int32_t efr32xgxx_set_rx(const uint32_t timeout);
int32_t efr32xgxx_set_tx_cw(void);
int32_t efr32xgxx_set_tx_cpbl(void);
int32_t efr32xgxx_set_rf_freq(const uint32_t freq_in_hz);
int32_t efr32xgxx_get_rssi_inst(int16_t* rssi_in_dbm);
int32_t efr32xgxx_get_random_numbers(uint32_t* numbers, unsigned int n);
uint32_t efr32xgxx_get_gfsk_time_on_air_in_ms(const efr32xgxx_pkt_params_gfsk_t* pkt_p,
                                              const efr32xgxx_mod_params_gfsk_t* mod_p);
int32_t efr32xgxx_set_gfsk_sync_word(const uint8_t* sync_word, const uint8_t sync_word_len);
int32_t efr32xgxx_set_gfsk_mod_params(const efr32xgxx_mod_params_gfsk_t* params);
int32_t efr32xgxx_set_gfsk_pkt_params(const efr32xgxx_pkt_params_gfsk_t* params);

uint32_t compute_crc32(const uint8_t* buffer, uint16_t length);
uint16_t compute_crc16(const uint8_t* buffer, uint16_t length);

int32_t sid_pal_radio_get_fsk_mod_shaping(uint8_t idx, uint8_t *ms);
int16_t sid_pal_radio_get_fsk_mod_shaping_idx(uint8_t ms);
int32_t sid_pal_radio_get_fsk_bw(uint8_t idx, uint8_t *bw);
int16_t sid_pal_radio_get_fsk_bw_idx(uint8_t bw);
int32_t sid_pal_radio_get_fsk_addr_comp(uint8_t idx, uint8_t *ac);
int16_t sid_pal_radio_get_fsk_addr_comp_idx(uint8_t ac);
int32_t sid_pal_radio_get_fsk_preamble_detect(uint8_t idx, uint8_t *pd);
int16_t sid_pal_radio_get_fsk_preamble_detect_idx(uint8_t pd);
int32_t sid_pal_radio_get_fsk_crc_type(uint8_t idx, uint8_t *crc);
int16_t sid_pal_radio_get_fsk_crc_type_idx(uint8_t crc);

#ifdef __cplusplus
}
#endif

#endif /* EFR32XGXX_H */

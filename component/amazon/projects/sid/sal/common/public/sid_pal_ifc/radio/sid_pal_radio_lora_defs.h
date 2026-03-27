/*
 * Copyright 2020-2023 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_RADIO_LORA_DEFS_H
#define SID_PAL_RADIO_LORA_DEFS_H

/**
 * \addtogroup sid_ifc
 * @{
 */
/**
 * \addtogroup sid_pal_ifc
 * @{
 */
/**
 * \addtogroup sid_pal_radio_ifc
 * @{
 */
/**
 * \addtogroup sid_pal_radio_lora_ifc
 * @{
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * @addtogroup sid_pal_radio_lora_ifc_types Type definitions
 * @ingroup sid_pal_radio_lora_ifc
 * @{
 *****************************************************************************/

/**
 * @brief Spreading Factor
 */
#define SID_PAL_RADIO_LORA_SF5 0x05     /*!< Spreading factor 5 */
#define SID_PAL_RADIO_LORA_SF6 0x06     /*!< Spreading factor 6 */
#define SID_PAL_RADIO_LORA_SF7 0x07     /*!< Spreading factor 7 */
#define SID_PAL_RADIO_LORA_SF8 0x08     /*!< Spreading factor 8 */
#define SID_PAL_RADIO_LORA_SF9 0x09     /*!< Spreading factor 9 */
#define SID_PAL_RADIO_LORA_SF10 0x0A    /*!< Spreading factor 10 */
#define SID_PAL_RADIO_LORA_SF11 0x0B    /*!< Spreading factor 11 */
#define SID_PAL_RADIO_LORA_SF12 0x0C    /*!< Spreading factor 12 */

/**
 * @brief Bandwidth
 */
#define SID_PAL_RADIO_LORA_BW_7KHZ 0x00     /*!< Bandwidth of 7.8 KHz. */
#define SID_PAL_RADIO_LORA_BW_10KHZ 0x08    /*!< Bandwidth of 10.4 KHz. */
#define SID_PAL_RADIO_LORA_BW_15KHZ 0x01    /*!< Bandwidth of 15.6 KHz. */
#define SID_PAL_RADIO_LORA_BW_20KHZ 0x09    /*!< Bandwidth of 20.8 KHz. */
#define SID_PAL_RADIO_LORA_BW_31KHZ 0x02    /*!< Bandwidth of 31.25 KHz. */
#define SID_PAL_RADIO_LORA_BW_41KHZ 0x0A    /*!< Bandwidth of 41.7 KHz. */
#define SID_PAL_RADIO_LORA_BW_62KHZ 0x03    /*!< Bandwidth of 62.5 KHz. */
#define SID_PAL_RADIO_LORA_BW_125KHZ 0x04   /*!< Bandwidth of 125 KHz. */
#define SID_PAL_RADIO_LORA_BW_250KHZ 0x05   /*!< Bandwidth of 250 KHz. */
#define SID_PAL_RADIO_LORA_BW_500KHZ 0x06   /*!< Bandwidth of 500 KHz. */


/**
 * @brief Coding Rate
 */
#define SID_PAL_RADIO_LORA_CODING_RATE_4_5 0x01     /*!< Coding rate 4/5 */
#define SID_PAL_RADIO_LORA_CODING_RATE_4_6 0x02     /*!< Coding rate 4/6 */
#define SID_PAL_RADIO_LORA_CODING_RATE_4_7 0x03     /*!< Coding rate 4/7 */
#define SID_PAL_RADIO_LORA_CODING_RATE_4_8 0x04     /*!< Coding rate 4/8 */
#define SID_PAL_RADIO_LORA_CODING_RATE_4_5_LI 0x05  /*!< Coding rate 4/5 LI */
#define SID_PAL_RADIO_LORA_CODING_RATE_4_6_LI 0x06  /*!< Coding rate 4/6 LI */
#define SID_PAL_RADIO_LORA_CODING_RATE_4_8_LI 0x07  /*!< Coding rate 4/8 LI */

/**
 * @brief packet params header type
 */
#define SID_PAL_RADIO_LORA_HEADER_TYPE_VARIABLE_LENGTH 0x00     /*!< Variable length header */
#define SID_PAL_RADIO_LORA_HEADER_TYPE_FIXED_LENGTH 0x01        /*!< Fixed length header */

/**
 * @brief packet params crc modes
 */
#define SID_PAL_RADIO_LORA_CRC_OFF 0x00     /*!< CRC off */
#define SID_PAL_RADIO_LORA_CRC_ON 0x01      /*!< CRC on */

/**
 * @brief packet params IQ modes
 */
#define SID_PAL_RADIO_LORA_IQ_NORMAL 0x00       /*!< Normal IQ */
#define SID_PAL_RADIO_LORA_IQ_INVERTED 0x01     /*!< Inverted IQ */

/**
 * @brief packet params LI modes
 */
#define SID_PAL_RADIO_LORA_LDR_LONG_INTERLEAVER_OFF 0x00    /*!< Long interleaver off */
#define SID_PAL_RADIO_LORA_LDR_LONG_INTERLEAVER_ON  0x01    /*!< Long interleaver on */

/**
 * @brief cad params
 */
#define SID_PAL_RADIO_LORA_CAD_01_SYMBOL 0x00   /*!< 01 symbol */
#define SID_PAL_RADIO_LORA_CAD_02_SYMBOL 0x01   /*!< 02 symbol */
#define SID_PAL_RADIO_LORA_CAD_04_SYMBOL 0x02   /*!< 04 symbol */
#define SID_PAL_RADIO_LORA_CAD_08_SYMBOL 0x03   /*!< 08 symbol */
#define SID_PAL_RADIO_LORA_CAD_16_SYMBOL 0x04   /*!< 16 symbol */

#define SID_PAL_RADIO_LORA_CAD_EXIT_MODE_CAD_ONLY 0x00  /*!< CAD only */
#define SID_PAL_RADIO_LORA_CAD_EXIT_MODE_CAD_RX 0x01    /*!< CAD followed by RX */
#define SID_PAL_RADIO_LORA_CAD_EXIT_MODE_CAD_LBT 0x10   /*!< CAD followed by LBT */

#define SID_PAL_RADIO_LORA_SF5_SF6_MIN_PREAMBLE_LEN 12  /*!< Minimum preamble length for SF5 and SF6 */

/**
 * @brief timeout duration in usec. the conversion to semtech ticks is done by start_rx() start_tx()
 */
#define SECS_TO_MUS(X)                         (X * 1000000UL)
/**
 * @brief set max radio timeout to 5sec
 */
#define SID_PAL_RADIO_LORA_CAD_DEFAULT_TX_TIMEOUT       SECS_TO_MUS(5)
#define SID_PAL_RADIO_LORA_DEFAULT_TX_TIMEOUT           SID_PAL_RADIO_LORA_CAD_DEFAULT_TX_TIMEOUT   /*!< Default transmission timeout */
/**
 * @brief 1 sec timeout used by diagnostics code
 */
#define SID_PAL_RADIO_LORA_TIMEOUT_DURATION_1_SEC       SECS_TO_MUS(1)

#define SID_PAL_RADIO_LORA_PRIVATE_NETWORK_SYNC_WORD    0x1424                      /*!< Private network sync word */
#define SID_PAL_RADIO_LORA_PUBLIC_NETWORK_SYNC_WORD LORA_MAC_PUBLIC_SYNCWORD        /*!< Public network sync word */
#define SID_PAL_RADIO_LORA_MAX_PAYLOAD_LENGTH       250                             /*!< Maximum payload length */

#define SID_PAL_RADIO_LORA_ED_PREAMBLE_LENGTH_DEFAULT (250 << 3)                    /*!< Default preamble length */
#define SID_PAL_RADIO_LORA_ED_MOD_SHAPING MOD_SHAPING_G_BT_1                        /*!< Default modulation shaping */
#define SID_PAL_RADIO_LORA_ED_PREAMBLE_MIN_DETECT RADIO_PREAMBLE_DETECTOR_08_BITS   /*!< Default preamble detection */
#define SID_PAL_RADIO_LORA_ED_SYNCWORD_LENGTH_DEFAULT (3 << 3)                      /*!< Default sync word length */
#define SID_PAL_RADIO_LORA_ED_ADDRCOMP_DEFAULT RADIO_ADDRESSCOMP_FILT_OFF           /*!< Default address comparison */
#define SID_PAL_RADIO_LORA_ED_HEADER_TYPE_DEFAULT RADIO_PACKET_VARIABLE_LENGTH      /*!< Default header type */
#define SID_PAL_RADIO_LORA_ED_CRC_LENGTH_DEFAULT  RADIO_CRC_2_BYTES_CCIT            /*!< Default CRC length */
#define SID_PAL_RADIO_LORA_ED_RADIO_WHITENING_MODE_DEFAULT RADIO_DC_FREEWHITENING   /*!< Default whitening mode */
#define SID_PAL_RADIO_LORA_ED_PAYLOAD_LENGTH_DEFAULT 0                              /*!< Default payload length */
#define SID_PAL_RADIO_LORA_ED_DEFAULT_WHITENING_SEED 0x01FF                         /*!< Default whitening seed */

/**
 * @brief Sidewalk phy lora crc present
 */
typedef enum sid_pal_radio_lora_crc_present {
    SID_PAL_RADIO_CRC_PRESENT_INVALID = 0,                              /*!< Invalid value */
    SID_PAL_RADIO_CRC_PRESENT_OFF = 1,                                  /*!< CRC not present */
    SID_PAL_RADIO_CRC_PRESENT_ON = 2,                                   /*!< CRC present */
    SID_PAL_RADIO_CRC_PRESENT_MAX_NUM = SID_PAL_RADIO_CRC_PRESENT_ON,   /*!< Maximum number of CRC present */
} sid_pal_radio_lora_crc_present_t;

/**
 * @brief Sidewalk phy lora modulation parameters
 */
typedef struct sid_pal_radio_lora_modulation_params {
    uint8_t spreading_factor;   /*!< Spreading factor */
    uint8_t bandwidth;          /*!< Bandwidth */
    uint8_t coding_rate;        /*!< Coding rate */
} sid_pal_radio_lora_modulation_params_t;

/**
 * @brief Sidewalk phy lora packet parameters
 */
typedef struct sid_pal_radio_lora_packet_params {
    uint16_t preamble_length;       /*!< Length of the preamble */
    uint8_t header_type;            /*!< Type of the header */  
    uint8_t payload_length;         /*!< Length of the payload */
    uint8_t crc_mode;               /*!< Type of the CRC */
    uint8_t invert_IQ;              /*!< IQ inversion */
} sid_pal_radio_lora_packet_params_t;

/**
 * @brief Sidewalk Phy received LORA packet status
 */
typedef struct sid_pal_radio_lora_rx_packet_status {
    int16_t rssi;                                       /*!< RSSI (Received Signal Strength Indicator) */
    int8_t snr;                                         /*!< Signal-to-Noise Ratio */
    int8_t signal_rssi;                                 /*!< Signal RSSI */
    sid_pal_radio_lora_crc_present_t is_crc_present;    /*!< Flag to indicate if CRC is present */
} sid_pal_radio_lora_rx_packet_status_t;

/**
 * @brief Sidewalk phy lora cad parameters
 */
typedef struct sid_pal_radio_lora_cad_params {
    uint8_t cad_symbol_num;     /*!< Number of CAD symbols */
    uint8_t cad_detect_peak;    /*!< CAD detection peak */
    uint8_t cad_detect_min;     /*!< CAD detection minimum */
    uint8_t cad_exit_mode;      /*!< CAD exit mode */
    uint32_t cad_timeout;       /*!< CAD timeout in microseconds */
} sid_pal_radio_lora_cad_params_t;

/**
 * @brief Sidewalk phy lora configuation handle
 */
typedef struct sid_pal_radio_lora_phy_settings {
    uint32_t freq;                                                  /*!< Frequency for the LoRa modulation */
    int8_t power;                                                   /*!< Transmission power in dBm */
    uint16_t sync_word;                                             /*!< Sync word for the LoRa modulation */
    uint8_t symbol_timeout;                                         /*!< Symbol timeout value */
    uint32_t tx_timeout;                                            /*!< Transmission timeout in microseconds */
    uint8_t lora_ldr_long_interleaved_enable;                       /*!< Flag to enable long interleaved mode */
    sid_pal_radio_lora_modulation_params_t lora_modulation_params;  /*!< Modulation parameters for the LoRa modulation */
    sid_pal_radio_lora_packet_params_t lora_packet_params;          /*!< Packet parameters for the LoRa modulation */
    sid_pal_radio_lora_cad_params_t lora_cad_params;                /*!< CAD (Channel Activity Detection) parameters for the LoRa modulation */
} sid_pal_radio_lora_phy_settings_t;

/** @} (end sid_pal_radio_lora_ifc_types) */

#ifdef __cplusplus
}
#endif

#endif /* SID_PAL_RADIO_LORA_DEFS_H */

/** @} */ // end of sid_ifc group
/** @} */ // end of sid_pal_ifc group
/** @} */ // end of sid_pal_radio_ifc group
/** @} */ // end of sid_pal_radio_lora_ifc group
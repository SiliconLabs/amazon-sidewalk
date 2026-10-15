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

#ifndef SID_PAL_RADIO_FSK_DEFS_H
#define SID_PAL_RADIO_FSK_DEFS_H

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
 * \addtogroup sid_pal_radio_fsk_ifc
 * @{
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * @addtogroup sid_pal_radio_fsk_ifc_types Type definitions
 * @ingroup sid_pal_radio_fsk_ifc
 * @{
 *****************************************************************************/


/**
 * @brief Radio Mod Shaping parameter
 */
#define SID_PAL_RADIO_FSK_MOD_SHAPING_OFF 0x00      /*!< No modulation shaping. */
#define SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_03 0x08  /*!< Gaussian shaping with BT = 0.3. */
#define SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_05 0x09  /*!< Gaussian shaping with BT = 0.5. */
#define SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_07 0x0A  /*!< Gaussian shaping with BT = 0.7. */
#define SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_1 0x0B   /*!< Gaussian shaping with BT = 1.0. */

/**
 * @brief Bandwidth
 */
#define SID_PAL_RADIO_FSK_BW_4800 0x1F      /*!< Bandwidth of 4800 Hz. */
#define SID_PAL_RADIO_FSK_BW_5800 0x07      /*!< Bandwidth of 5800 Hz. */
#define SID_PAL_RADIO_FSK_BW_7300 0x0F      /*!< Bandwidth of 7300 Hz. */
#define SID_PAL_RADIO_FSK_BW_9700 0x1E      /*!< Bandwidth of 9700 Hz. */
#define SID_PAL_RADIO_FSK_BW_11700 0x16     /*!< Bandwidth of 11700 Hz. */
#define SID_PAL_RADIO_FSK_BW_14600 0x0E     /*!< Bandwidth of 14600 Hz. */
#define SID_PAL_RADIO_FSK_BW_19500 0x1D     /*!< Bandwidth of 19500 Hz. */
#define SID_PAL_RADIO_FSK_BW_23400 0x15     /*!< Bandwidth of 23400 Hz. */
#define SID_PAL_RADIO_FSK_BW_29300 0x0D     /*!< Bandwidth of 29300 Hz. */
#define SID_PAL_RADIO_FSK_BW_39000 0x1C     /*!< Bandwidth of 39000 Hz. */
#define SID_PAL_RADIO_FSK_BW_46900 0x14     /*!< Bandwidth of 46900 Hz. */
#define SID_PAL_RADIO_FSK_BW_58600 0x0C     /*!< Bandwidth of 58600 Hz. */
#define SID_PAL_RADIO_FSK_BW_78200 0x1B     /*!< Bandwidth of 78200 Hz. */
#define SID_PAL_RADIO_FSK_BW_93800 0x13     /*!< Bandwidth of 93800 Hz. */
#define SID_PAL_RADIO_FSK_BW_117300 0x0B    /*!< Bandwidth of 117300 Hz. */
#define SID_PAL_RADIO_FSK_BW_156200 0x1A    /*!< Bandwidth of 156200 Hz. */
#define SID_PAL_RADIO_FSK_BW_187200 0x12    /*!< Bandwidth of 187200 Hz. */
#define SID_PAL_RADIO_FSK_BW_234300 0x0A    /*!< Bandwidth of 234300 Hz. */
#define SID_PAL_RADIO_FSK_BW_312000 0x19    /*!< Bandwidth of 312000 Hz. */
#define SID_PAL_RADIO_FSK_BW_373600 0x11    /*!< Bandwidth of 373600 Hz. */
#define SID_PAL_RADIO_FSK_BW_467000 0x09    /*!< Bandwidth of 467000 Hz. */

#define SID_PAL_RADIO_FSK_BW_100KHZ SID_PAL_RADIO_FSK_BW_93800  /*!< Bandwidth of 100000 Hz. */
#define SID_PAL_RADIO_FSK_BW_117KHZ SID_PAL_RADIO_FSK_BW_117300 /*!< Bandwidth of 117000 Hz. */
#define SID_PAL_RADIO_FSK_BW_125KHZ SID_PAL_RADIO_FSK_BW_156200 /*!< Bandwidth of 125000 Hz. */
#define SID_PAL_RADIO_FSK_BW_150KHZ SID_PAL_RADIO_FSK_BW_156200 /*!< Bandwidth of 150000 Hz. */
#define SID_PAL_RADIO_FSK_BW_250KHZ SID_PAL_RADIO_FSK_BW_312000 /*!< Bandwidth of 250000 Hz. */
#define SID_PAL_RADIO_FSK_BW_500KHZ SID_PAL_RADIO_FSK_BW_467000 /*!< Bandwidth of 500000 Hz. */


/**
 * @brief Radio Preamble detection
 */
#define SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_OFF 0x00        /*!< Preamble detection off. */
#define SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_08_BITS 0x04    /*!< Preamble detection of 8 bits. */
#define SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_16_BITS 0x05    /*!< Preamble detection of 16 bits. */
#define SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_24_BITS 0x06    /*!< Preamble detection of 24 bits. */
#define SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_32_BITS 0x07    /*!< Preamble detection of 32 bits. */

/**
 * @brief Radio sync word correlators activated
 */
#define SID_PAL_RADIO_FSK_ADDRESSCOMP_FILT_OFF 0x00         /*!< Address comparison off. */
#define SID_PAL_RADIO_FSK_ADDRESSCOMP_FILT_NODE 0x01        /*!< Address comparison node. */
#define SID_PAL_RADIO_FSK_ADDRESSCOMP_FILT_NODE_BROAD 0x02  /*!< Address comparison node and broadcast. */

/**
 * @brief Radio packet length modes
 */
#define SID_PAL_RADIO_FSK_RADIO_PACKET_FIXED_LENGTH 0x00        /*!< Fixed length packet. */
#define SID_PAL_RADIO_FSK_RADIO_PACKET_VARIABLE_LENGTH 0x01     /*!< Variable length packet. */

/**
 * @brief packet params crc types
 */
#define SID_PAL_RADIO_FSK_CRC_OFF 0x01          /*!< CRC off. */
#define SID_PAL_RADIO_FSK_CRC_1_BYTES 0x00      /*!< CRC 1 byte. */
#define SID_PAL_RADIO_FSK_CRC_2_BYTES 0x02      /*!< CRC 2 bytes. */
#define SID_PAL_RADIO_FSK_CRC_1_BYTES_INV 0x04  /*!< CRC 1 byte inverted. */
#define SID_PAL_RADIO_FSK_CRC_2_BYTES_INV 0x06  /*!< CRC 2 bytes inverted. */
#define SID_PAL_RADIO_FSK_CRC_2_BYTES_IBM 0xF1  /*!< CRC 2 bytes IBM. */
#define SID_PAL_RADIO_FSK_CRC_2_BYTES_CCIT 0xF2 /*!< CRC 2 bytes CCIT. */

/**
 * @brief packet params Radio whitening mode
 */
#define SID_PAL_RADIO_FSK_DC_FREE_OFF 0x00      /*!< DC free off. */
#define SID_PAL_RADIO_FSK_DC_FREEWHITENING 0x01 /*!< DC free whitening. */

#define SID_PAL_RADIO_FSK_WHITENING_SEED 0x01FF /*!< Default whitening seed. */

#define SID_PAL_RADIO_FSK_SYNC_WORD_LENGTH   8  /*!< Length of the sync word in bits. */

/**
 * @brief timeout duration in usec. the conversion to semtec ticks is done by start_rx() start_tx()
 */
#define SECS_TO_MUS(X)                         (X * 1000000UL)
/**
 * @brief set max radio timeout to 5sec
 */
#define SID_PAL_RADIO_FSK_DEFAULT_TX_TIMEOUT            SECS_TO_MUS(5)
/**
 * @brief 1 sec timeout used by diagnostics code
 */
#define SID_PAL_RADIO_FSK_TIMEOUT_DURATION_1_SEC        SECS_TO_MUS(1)

/**
 * @brief Sidewalk Phy FSK CAD (Channel Activity Detection) parameters
 */
typedef struct sid_pal_radio_fsk_cad_params {
    int16_t fsk_ed_rssi_threshold;      /*!< RSSI threshold for energy detection */
    uint16_t fsk_ed_duration_mus;       /*!< Duration for energy detection in microseconds */
    uint8_t fsk_cs_min_prm_det;         /*!< Minimum preamble detection for carrier sense */
    uint32_t fsk_cs_duration_us;        /*!< Duration for carrier sense in microseconds */
    uint32_t fsk_cs_lbt_rx_timeout;     /*!< RX timeout for Listen Before Talk (LBT) */
    uint16_t fsk_cs_lbt_preamble_len;   /*!< Preamble length for Listen Before Talk (LBT) */
} sid_pal_radio_fsk_cad_params_t;

/**
 * @brief Sidewalk Phy FSK header type
 */
enum sid_pal_radio_fsk_header_type {
    SID_PAL_RADIO_FSK_SIDEWALK_HEADER = 0,  /*!< Sidewalk-specific header */
    SID_PAL_RADIO_FSK_CUSTOM_HEADER = 1,    /*!< Custom header */
};

/**
 * @brief Sidewalk phy fsk modulation parameters
 */
typedef struct sid_pal_radio_fsk_modulation_params {
    uint32_t bit_rate;                              /*!< Bit rate for the FSK modulation */
    uint32_t freq_dev;                              /*!< Frequency deviation for the FSK modulation */
    enum sid_pal_radio_fsk_header_type header_type; /*!< FSK header type */
    uint8_t mod_shaping;                            /*!< Modulation shaping parameter */
    uint8_t bandwidth;                              /*!< Bandwidth for the FSK modulation */
    uint8_t custom_rate_idx;                        /*!< Rate index if the data rate is custom */
} sid_pal_radio_fsk_modulation_params_t;

/**
 * @brief Sidewalk phy fsk packet parameters
 */
typedef struct sid_pal_radio_fsk_packet_params {
    uint16_t preamble_length;       /*!< Length of the preamble */
    uint8_t preamble_min_detect;    /*!< Minimum preamble detection */
    uint8_t sync_word_length;       /*!< Length of the sync word */
    uint8_t addr_comp;              /*!< Address comparison mode */
    uint8_t header_type;            /*!< Type of the header */
    uint8_t payload_length;         /*!< Length of the payload */
    uint8_t *payload;               /*!< Pointer to the payload */
    uint8_t crc_type;               /*!< Type of the CRC */
    uint8_t radio_whitening_mode;   /*!< Radio whitening mode */
} sid_pal_radio_fsk_packet_params_t;

/**
 * @brief Radio FSK FCS enumeration definition
 */
typedef enum {
    RADIO_FSK_FCS_TYPE_0 = 0,  /*!< 4-octet FCS */
    RADIO_FSK_FCS_TYPE_1 = 1,  /*!< 2-octet FCS */
} radio_fsk_fcs_t;

#define SID_MAX_CUSTOM_PHYHDR_SZ 4      /*!< Defines the maximum size of the custom PHY header. */

/**
 * @brief Radio FSK PHY HDR structure definition
 */
typedef struct {
    radio_fsk_fcs_t   fcs_type;                     /*!< FCS (Frame Check Sequence) type */
    bool              is_data_whitening_enabled;    /*!< Flag to indicate if data whitening is enabled */
    bool              is_fec_enabled;               /*!< Flag to indicate if FEC (Forward Error Correction) is enabled */
    uint8_t phy_hdr_len;                            /*!< Length of the PHY header */
    uint8_t phy_header[SID_MAX_CUSTOM_PHYHDR_SZ];   /*!< PHY header */
} sid_pal_radio_fsk_phy_hdr_t;

/**
 * @brief Sidewalk Phy FSK packet configuration
 */
typedef struct {
    sid_pal_radio_fsk_phy_hdr_t  *phy_hdr;              /*!< Pointer to the PHY header for the FSK modulation */
    sid_pal_radio_fsk_packet_params_t  *packet_params;  /*!< Pointer to the packet parameters for the FSK modulation */
    uint32_t                  packet_timeout;           /*!< Packet timeout in microseconds */
    uint8_t                   *sync_word;               /*!< Pointer to the sync word for the FSK modulation */
    uint8_t                   *payload;                 /*!< Pointer to the payload to be transmitted */
} sid_pal_radio_fsk_pkt_cfg_t;

/**
 * @brief Sidewalk Phy received FSK packet status
 */
typedef struct sid_pal_radio_fsk_rx_packet_status {
    int8_t rssi_avg;    /*!< Average RSSI (Received Signal Strength Indicator) */
    int8_t rssi_sync;   /*!< RSSI during sync word detection */
    int8_t snr;         /*!< Signal-to-Noise Ratio */
} sid_pal_radio_fsk_rx_packet_status_t;

/**
 * @brief Sidewalk phy fsk configuration handle
 */
typedef struct sid_pal_radio_fsk_phy_settings {
    uint32_t freq;                                                  /*!< Frequency for the FSK modulation */
    int8_t power;                                                   /*!< Transmission power in dBm */
    uint8_t sync_word[SID_PAL_RADIO_FSK_SYNC_WORD_LENGTH];          /*!< Sync word for the FSK modulation */
    uint8_t sync_word_len;                                          /*!< Length of the sync word */
    uint16_t whitening_seed;                                        /*!< Whitening seed for the FSK modulation */
    uint16_t crc_polynomial;                                        /*!< CRC polynomial for the FSK modulation */
    uint16_t crc_seed;                                              /*!< CRC seed for the FSK modulation */
    uint32_t tx_timeout;                                            /*!< Transmission timeout in microseconds */
    uint32_t symbol_timeout;                                        /*!< Symbol timeout in microseconds */
    sid_pal_radio_fsk_modulation_params_t fsk_modulation_params;    /*!< Modulation parameters for the FSK modulation */
    sid_pal_radio_fsk_packet_params_t fsk_packet_params;            /*!< Packet parameters for the FSK modulation */
    sid_pal_radio_fsk_cad_params_t fsk_cad_params;                  /*!< CAD (Channel Activity Detection) parameters for the FSK modulation */
    sid_pal_radio_fsk_phy_hdr_t fsk_phy_hdr;                        /*!< PHY header for the FSK modulation */
} sid_pal_radio_fsk_phy_settings_t;

/** @} (end sid_pal_radio_fsk_ifc_types) */

#ifdef __cplusplus
}
#endif

#endif /* SID_PAL_RADIO_FSK_DEFS_H */

/** @} */ // end of sid_ifc group
/** @} */ // end of sid_pal_ifc group
/** @} */ // end of sid_pal_radio_ifc group
/** @} */ // end of sid_pal_radio_fsk_ifc group
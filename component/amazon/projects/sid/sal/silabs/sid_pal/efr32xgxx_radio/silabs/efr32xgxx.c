/***************************************************************************//**
 * @file
 * @brief efr32xgxx.c
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stdbool.h>
#include <sid_clock_ifc.h>
#include <sid_pal_delay_ifc.h>
#include <sl_rail_util_compatible_pa.h>
#if defined(SL_SIDEWALK_UNIT_TEST)
#include "sl_sidewalk_log_pal_mock.h"
#else
#include "sl_sidewalk_log_pal.h"
#endif
#include <sid_pal_assert_ifc.h>

#include "silabs/efr32xgxx.h"
#include "efr32xgxx_radio.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define TX_FIFO_SIZE                                (256) // Any power of 2 from [64, 4096] on the EFR32
#define RF_RANDOM_TIMES                             (8)
#define RSSI_QUARTER_ORDER                          (2)

#define EFR32XGXX_RADIO_WARMUP_VALUE                (220)

#define EFR32XGXX_PHR_CRC16_BYTES                   (2)
#define EFR32XGXX_PHR_CRC32_BYTES                   (4)
#define EFR32XGXX_PHR_FIELD_LEN_SHIFT_BITS          (8)
#define EFR32XGXX_PHR_FIELD_CRC_SHIFT_BITS          (4)
#define EFR32XGXX_PHR_FIELD_WHITENING_SHIFT_BITS    (3)
#define EFR32XGXX_PHR_FIELD_MODESW_SHIFT_BITS       (7)
#define EFR32XGXX_PHR_FIELD_LEN_HI_MASK             (0x07)
#define EFR32XGXX_PHR_FIELD_LEN_LO_MASK             (0xff)

#define EFR32XGXX_RAIL_MAXPOWER                     (20)

#define EFR32XGXX_RAIL_INVALID_IDX                  (-1)
#define EFR32XGXX_RAIL_50KBPS_IDX                   (0)
#define EFR32XGXX_RAIL_150KBPS_IDX                  (1)
#define EFR32XGXX_RAIL_250KBPS_IDX                  (2)

#define EFR32XGXX_SYNCWORD_BYTES                    (8)

#define EFR32XGXX_CRC_16_TYPE                       (0)
#define EFR32XGXX_CRC_32_TYPE                       (1)
#define EFR32XGXX_CRC_BYTES_ZERO                    (0)
#define EFR32XGXX_CRC_BYTES_ONE                     (1)
#define EFR32XGXX_CRC_BYTES_TWO                     (2)

#define EFR32XGXX_MAX_PAYLOAD                       (255)
#define EFR32XGXX_PHR_LOW_BYTE                      (0)
#define EFR32XGXX_PHR_HIGH_BYTE                     (1)
#define EFR32XGXX_PHR_ENABLE                        (1)

#define POLYNOMIAL_CRC16                            (0x1021)
#define POLYNOMIAL_CRC32                            (0x04C11DB7)

#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
// Greater the priority value, lesser the priority
#define EFR32XGXX_RX_PRIORITY                       (200)
#define EFR32XGXX_TX_PRIORITY                       (100)

// RAIL scheduler reconfigures the radio in between protocol switches
#define EFR32XGXX_RADIO_PROTOCOL_SWITCH_TIME        (4000)  // (Max BLE packet transmit time + ACK) + (2 x the protocol switch time)
#endif

/** Macro to determine array size. */
#define COMMON_UTILS_COUNTOF(a) (sizeof(a) / sizeof((a)[0]))

#define SL_RAIL_BUILTIN_TX_FIFO_BYTES 512

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static void radio_irq(sl_rail_handle_t rail_handle, sl_rail_events_t events);
static uint32_t efr32xgxx_get_gfsk_crc_len_in_bytes(efr32xgxx_gfsk_crc_types_t crc_type);
static void efr32xgxx_cancel_radio_timer(void);
static void radio_cfg_changed_hander(sl_rail_handle_t rail_handle, const sl_rail_channel_config_entry_t *entry);
static void efr32xgxx_set_radio_idle(void);
static void efr32xgxx_rfready(sl_rail_handle_t rail_handle);
static void efr32xgxx_event_notify(sid_pal_radio_events_t radio_event);
static void efr32xgxx_rx_timer_expired(sl_rail_handle_t rail_handle);
static void efr32xgxx_tx_timer_expired(sl_rail_handle_t rail_handle);
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
static void efr32xgxx_radio_yield(void);
#endif
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
extern const sl_rail_channel_config_t *efr32xgxx_channelConfigs[]; // PHY link

sl_rail_handle_t g_rail_handle = SL_RAIL_EFR32_HANDLE;
// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
union g_tx_fifo_u {
  SL_RAIL_DECLARE_FIFO_BUFFER(aligned, TX_FIFO_SIZE);
  uint8_t bytes[TX_FIFO_SIZE]; // For byte accessing
};
static union g_tx_fifo_u g_tx_fifo;
static int8_t g_last_set_tx_power_level = 0;
static uint8_t g_preamble_detected = 0;
static uint16_t g_channel = 0;
static uint32_t g_old_br_in_bps = RADIO_FSK_BR_50KBPS;
static int8_t g_rf_profile = EFR32XGXX_RAIL_50KBPS_IDX;
static bool g_platform_init_once = false;
static bool g_radio_init_once = false;
static bool g_tx_pwr_cfg_init_once = false;
static bool g_is_first_set_gfsk_mod_params = true;
static sid_pal_radio_events_t g_last_radio_event = SID_PAL_RADIO_EVENT_UNKNOWN;

// Provide weak overridable internal RX FIFO and RX Packet Buffer for
// non-UC protocols to share and for RAIL 2.x backwards compatibility.
static sl_rail_packet_queue_entry_t builtin_rx_packet_queue[SL_RAIL_BUILTIN_RX_PACKET_QUEUE_ENTRIES];
static SL_RAIL_DECLARE_FIFO_BUFFER(builtin_rx_fifo, SL_RAIL_BUILTIN_RX_FIFO_BYTES);
__WEAK sl_rail_packet_queue_entry_t * const sl_rail_builtin_rx_packet_queue_ptr = builtin_rx_packet_queue;
__WEAK sl_rail_fifo_buffer_align_t * const sl_rail_builtin_rx_fifo_ptr = builtin_rx_fifo;
__WEAK const uint16_t sl_rail_builtin_rx_packet_queue_entries = COMMON_UTILS_COUNTOF(builtin_rx_packet_queue);
__WEAK const uint16_t sl_rail_builtin_rx_fifo_bytes = sizeof(builtin_rx_fifo);

// Provide weak overridable internal RX FIFO and RX Packet Buffer for
// non-UC protocols to share and for RAIL 2.x backwards compatibility.
static SL_RAIL_DECLARE_FIFO_BUFFER(sli_tx_fifo_buffer_sidewalk, SL_RAIL_BUILTIN_TX_FIFO_BYTES);

static sl_rail_config_t rail_init_config;

#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
static uint16_t g_prev_channel = 0;
// For multiprotocol versions of RAIL, this can be used to control how a receive or transmit operation is run.
static sl_rail_scheduler_info_t g_schedulerInfo = {
  .priority = EFR32XGXX_TX_PRIORITY,
  .slip_time = 0,        // This value is not relevant as upper layer controlls direct radio operations thourgh the sid_pal start_rx/tx.
  .transaction_time = 0  // This value is not relevant as upper layer controlls direct radio operations thourgh the sid_pal start_rx/tx.
};
#endif

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
uint32_t compute_crc32(const uint8_t *buffer, uint16_t length)
{
  if (!buffer || !length) {
    return 0;
  }

  uint8_t temp_buffer[sizeof(uint32_t)] = { 0 };
  uint32_t crc32                        = 0xFFFFFFFF;
  const uint8_t *buffer_local;
  uint16_t length_local;

  if (length < sizeof(uint32_t)) {
    memcpy(temp_buffer, buffer, length);
    length_local = sizeof(uint32_t);
    buffer_local = temp_buffer;
  } else {
    length_local = length;
    buffer_local = buffer;
  }

  for (uint16_t index_buffer = 0; index_buffer < length_local; index_buffer++) {
    crc32 ^= (index_buffer < length) ? ((uint32_t)buffer_local[index_buffer] << 24) : 0x00000000;

    for (uint8_t i = 0; i < 8; i++) {
      crc32 = (crc32 & 0x80000000) ? (crc32 << 1) ^ POLYNOMIAL_CRC32 : (crc32 << 1);
    }
  }

  return ~crc32;
}

uint16_t compute_crc16(const uint8_t *buffer, uint16_t length)
{
  if (!buffer || !length) {
    return 0;
  }

  uint16_t crc16 = 0x0000;

  for (uint16_t index_buffer = 0; index_buffer < length; index_buffer++) {
    crc16 ^= ((uint16_t)buffer[index_buffer] << 8);

    for (uint8_t i = 0; i < 8; i++) {
      crc16 = (crc16 & 0x8000U) ? (crc16 << 1U) ^ POLYNOMIAL_CRC16 : (crc16 << 1U);
    }
  }

  return crc16;
}

void efr32xgxx_event_handler(void)
{
  const halo_drv_silabs_ctx_t *drv_ctx = efr32xgxx_get_drv_ctx();

  if (SID_PAL_RADIO_EVENT_UNKNOWN != g_last_radio_event) {
    drv_ctx->report_radio_event(g_last_radio_event);
  }
}

void efr32xgxx_radio_irq_process(void)
{
  efr32xgxx_event_handler();
}

uint16_t reverse16(uint16_t n)
{
  return (reverse8(n >> 8U) | (reverse8(n & 0xFFU) << 8U));
}

uint8_t reverse8(uint8_t n)
{
  n = ((0xF0U & n) >> 4U) | ((0x0FU & n) << 4U);
  n = ((0xCCU & n) >> 2U) | ((0x33U & n) << 2U);
  n = ((0xAAU & n) >> 1U) | ((0x55U & n) << 1U);
  return n;
}

uint16_t efr32xgxx_set_phr(uint8_t *hdr, uint8_t modesw, uint8_t crc, uint8_t whitening, uint16_t len)
{
  uint16_t tmp_len;
  uint8_t tmp_hdr;

  hdr[EFR32XGXX_PHR_LOW_BYTE] = hdr[EFR32XGXX_PHR_HIGH_BYTE] = 0;

  if (modesw == EFR32XGXX_PHR_ENABLE) {
    hdr[EFR32XGXX_PHR_LOW_BYTE] |= 0x1 << EFR32XGXX_PHR_FIELD_MODESW_SHIFT_BITS;
  }

  if (crc == EFR32XGXX_PHR_ENABLE) {
    hdr[EFR32XGXX_PHR_LOW_BYTE] |= (0x1 << EFR32XGXX_PHR_FIELD_CRC_SHIFT_BITS);
  }

  if (whitening == EFR32XGXX_PHR_ENABLE) {
    hdr[EFR32XGXX_PHR_LOW_BYTE] |= (0x1 << EFR32XGXX_PHR_FIELD_WHITENING_SHIFT_BITS);
  }

  tmp_len = reverse16(len);

  hdr[EFR32XGXX_PHR_LOW_BYTE] |= ((tmp_len << EFR32XGXX_PHR_FIELD_LEN_SHIFT_BITS)
                                  & EFR32XGXX_PHR_FIELD_LEN_HI_MASK);
  hdr[EFR32XGXX_PHR_HIGH_BYTE] = ((tmp_len >> EFR32XGXX_PHR_FIELD_LEN_SHIFT_BITS)
                                  & EFR32XGXX_PHR_FIELD_LEN_LO_MASK);

  tmp_hdr = reverse8(hdr[EFR32XGXX_PHR_LOW_BYTE]);

  hdr[EFR32XGXX_PHR_LOW_BYTE] = tmp_hdr;

  return len;
}

/**************************************************************************//**
 * Return RAIL handle object.
 *****************************************************************************/
sl_rail_handle_t efr32xgxx_get_railhandle(void)
{
  return g_rail_handle;
}

/**************************************************************************//**
 * Handle received packets.
 *****************************************************************************/
uint16_t efr32xgxx_get_rxpacket(uint8_t *phr, uint8_t *payload, int8_t *rssi, bool msb)
{
  sl_rail_rx_packet_info_t pktinfo;
  sl_rail_rx_packet_details_t pktDetails;
  sl_rail_rx_packet_handle_t pktHandle;
  uint16_t len = 0;
  sl_rail_status_t status;

  pktHandle = sl_rail_get_rx_packet_info(g_rail_handle, SL_RAIL_RX_PACKET_HANDLE_OLDEST, &pktinfo);
  if (!(pktHandle != SL_RAIL_RX_PACKET_HANDLE_INVALID)
      || ((pktinfo.packet_status != SL_RAIL_RX_PACKET_READY_SUCCESS)
          && (pktinfo.packet_status != SL_RAIL_RX_PACKET_READY_CRC_ERROR))) {
    SL_SID_LOG_PAL_ERROR("pal rail: unexpected RX pkt status: %d", pktinfo.packet_status);
    goto ret;
  }

  status = sl_rail_get_rx_packet_details(g_rail_handle, pktHandle, &pktDetails);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    SL_SID_LOG_PAL_ERROR("pal rail: get RX pkt detail err: %d", status);
    goto ret;
  }

  if (pktinfo.packet_bytes <= EFR32XGXX_MAX_PAYLOAD) {
    uint16_t peek_len = sl_rail_peek_rx_packet(g_rail_handle, pktHandle, payload, pktinfo.packet_bytes, 0);
    if (peek_len != pktinfo.packet_bytes) {
      SL_SID_LOG_PAL_ERROR("pal rail: RX pkt len not consistent: %d", peek_len);
      goto ret;
    }

    memcpy(phr, payload, EFR32XGXX_PHR_LENGTH);

    if (msb) {
      for (uint16_t i = 0; i < pktinfo.packet_bytes - EFR32XGXX_PHR_LENGTH; i++) {
        payload[i] = reverse8(payload[i + EFR32XGXX_PHR_LENGTH]);
      }
    } else {
      SL_SID_LOG_PAL_ERROR("pal rail: unsupported endianness");
      goto ret;
    }
  } else {
    SL_SID_LOG_PAL_ERROR("pal rail: RX pkt len more than supported: %d", pktinfo.packet_bytes);
    goto ret;
  }

  len = reverse16((phr[EFR32XGXX_PHR_LOW_BYTE] << EFR32XGXX_PHR_FIELD_LEN_SHIFT_BITS
                   | phr[EFR32XGXX_PHR_HIGH_BYTE]));
  len = ((len & EFR32XGXX_PHR_FIELD_LEN_HI_MASK) << EFR32XGXX_PHR_FIELD_LEN_SHIFT_BITS)
        | ((len >> EFR32XGXX_PHR_FIELD_LEN_SHIFT_BITS) & EFR32XGXX_PHR_FIELD_LEN_LO_MASK);
  *rssi = pktDetails.rssi_dbm;

  ret:
  return len;
}

/**************************************************************************//**
 * RAIL init.
 *****************************************************************************/
int32_t efr32xgxx_set_platform(void)
{
  int32_t err = RADIO_ERROR_NONE;
  sl_rail_status_t status;

  if (g_platform_init_once) {
    // Already initialized
    err = RADIO_ERROR_NONE;
    goto ret;
  }

  // Initialize the rail_init_config structure
  rail_init_config.events_callback = &radio_irq;
  rail_init_config.rx_packet_queue_entries = sl_rail_builtin_rx_packet_queue_entries;
  rail_init_config.rx_fifo_bytes = sl_rail_builtin_rx_fifo_bytes;
  rail_init_config.p_rx_packet_queue = sl_rail_builtin_rx_packet_queue_ptr;
  rail_init_config.p_rx_fifo_buffer = sl_rail_builtin_rx_fifo_ptr;
  rail_init_config.tx_fifo_bytes = SL_RAIL_BUILTIN_TX_FIFO_BYTES;
  rail_init_config.tx_fifo_init_bytes = 0U;
  rail_init_config.p_tx_fifo_buffer = sli_tx_fifo_buffer_sidewalk;

  // initializes the RAIL core
  status = sl_rail_init(&g_rail_handle, &rail_init_config, &efr32xgxx_rfready);

  if(SL_RAIL_STATUS_NO_ERROR != status) {
    SL_SID_LOG_PAL_ERROR("pal rail: sl_rail_init failed, return value: %ld", status);
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  } else if(NULL == g_rail_handle) {
    SL_SID_LOG_PAL_ERROR("pal rail: sl_rail_init failed, return value: %ld", status);
    // Handle is null, no reason to call sl_rail_idle
    return RADIO_ERROR_HARDWARE_ERROR;
  }

  while (!sl_rail_is_initialized(g_rail_handle)) {
    // Wait for RAIL init...
  }

  // Platform is initialized
  g_platform_init_once = true;

  ret:
  sl_rail_idle(g_rail_handle, SL_RAIL_IDLE, true);
  return err;
}

/**************************************************************************//**
 * Radio calibration.
 *****************************************************************************/
int32_t efr32xgxx_set_radio_init_hard(void)
{
  int32_t err = RADIO_ERROR_NONE;
  sl_rail_status_t status;

  if (g_radio_init_once) {
    // Already calibrated
    err = RADIO_ERROR_NONE;
    goto ret;
  }

  if (g_rf_profile == EFR32XGXX_RAIL_INVALID_IDX) {
    SL_SID_LOG_PAL_ERROR("pal rail: wrong RF profile: %d", g_rf_profile);
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  // Enable the PA calibration
  sl_rail_enable_pa_cal(g_rail_handle, true);

  // Initialize RAIL calibration
  status = sl_rail_config_cal(g_rail_handle, SL_RAIL_CAL_TEMP);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    SL_SID_LOG_PAL_ERROR("pal rail: calib cfg err: %d", status);
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  // Image Rejection Calibration (IRCAL)
  status = sl_rail_calibrate(g_rail_handle, NULL, SL_RAIL_CAL_ONETIME_IR_CAL);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    SL_SID_LOG_PAL_ERROR("pal rail: calib err: %d", status);
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  // Radio is initialized
  g_radio_init_once = true;

  ret:
  return err;
}

/**************************************************************************//**
 * Radio configuration.
 *****************************************************************************/
int32_t efr32xgxx_set_radio_init(void)
{
  int32_t err = RADIO_ERROR_NONE;
  sl_rail_status_t status;

  status = sl_rail_config_channels(g_rail_handle, efr32xgxx_channelConfigs[g_rf_profile], &radio_cfg_changed_hander);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    SL_SID_LOG_PAL_ERROR("pal rail: sl_rail_config_channels failed with error code %ld\n", status);
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }
  if(efr32xgxx_channelConfigs[g_rf_profile] != NULL)
  {
    uint16_t new_channel = sl_rail_get_first_channel(g_rail_handle, efr32xgxx_channelConfigs[g_rf_profile]);
    if(new_channel != SL_RAIL_CHANNEL_INVALID) {
      status = sl_rail_prepare_channel(g_rail_handle, new_channel);
      if (status != SL_RAIL_STATUS_NO_ERROR) {
        SL_SID_LOG_PAL_ERROR("pal rail: sl_rail_prepare_channel failed with error code %ld\n", status);
        err = RADIO_ERROR_HARDWARE_ERROR;
        goto ret;
      }
    } else {
      err = RADIO_ERROR_HARDWARE_ERROR;
      goto ret;
    }
  }

  err = efr32xgxx_set_radio_init_hard();
  if (err != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  sl_rail_events_t events = SL_RAIL_EVENT_CAL_NEEDED
                         | SL_RAIL_EVENT_RX_PACKET_RECEIVED
                         | SL_RAIL_EVENT_RX_FRAME_ERROR
                         | SL_RAIL_EVENT_RX_FIFO_OVERFLOW
                         | SL_RAIL_EVENT_RX_PACKET_ABORTED
                         | SL_RAIL_EVENT_TX_PACKET_SENT
                         | SL_RAIL_EVENT_TX_ABORTED
                         | SL_RAIL_EVENT_TX_BLOCKED
                         | SL_RAIL_EVENT_TX_UNDERFLOW
                         | SL_RAIL_EVENT_RX_PREAMBLE_DETECT;

  // Configure radio events
  status = sl_rail_config_events(g_rail_handle, SL_RAIL_EVENTS_ALL, events);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    SL_SID_LOG_PAL_ERROR("pal rail: evts cfg err: %d", status);
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  uint16_t allocated_tx_fifo_size = TX_FIFO_SIZE;
  status = sl_rail_set_tx_fifo(g_rail_handle, g_tx_fifo.aligned, allocated_tx_fifo_size, 0, 0);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    SL_SID_LOG_PAL_ERROR("pal rail: sl_rail_set_tx_fifo() failed with error code %lx\n", status);
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  status = sl_rail_init_power_manager();
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    SL_SID_LOG_PAL_ERROR("pal rail: pwr mngr init err: %d", status);
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  sl_rail_timer_sync_config_t timer_sync_config = SL_RAIL_TIMER_SYNC_DEFAULT;
  timer_sync_config.sleep = SL_RAIL_SLEEP_CONFIG_TIMERSYNC_ENABLED;

  status = sl_rail_config_sleep(g_rail_handle, &timer_sync_config);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    SL_SID_LOG_PAL_ERROR("pal rail: sl_rail_config_sleep err: %d", status);
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  ret:
  return err;
}

void efr32xgxx_get_txpower(int8_t *max_tx_power, int8_t *min_tx_power)
{
  sl_rail_status_t status;

  sl_rail_tx_power_t min_power_ddbm = 0;
  sl_rail_tx_power_t max_power_ddbm = 0;
  sl_rail_tx_power_t step_power_ddbm = 0;

  status = sl_rail_util_pa_get_tx_power_limits(g_rail_handle,
                                               SL_RAIL_TX_PA_MODE_SUB_GHZ,
                                               &min_power_ddbm,
                                               &max_power_ddbm,
                                               &step_power_ddbm);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    SL_SID_LOG_PAL_ERROR("pal rail: sl_rail_util_pa_get_tx_power_limits failed with error code %lx\n", status);
    return;
  }

  if (max_tx_power) {
    *max_tx_power = (int8_t) (max_power_ddbm / 10);
  }
  if (min_tx_power) {
    *min_tx_power = (int8_t) (min_power_ddbm / 10);
  }
}

int32_t efr32xgxx_set_txpower(int8_t power)
{
  sl_rail_status_t status;
  int32_t err = RADIO_ERROR_NONE;
  const halo_drv_silabs_ctx_t *drv_ctx = efr32xgxx_get_drv_ctx();

  if (!g_tx_pwr_cfg_init_once) {
    status = sl_rail_util_pa_post_init(g_rail_handle, SL_RAIL_TX_PA_MODE_SUB_GHZ);
    if (status != SL_RAIL_STATUS_NO_ERROR) {
      SL_SID_LOG_PAL_ERROR("pal rail: PA init err: %d", status);
      err = RADIO_ERROR_HARDWARE_ERROR;
      goto ret;
    }
  }

  if (power != g_last_set_tx_power_level) {
    int8_t max_pwr = 0;

    efr32xgxx_get_txpower(&max_pwr, NULL);
    if (power > max_pwr) {
      power = max_pwr;
    }

    sl_rail_tx_power_t powerLevelDeciDbm = (int16_t)power * 10; // convert from dBm to deci-dBm
    status = sl_rail_set_tx_power_dbm(g_rail_handle, powerLevelDeciDbm);
    if (status != SL_RAIL_STATUS_NO_ERROR) {
      SL_SID_LOG_PAL_ERROR("pal rail: set TX pwr err: %d", status);
      err = RADIO_ERROR_HARDWARE_ERROR;
      goto ret;
    }

    g_last_set_tx_power_level = power;
  }

  g_tx_pwr_cfg_init_once = true;

  ret:
  return err;
}

int32_t efr32xgxx_set_sleep(void)
{
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
  efr32xgxx_radio_yield();
#else
  efr32xgxx_set_radio_idle();
#endif
  return RADIO_ERROR_NONE;
}

int32_t efr32xgxx_set_standby(void)
{
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
  efr32xgxx_radio_yield();
#else
  efr32xgxx_set_radio_idle();
#endif
  return RADIO_ERROR_NONE;
}

int32_t efr32xgxx_set_tx_payload(const uint8_t *buffer, uint8_t size, bool msb)
{
  uint8_t data[EFR32XGXX_FSK_MAX_PAYLOAD_LENGTH] = { 0 };
  uint8_t *payload = data;
  int32_t err = RADIO_ERROR_NONE;

  if (msb) {
    memcpy(payload, buffer, EFR32XGXX_PHR_LENGTH);
    for (uint16_t i = EFR32XGXX_PHR_LENGTH; i < size; i++) {
      payload[i] = reverse8(buffer[i]);
    }
  } else {
    payload = (uint8_t *)buffer;
  }

  if (!sl_rail_write_tx_fifo(g_rail_handle, payload, size, true)) {
    SL_SID_LOG_PAL_ERROR("pal rail: write TX fifo err");
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  ret:
  return err;
}

int32_t efr32xgxx_set_tx(const uint32_t timeout)
{
  int32_t err = RADIO_ERROR_NONE;

#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
  efr32xgxx_cancel_radio_timer();
#else
  efr32xgxx_set_radio_idle();
#endif

  // Start sending
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
  g_schedulerInfo = (sl_rail_scheduler_info_t) { .priority = EFR32XGXX_TX_PRIORITY }; // TX priority value shall be less than TX
  sl_rail_status_t status = sl_rail_start_tx(g_rail_handle, g_channel, SL_RAIL_TX_OPTION_ALT_PREAMBLE_LEN, &g_schedulerInfo);
#else
  sl_rail_status_t status = sl_rail_start_tx(g_rail_handle, g_channel, SL_RAIL_TX_OPTION_ALT_PREAMBLE_LEN, NULL);
#endif
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    SL_SID_LOG_PAL_ERROR("pal rail: start sch TX err: %d", status);
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  if (timeout) {
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
    status = sl_rail_set_timer(g_rail_handle,
                           timeout + EFR32XGXX_RADIO_WARMUP_VALUE + EFR32XGXX_RADIO_PROTOCOL_SWITCH_TIME,
                           SL_RAIL_TIME_DELAY,
                           &efr32xgxx_tx_timer_expired);
#else
    status = sl_rail_set_timer(g_rail_handle, timeout + EFR32XGXX_RADIO_WARMUP_VALUE, SL_RAIL_TIME_DELAY, &efr32xgxx_tx_timer_expired);
#endif
    if (status != SL_RAIL_STATUS_NO_ERROR) {
      SL_SID_LOG_PAL_ERROR("pal rail: set tmr err: %d", status);
      err = RADIO_ERROR_HARDWARE_ERROR;
      goto ret;
    }
  }

  ret:
  return err;
}

int32_t efr32xgxx_set_rx(const uint32_t timeout)
{
  // timeout 0 is continuous receive
  int32_t err = RADIO_ERROR_NONE;
  uint32_t rail_timeout = timeout;
  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;

  g_preamble_detected = 0;

#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
  // Check if channel has changed
  // If we call sl_rail_start_rx while not idle but with a different channel, any ongoing receive or transmit operation will be aborted
  if (g_prev_channel != g_channel) {
    efr32xgxx_set_radio_idle();
  } else {
    efr32xgxx_cancel_radio_timer();
  }
#else
  efr32xgxx_set_radio_idle();
#endif

#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
  // Start backround RX
  g_schedulerInfo = (sl_rail_scheduler_info_t) { .priority = EFR32XGXX_RX_PRIORITY }; // RX priority value shall be greater than TX
  status = sl_rail_start_rx(g_rail_handle, g_channel, &g_schedulerInfo);
#else
  // Start RX
  status = sl_rail_start_rx(g_rail_handle, g_channel, NULL);
#endif
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    SL_SID_LOG_PAL_ERROR("pal rail: schedule RX err: %d", status);
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
  // Update prev channel
  g_prev_channel = g_channel;
#endif

  if (rail_timeout) {
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
    status = sl_rail_set_timer(g_rail_handle,
                               rail_timeout + EFR32XGXX_RADIO_WARMUP_VALUE + EFR32XGXX_RADIO_PROTOCOL_SWITCH_TIME,
                               SL_RAIL_TIME_DELAY,
                               &efr32xgxx_rx_timer_expired);
#else
    status = sl_rail_set_timer(g_rail_handle, rail_timeout + EFR32XGXX_RADIO_WARMUP_VALUE, SL_RAIL_TIME_DELAY, &efr32xgxx_rx_timer_expired);
#endif
    if (status != SL_RAIL_STATUS_NO_ERROR) {
      SL_SID_LOG_PAL_ERROR("pal rail: set tmr err: %d", status);
      err = RADIO_ERROR_HARDWARE_ERROR;
      goto ret;
    }
  }

  ret:
  return err;
}

// This function is not supported.
int32_t efr32xgxx_set_tx_cw(void)
{
  return RADIO_ERROR_NOT_SUPPORTED;
}

// This function is not supported.
int32_t efr32xgxx_set_tx_cpbl(void)
{
  return RADIO_ERROR_NOT_SUPPORTED;
}

int32_t efr32xgxx_set_rf_freq(const uint32_t freq_in_hz)
{
  // Compute channel from frequency
  g_channel = (uint16_t)((freq_in_hz - efr32xgxx_channelConfigs[g_rf_profile]->p_entries->base_frequency_hz) / efr32xgxx_channelConfigs[g_rf_profile]->p_entries->channel_spacing_hz);

  return RADIO_ERROR_NONE;
}

int32_t efr32xgxx_set_gfsk_mod_params(const efr32xgxx_mod_params_gfsk_t *params)
{
  int32_t err = RADIO_ERROR_NONE;

  if (g_is_first_set_gfsk_mod_params) {
    g_is_first_set_gfsk_mod_params = false;
    goto ret;
  }

  if (params->br_in_bps != g_old_br_in_bps) {
    if (params->br_in_bps == RADIO_FSK_BR_50KBPS) {
      g_rf_profile = EFR32XGXX_RAIL_50KBPS_IDX;
    } else if (params->br_in_bps == RADIO_FSK_BR_150KBPS) {
      g_rf_profile = EFR32XGXX_RAIL_150KBPS_IDX;
    } else if (params->br_in_bps == RADIO_FSK_BR_250KBPS) {
      g_rf_profile = EFR32XGXX_RAIL_250KBPS_IDX;
    } else {
      SL_SID_LOG_PAL_ERROR("pal rail: wrong RF profile: %d", g_rf_profile);
      g_rf_profile = EFR32XGXX_RAIL_INVALID_IDX;
      err = RADIO_ERROR_HARDWARE_ERROR;
      goto ret;
    }
  }

  if (efr32xgxx_set_standby() != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  if (efr32xgxx_set_radio_init() != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  g_old_br_in_bps = params->br_in_bps;

  ret:
  return err;
}

int32_t efr32xgxx_set_gfsk_pkt_params(const efr32xgxx_pkt_params_gfsk_t *params)
{
  int32_t err = RADIO_ERROR_NONE;
  sl_rail_status_t status;

  status = sl_rail_set_tx_alt_preamble_length(g_rail_handle, params->pbl_len_in_bits);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    SL_SID_LOG_PAL_ERROR("pal rail: preamble set err: %d (bits: %d)", status, params->pbl_len_in_bits);
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  ret:
  return err;
}

int32_t efr32xgxx_get_rssi_inst(int16_t *rssi_in_dbm)
{
  int16_t rssi_local;
  int32_t err = RADIO_ERROR_NONE;
  sl_rail_time_t timeout;

  // 'wait' should never be set 'true' in multiprotocol as the wait time is not consistent,
  // so scheduling a scheduler slot cannot be done accurately.
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
  timeout = SL_RAIL_GET_RSSI_NO_WAIT;
#else
  timeout = SL_RAIL_GET_RSSI_WAIT_WITHOUT_TIMEOUT;
#endif

  if ((rssi_local = sl_rail_get_rssi(g_rail_handle, timeout)) == SL_RAIL_RSSI_INVALID) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }
  *rssi_in_dbm = (rssi_local >> RSSI_QUARTER_ORDER);

  ret:
  return err;
}

uint32_t efr32xgxx_get_gfsk_time_on_air_numerator(const efr32xgxx_pkt_params_gfsk_t *pkt_p)
{
  return pkt_p->pbl_len_in_bits
         + (pkt_p->hdr_type == EFR32XGXX_GFSK_PKT_VAR_LEN ? 8 : 0)
         + pkt_p->sync_word_len_in_bits
         + ((pkt_p->pld_len_in_bytes
             + (pkt_p->addr_cmp == EFR32XGXX_GFSK_ADDR_CMP_FILT_OFF ? 0 : 1)
             + efr32xgxx_get_gfsk_crc_len_in_bytes(pkt_p->crc_type)) << 3);
}

uint32_t efr32xgxx_get_gfsk_time_on_air_in_ms(const efr32xgxx_pkt_params_gfsk_t *pkt_p,
                                              const efr32xgxx_mod_params_gfsk_t *mod_p)
{
  uint32_t numerator   = MS_IN_SEC * efr32xgxx_get_gfsk_time_on_air_numerator(pkt_p);
  uint32_t denominator = mod_p->br_in_bps;

  // Perform integral ceil()
  return (numerator + denominator - 1) / denominator;
}

int32_t efr32xgxx_get_random_numbers(uint32_t *numbers, unsigned int n)
{
  int8_t seed = 0;
  int8_t cnt = 0;
  int8_t err_cnt = 0;
  int16_t rssi;
  uint32_t noise = 0;
  uint32_t r_val = 0;
  int32_t err = RADIO_ERROR_NONE;

  if (n < 1) {
    n = 1;
  }

  do {
    sid_pal_delay_us(n);
    if (((rssi = sid_pal_radio_rssi()) == 0) || (rssi == INT16_MAX)) {
      if (err_cnt++ > RSSI_ERROR_COUNT) {
        err = RADIO_ERROR_HARDWARE_ERROR;
        goto ret;
      }
      continue;
    }

    noise += (-1 * rssi);
    seed = (noise % 8);
    r_val |= (((noise % 16) & 0xf) << (((seed + cnt) % 8) * 4));
    r_val |= noise;
    cnt++;
  } while (RF_RANDOM_TIMES > cnt);
  *numbers = r_val;

  ret:
  return err;
}

int32_t efr32xgxx_set_gfsk_sync_word(const uint8_t* sync_word, const uint8_t sync_word_len)
{
  (void)sync_word;
  (void)sync_word_len;
  return RADIO_ERROR_NONE;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static uint32_t efr32xgxx_get_gfsk_crc_len_in_bytes(efr32xgxx_gfsk_crc_types_t crc_type)
{
  switch (crc_type) {
    case EFR32XGXX_GFSK_CRC_OFF:
      return EFR32XGXX_CRC_BYTES_ZERO;
    case EFR32XGXX_GFSK_CRC_1_BYTE:
    case EFR32XGXX_GFSK_CRC_1_BYTE_INV:
      return EFR32XGXX_CRC_BYTES_ONE;
    case EFR32XGXX_GFSK_CRC_2_BYTES:
    case EFR32XGXX_GFSK_CRC_2_BYTES_INV:
      return EFR32XGXX_CRC_BYTES_TWO;
  }

  return EFR32XGXX_CRC_BYTES_ZERO;
}

static void radio_cfg_changed_hander(sl_rail_handle_t rail_handle, const sl_rail_channel_config_entry_t *entry)
{
  (void)rail_handle;
  (void)entry;

  efr32xgxx_set_txpower(g_last_set_tx_power_level);
}

static void efr32xgxx_set_radio_idle(void)
{
  efr32xgxx_cancel_radio_timer();
  sl_rail_idle(g_rail_handle, SL_RAIL_IDLE, true);
}

#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
static void efr32xgxx_radio_yield(void)
{
  efr32xgxx_cancel_radio_timer();
  sl_rail_yield_radio(g_rail_handle);
}
#endif

static void efr32xgxx_cancel_radio_timer(void)
{
  if (sl_rail_is_timer_running(g_rail_handle)) {
    sl_rail_cancel_timer(g_rail_handle);
  }
}

/**************************************************************************//**
 * A callback that notifies the application
 * when the radio is finished initializing
 * and is ready for further configuration.
 *****************************************************************************/
static void efr32xgxx_rfready(sl_rail_handle_t rail_handle)
{
  (void)rail_handle;
}

static void efr32xgxx_event_notify(sid_pal_radio_events_t radio_event)
{
  const halo_drv_silabs_ctx_t *drv_ctx = efr32xgxx_get_drv_ctx();

  g_last_radio_event = radio_event;

  drv_ctx->irq_handler();
}

// to mimic semtech behaviour
static void radio_irq(sl_rail_handle_t rail_handle, sl_rail_events_t events)
{
  sl_rail_status_t status;

  //----------------- RX --------------------------
  // Handle RX Events
  if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
    halo_drv_silabs_ctx_t *drv_ctx = efr32xgxx_get_drv_ctx();

    sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &drv_ctx->radio_rx_packet->rcv_tm, NULL);
    if (radio_fsk_process_rx_done(drv_ctx) == RADIO_ERROR_NONE) {
      memset(&drv_ctx->radio_rx_packet->lora_rx_packet_status, 0, sizeof(sid_pal_radio_lora_rx_packet_status_t));
      efr32xgxx_event_notify(SID_PAL_RADIO_EVENT_RX_DONE);
    } else {
      SL_SID_LOG_PAL_ERROR("pal rail: pkt rcv err");
      efr32xgxx_event_notify(SID_PAL_RADIO_EVENT_RX_ERROR);
    }

    efr32xgxx_set_radio_idle();
  } else if (events & SL_RAIL_EVENT_RX_PREAMBLE_DETECT) {
    halo_drv_silabs_ctx_t *drv_ctx = efr32xgxx_get_drv_ctx();

    if (drv_ctx->cad_exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_LBT) {
      // listen before talk has detected activity on current channel
      // report this event to stack
      drv_ctx->radio_rx_packet->fsk_rx_packet_status.rssi_sync = (int8_t)sid_pal_radio_rssi();
      drv_ctx->cad_exit_mode = SID_PAL_RADIO_CAD_EXIT_MODE_NONE;
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
      efr32xgxx_set_radio_idle();
#endif
      efr32xgxx_event_notify(SID_PAL_RADIO_EVENT_CS_DONE);
    }

    g_preamble_detected = 1;
  } else if (events & (SL_RAIL_EVENT_RX_FIFO_OVERFLOW | SL_RAIL_EVENT_RX_PACKET_ABORTED | SL_RAIL_EVENT_RX_FRAME_ERROR)) {
    if (events & SL_RAIL_EVENT_RX_FIFO_OVERFLOW) {
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
      efr32xgxx_set_radio_idle();
#endif
      SL_SID_LOG_PAL_ERROR("pal rail: RX fifo overflow");
      efr32xgxx_event_notify(SID_PAL_RADIO_EVENT_RX_ERROR);
    }

    if (events & SL_RAIL_EVENT_RX_PACKET_ABORTED) {
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
      efr32xgxx_set_radio_idle();
#endif
      SL_SID_LOG_PAL_ERROR("pal rail: RX pkt abort");
      efr32xgxx_event_notify(SID_PAL_RADIO_EVENT_RX_ERROR);
    }

    if (events & SL_RAIL_EVENT_RX_FRAME_ERROR) {
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
      efr32xgxx_set_radio_idle();
#endif
      SL_SID_LOG_PAL_ERROR("pal rail: RX frame err");
      efr32xgxx_event_notify(SID_PAL_RADIO_EVENT_RX_ERROR);
    }
  }

  //----------------- TX --------------------------
  // Handle TX Events
  if (events & (SL_RAIL_EVENT_TX_ABORTED | SL_RAIL_EVENT_TX_BLOCKED | SL_RAIL_EVENT_TX_UNDERFLOW)) {
    SL_SID_LOG_PAL_ERROR("pal rail: TX err");
    efr32xgxx_event_notify(SID_PAL_RADIO_EVENT_TX_TIMEOUT);
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
    efr32xgxx_radio_yield();
#else
    efr32xgxx_set_radio_idle();
#endif
  } else if (events & SL_RAIL_EVENT_TX_PACKET_SENT) {
    efr32xgxx_event_notify(SID_PAL_RADIO_EVENT_TX_DONE);
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
    efr32xgxx_radio_yield();
#else
    efr32xgxx_set_radio_idle();
#endif
  }

  // Perform temperature calibration when needed
  if (events & SL_RAIL_EVENT_CAL_NEEDED) {
    sl_rail_cal_mask_t pending_calib = sl_rail_get_pending_cal(rail_handle);
    if (pending_calib & SL_RAIL_CAL_TEMP) {
      status = sl_rail_calibrate_temp(rail_handle);
      if (status != SL_RAIL_STATUS_NO_ERROR) {
        SL_SID_LOG_PAL_ERROR("pal rail: temp calib err: %d", status);
      }
    }
  }
}

static void efr32xgxx_tx_timer_expired(sl_rail_handle_t rail_handle)
{
  (void)rail_handle;
  efr32xgxx_event_notify(SID_PAL_RADIO_EVENT_TX_TIMEOUT);
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
  efr32xgxx_radio_yield();
#else
  efr32xgxx_set_radio_idle();
#endif
}

static void efr32xgxx_rx_timer_expired(sl_rail_handle_t rail_handle)
{
  (void)rail_handle;
  halo_drv_silabs_ctx_t *drv_ctx = efr32xgxx_get_drv_ctx();

  if (!g_preamble_detected) {
    if (drv_ctx->cad_exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_LBT) {
      // rx window was a listen before talk
      // channel is free so we can schedule tx
      drv_ctx->cad_exit_mode = SID_PAL_RADIO_CAD_EXIT_MODE_NONE;
      efr32xgxx_event_notify(SID_PAL_RADIO_EVENT_UNKNOWN);
      sid_pal_radio_start_tx(0);
      return;
    }
    // for standard rx windows, report timeout event
    efr32xgxx_event_notify(SID_PAL_RADIO_EVENT_RX_TIMEOUT);
    // switch radio mode
    efr32xgxx_set_radio_idle();
  } else {
    // preamble have been received since start of the rx window
    // continue rx until SL_RAIL_EVENT_RX_PACKET_RECEIVED event
  }
}

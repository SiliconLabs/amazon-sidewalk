/***************************************************************************//**
 * @file
 * @brief efr32xgxx_radio.c
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
#include <sid_pal_delay_ifc.h>
#include <sid_pal_log_ifc.h>
#include <sid_pal_assert_ifc.h>
#include <sid_clock_ifc.h>
#include <sid_time_ops.h>
#include <sid_time_types.h>
#include "efr32xgxx_radio.h"

#include <stdio.h>
extern void efr32xgxx_radio_irq_process(void);

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define EFR32XGXX_RADIO_NOISE_SAMPLE_SIZE     (32)
#define EFR32XGXX_MIN_CHANNEL_FREE_DELAY_US   (1)
#define EFR32XGXX_MIN_CHANNEL_NOISE_DELAY_US  (30)
#define SIDEWALK_FSK_US_START_FREQUENCY       (902200000)
#define SIDEWALK_FSK_US_END_FREQUENCY         (916000000)
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static int32_t radio_efr32xgxx_platform_init(void);
static int32_t sid_pal_radio_get_tx_power_range(int8_t *max_tx_power, int8_t *min_tx_power);
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
static int8_t g_tx_power = 0;

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
static halo_drv_silabs_ctx_t drv_ctx = { 0 };

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
halo_drv_silabs_ctx_t *efr32xgxx_get_drv_ctx(void)
{
  return &drv_ctx;
}

void set_radio_efr32xgxx_device_config(const radio_efr32xgxx_device_config_t *cfg)
{
  SID_PAL_ASSERT(cfg);
  drv_ctx.config = cfg;
}

uint8_t sid_pal_radio_get_status(void)
{
  return drv_ctx.radio_state;
}

sid_pal_radio_modem_mode_t sid_pal_radio_get_modem_mode(void)
{
  return SID_PAL_RADIO_MODEM_MODE_FSK;
}

int32_t sid_pal_radio_set_modem_mode(sid_pal_radio_modem_mode_t mode)
{
  int32_t err = RADIO_ERROR_NONE;

  if (mode != SID_PAL_RADIO_MODEM_MODE_FSK) {
    err = RADIO_ERROR_NOT_SUPPORTED;
    goto ret;
  }

  drv_ctx.modem = SID_PAL_RADIO_MODEM_MODE_FSK;

  ret:
  return err;
}

int32_t sid_pal_radio_irq_process(void)
{
  efr32xgxx_radio_irq_process();

  return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_frequency(uint32_t freq)
{
  int32_t err = RADIO_ERROR_NONE;
  if(drv_ctx.regional_radio_param.param_region == RADIO_REGION_NA) {
    if((freq < SIDEWALK_FSK_US_START_FREQUENCY) || (freq > SIDEWALK_FSK_US_END_FREQUENCY)) {
      //invalid frequency
      return RADIO_ERROR_INVALID_PARAMS;
    }
  }

  if (efr32xgxx_set_rf_freq(freq) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  drv_ctx.radio_freq_hz = freq;

  ret:
  return err;
}

int32_t sid_pal_radio_get_max_tx_power(sid_pal_radio_data_rate_t data_rate, int8_t *tx_power)
{
  int32_t err = RADIO_ERROR_NONE;
  int8_t radio_max = 0 , radio_min = 0;

  if (data_rate < SID_PAL_RADIO_DATA_RATE_50KBPS || data_rate > SID_PAL_RADIO_DATA_RATE_250KBPS) {
    err = RADIO_ERROR_INVALID_PARAMS;
    goto ret;
  }
  //get capable power range of the radio
  if (sid_pal_radio_get_tx_power_range(&radio_max, &radio_min) != RADIO_ERROR_NONE) {
     err = RADIO_ERROR_HARDWARE_ERROR;
     goto ret;
  }
  (void) radio_min;
  *tx_power = radio_max;

  int8_t region_max = drv_ctx.regional_radio_param.max_tx_power[data_rate - 1];
  //region param is below the max capacity of radio
  if(region_max < radio_max) {
    *tx_power = region_max;
  }

  ret:
  return err;
}

int32_t sid_pal_radio_set_tx_power(int8_t power)
{
  int8_t max = 0, min = 0;
  int32_t err = RADIO_ERROR_NONE;

  if(sid_pal_radio_get_tx_power_range(&max, &min) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }
  if (power < min) {
    SID_PAL_LOG_WARNING("tx power config is below of the allowed range [%d, %d] configured: %d dBm. Value will be capped.", min, max, power);
    power = min;
  }
  if (power > max) {
    SID_PAL_LOG_WARNING("tx power config is above of the allowed range [%d, %d] configured: %d dBm. Value will be capped.", min, max, power);
    power = max;
  }
  if (power != g_tx_power) {
      if (efr32xgxx_set_txpower(power) != RADIO_ERROR_NONE) {
        err = RADIO_ERROR_HARDWARE_ERROR;
        goto ret;
      }
      g_tx_power = power;
  }

 ret:
  return err;
}

/* TODO: SIDEWALK-889
 * implement sleep_ms period
*/
int32_t sid_pal_radio_sleep(uint32_t sleep_ms)
{
  (void)sleep_ms;

  int32_t err = RADIO_ERROR_NONE;

  if (drv_ctx.radio_state == SID_PAL_RADIO_SLEEP) {
    err = RADIO_ERROR_NONE;
    goto ret;
  }

  if (efr32xgxx_set_sleep() != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  drv_ctx.radio_state = SID_PAL_RADIO_SLEEP;

  ret:
  return err;
}

int32_t sid_pal_radio_standby(void)
{
  int32_t err = RADIO_ERROR_NONE;

  if (drv_ctx.radio_state == SID_PAL_RADIO_STANDBY) {
    err = RADIO_ERROR_NONE;
    goto ret;
  }

  if (efr32xgxx_set_standby() != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  drv_ctx.radio_state = SID_PAL_RADIO_STANDBY;

  ret:
  return err;
}

int32_t sid_pal_radio_set_tx_payload(const uint8_t *buffer, uint8_t size)
{
  int32_t err = RADIO_ERROR_NONE;

  if (buffer == NULL || size == 0) {
    err = RADIO_ERROR_INVALID_PARAMS;
    goto ret;
  }

  // Silabs uses LSB, convert payload data to MSB order
  if (efr32xgxx_set_tx_payload(buffer, size, PAYLOAD_IS_MSB) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_IO_ERROR;
  }

  ret:
  return err;
}

int32_t sid_pal_radio_start_tx(uint32_t timeout)
{
  int32_t err = RADIO_ERROR_NONE;

  if (efr32xgxx_set_tx(timeout) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  drv_ctx.radio_state = SID_PAL_RADIO_TX;

  ret:
  return err;
}

// Not used by Silabs
int32_t sid_pal_radio_set_tx_continuous_wave(uint32_t freq, int8_t power)
{
  (void)freq;
  (void)power;
  return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_start_rx(uint32_t timeout)
{
  int32_t err = RADIO_ERROR_NONE;

  if (efr32xgxx_set_rx(timeout) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  drv_ctx.radio_state = SID_PAL_RADIO_RX;

  ret:
  return err;
}

int32_t sid_pal_radio_start_carrier_sense(uint32_t timeout, sid_pal_radio_cad_param_exit_mode_t exit_mode)
{
  int32_t err = RADIO_ERROR_NONE;

  if (efr32xgxx_set_rx(timeout) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  drv_ctx.radio_state = SID_PAL_RADIO_RX;
  drv_ctx.cad_exit_mode = exit_mode;

  ret:
  return err;
}

int32_t sid_pal_radio_start_continuous_rx(void)
{
  return sid_pal_radio_start_rx(0);
}

// Not used by Silabs
int32_t sid_pal_radio_set_rx_duty_cycle(uint32_t rx_time, uint32_t sleep_time)
{
  (void)rx_time;
  (void)sleep_time;
  return RADIO_ERROR_NOT_SUPPORTED;
}

int16_t sid_pal_radio_rssi(void)
{
  int16_t rssi;

  if (efr32xgxx_get_rssi_inst(&rssi) != RADIO_ERROR_NONE) {
    SID_PAL_LOG_ERROR("could not get rssi");
    rssi = INT16_MAX;
  }

  return rssi;
}

int32_t sid_pal_radio_is_channel_free(uint32_t freq, int16_t threshold, uint32_t delay_us, bool *is_channel_free)
{
  int32_t err = RADIO_ERROR_NONE;
  int16_t rssi;

  *is_channel_free = false;

  if ((err = sid_pal_radio_set_frequency(freq)) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  if ((err = sid_pal_radio_start_continuous_rx()) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  if (delay_us < EFR32XGXX_MIN_CHANNEL_FREE_DELAY_US) {
    SID_PAL_LOG_WARNING("min ch free delay raised (req: %ld, min: %ld)", delay_us, EFR32XGXX_MIN_CHANNEL_FREE_DELAY_US);
    delay_us = EFR32XGXX_MIN_CHANNEL_FREE_DELAY_US;
  }

  struct sid_timespec t_start, t_cur, t_threshold;

  t_threshold.tv_sec = delay_us / SID_TIME_USEC_PER_SEC;
  t_threshold.tv_nsec = (delay_us % SID_TIME_USEC_PER_SEC) * SID_TIME_NSEC_PER_USEC;
  t_threshold.tv_nsec += ((delay_us % 1000) * 1000);

  if (sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &t_start, NULL) != SID_ERROR_NONE) {
    err = RADIO_ERROR_GENERIC;
    goto ret;
  }

  do {
    sid_pal_delay_us(EFR32XGXX_MIN_CHANNEL_FREE_DELAY_US);
    rssi = sid_pal_radio_rssi();
    if (sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &t_cur, NULL) != SID_ERROR_NONE) {
      err = RADIO_ERROR_GENERIC;
      goto ret;
    }
    sid_time_sub(&t_cur, &t_start);
    if (rssi > threshold) {
      goto ret;
    }
    // The minimum time needed in between measurements is about 300 micro secs.
  } while (sid_time_gt(&t_threshold, &t_cur));

  *is_channel_free = true;

  if ((err = sid_pal_radio_standby()) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  ret:
  return err;
}

int32_t sid_pal_radio_random(uint32_t *random)
{
  int32_t err = RADIO_ERROR_NONE;

  *random = UINT32_MAX;

  if ((err = sid_pal_radio_start_continuous_rx()) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  if (efr32xgxx_get_random_numbers(random, 1) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  if ((err = sid_pal_radio_standby()) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  ret:
  return err;
}

int16_t sid_pal_radio_get_ant_dbi(void)
{
  return drv_ctx.regional_radio_param.ant_dbi;
}

int32_t sid_pal_radio_get_cca_level_adjust(sid_pal_radio_data_rate_t data_rate, int8_t *adj_level)
{
  int32_t err = RADIO_ERROR_NONE;

  if (data_rate < SID_PAL_RADIO_DATA_RATE_50KBPS || data_rate > SID_PAL_RADIO_DATA_RATE_250KBPS) {
    err = RADIO_ERROR_INVALID_PARAMS;
    goto ret;
  }

  *adj_level = drv_ctx.regional_radio_param.cca_level_adjust[data_rate - 1];

  ret:
  return err;
}

int32_t sid_pal_radio_get_chan_noise(uint32_t freq, int16_t *noise)
{
  int32_t err = RADIO_ERROR_NONE;
  int16_t rssi = 0;

  if ((err = sid_pal_radio_set_frequency(freq)) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  if ((err = sid_pal_radio_start_continuous_rx()) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  *noise = 0;

  for (uint8_t i = 0; i < EFR32XGXX_RADIO_NOISE_SAMPLE_SIZE; i++) {
    // Try to acquire valid rssi value
    sid_pal_delay_us(EFR32XGXX_MIN_CHANNEL_NOISE_DELAY_US);
    rssi = sid_pal_radio_rssi();
    if(rssi == INT16_MAX) {
      // invalid acquisition
      err = RADIO_ERROR_HARDWARE_ERROR;
      goto ret;
    }
      *noise += rssi;
  }

  *noise /= EFR32XGXX_RADIO_NOISE_SAMPLE_SIZE;

  if ((err = sid_pal_radio_standby()) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

ret:
  return err;
}

int32_t sid_pal_radio_set_region(sid_pal_radio_region_code_t region)
{
  int32_t err = RADIO_ERROR_NOT_SUPPORTED;

  if (region <= SID_PAL_RADIO_RC_NONE || region >= SID_PAL_RADIO_RC_MAX) {
    err = RADIO_ERROR_INVALID_PARAMS;
    goto ret;
  }

  for (uint8_t i = 0; i < drv_ctx.config->regional_config.reg_param_table_size; i++) {
    if (region == drv_ctx.config->regional_config.reg_param_table[i].param_region) {
      drv_ctx.regional_radio_param = drv_ctx.config->regional_config.reg_param_table[i];
      err = RADIO_ERROR_NONE;
      break;
    }
  }

  ret:
  return err;
}

int32_t sid_pal_radio_get_radio_state_transition_delays(sid_pal_radio_state_transition_timings_t *state_delay)
{
  *state_delay = drv_ctx.config->state_timings;
  return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_init(sid_pal_radio_event_notify_t notify, sid_pal_radio_irq_handler_t dio_irq_handler, sid_pal_radio_rx_packet_t *rx_packet)
{
  int32_t err = RADIO_ERROR_NONE;
  int8_t tx_power;

  if (notify == NULL || rx_packet == NULL) {
    err = RADIO_ERROR_INVALID_PARAMS;
    goto ret;
  }

  drv_ctx.radio_rx_packet = rx_packet;
  drv_ctx.report_radio_event = notify;
  drv_ctx.irq_handler = dio_irq_handler;   // to mimic semtech behaviour
  drv_ctx.modem = SID_PAL_RADIO_MODEM_MODE_FSK;

  if ((err = sid_pal_radio_set_region(drv_ctx.config->regional_config.radio_region)) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  if ((err = radio_efr32xgxx_platform_init()) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  drv_ctx.radio_state = SID_PAL_RADIO_UNKNOWN;
  if ((err = sid_pal_radio_standby()) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  if ((err = sid_pal_radio_get_max_tx_power(SID_PAL_RADIO_DATA_RATE_50KBPS, &tx_power)) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  if ((err = sid_pal_radio_set_tx_power(tx_power)) != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  ret:
  return err;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static int32_t radio_efr32xgxx_platform_init(void)
{
  int32_t err = RADIO_ERROR_NONE;

  if (efr32xgxx_set_platform() != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  if (efr32xgxx_set_radio_init() != RADIO_ERROR_NONE) {
    err = RADIO_ERROR_HARDWARE_ERROR;
    goto ret;
  }

  ret:
  return err;
}

static int32_t sid_pal_radio_get_tx_power_range(int8_t *max_tx_power, int8_t *min_tx_power)
{
  efr32xgxx_get_txpower(max_tx_power, min_tx_power);

  return RADIO_ERROR_NONE;
}
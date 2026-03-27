/***************************************************************************//**
 * @file
 * @brief app_subghz_config.c
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
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <sid_pal_serial_bus_efr32_spi_config.h>
#include <sx126x_config.h>
#include <gpio.h>
#include "app_subghz_config.h"
#include <mfg_store_app_values.h>
#include <sid_900_cfg.h>
#include <sx126x.h>
#include <sx126x_radio.h>
#include "sl_sidewalk_log_pal.h"

#include <sl_spidrv_exp_config.h>
#define SL_SPI_PERIPHERAL_ID         SL_SPIDRV_EXP_PERIPHERAL
#define SL_SPI_PERIPHERAL_BITRATE    SL_SPIDRV_EXP_BITRATE
#define SL_SPI_PERIPHERAL_CLOCK_MODE SL_SPIDRV_EXP_CLOCK_MODE

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/* This product has no external PA and SX1262 can support max of 22dBm*/
#define RADIO_SX1262_MAX_TX_POWER       22
#define RADIO_SX1262_MIN_TX_POWER       -9

/* same tx power applies for all data rates*/
#define RADIO_MAX_TX_POWER_NA           20
#define RADIO_MAX_TX_POWER_EU           14

#ifndef REGION_US915
    #define REGION_US915
#endif

#if defined (REGION_ALL)
#define RADIO_REGION                    RADIO_REGION_NONE
#elif defined (REGION_US915)
#define RADIO_REGION                    RADIO_REGION_NA
#elif defined (REGION_EU868)
#define RADIO_REGION                    RADIO_REGION_EU
#endif

#define RADIO_SX1262_SPI_BUFFER_SIZE    255

#define RADIO_SX1262_PA_DUTY_CYCLE      0x04
#define RADIO_SX1262_HP_MAX             0x07
#define RADIO_SX1262_DEVICE_SEL         0x00
#define RADIO_SX1262_PA_LUT             0x01

#define RADIO_RX_LNA_GAIN               0
#define RADIO_MAX_CAD_SYMBOL            SID_PAL_RADIO_LORA_CAD_04_SYMBOL
#define RADIO_ANT_GAIN(X)               ((X) * 100)

#define SAR_DCR_CONFIG                  (100)

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

static int32_t radio_sx1262_pa_cfg(int8_t tx_power, radio_sx126x_pa_cfg_t *pa_cfg);
static int32_t radio_sx1262_get_trim_cap_val(uint16_t *trim);
#if (MODULE_KG100S == 2)
static int32_t radio_dio3_ctrl_voltage(uint8_t radio_state);
static int32_t radio_band_sel_ctrl(bool tx_en);
#endif
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static const radio_sx126x_regional_param_t radio_sx126x_regional_param[] =
{
    #if defined (REGION_ALL) || defined (REGION_US915)
  {
    .param_region = RADIO_REGION_NA,
    .max_tx_power = { RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA,
                      RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA },
    .cca_level_adjust = { 0, 0, 0, 0, 0, 0 },
    .ant_dbi = (int16_t)RADIO_ANT_GAIN(2.15)
  },
    #endif
    #if defined (REGION_ALL) || defined (REGION_EU868)
  {
    .param_region = RADIO_REGION_EU,
    .max_tx_power = { RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU,
                      RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU },
    .cca_level_adjust = { 0, 0, 0, 0, 0, 0 },
    .ant_dbi = (int16_t)RADIO_ANT_GAIN(2.15)
  },
    #endif
};

static const struct sid_pal_serial_bus_efr32_spi_config radio_spi_config =
{
  .peripheral_id = SL_SPI_PERIPHERAL_ID,
};

static const struct sid_pal_serial_bus_factory radio_spi_factory =
{
  .create  = sid_pal_serial_bus_efr32_spi_create,
  .config  = &radio_spi_config,
};

static uint8_t radio_sx1262_buffer[RADIO_SX1262_SPI_BUFFER_SIZE] = { 0 };

const radio_sx126x_device_config_t radio_sx1262_cfg = {
  .id                         = SEMTECH_ID_SX1262,   // chip id register not supported
  .regulator_mode             = RADIO_SX126X_REGULATOR_DCDC,
#ifndef MODULE_KG100S
  .rx_boost                   = false,
#else
  .rx_boost                   = true,
#endif
  .lna_gain                   = RADIO_RX_LNA_GAIN,
  .bus_factory                = &radio_spi_factory,
  .gpio_power                 = SL_PIN_NRESET,
  .gpio_int1                  = SL_PIN_DIO,
  .gpio_radio_busy            = SL_PIN_BUSY,
  .gpio_rf_sw_ena             = HALO_GPIO_NOT_CONNECTED, // As DIO2 is configured to drive the RF switch, ANT_SW pin is redundant and can be set to logical 1
  .gpio_tx_bypass             = HALO_GPIO_NOT_CONNECTED,
  .pa_cfg_callback            = radio_sx1262_pa_cfg,
  .trim_cap_val_callback      = radio_sx1262_get_trim_cap_val,
#if (MODULE_KG100S == 2)
  .dio3_cfg_callback          = radio_dio3_ctrl_voltage,
  .band_sel_cfg_callback      = radio_band_sel_ctrl,
#endif

  .bus_selector = {
    .client_selector    = SL_PIN_NSS,
    .speed_hz           = SL_SPI_PERIPHERAL_BITRATE,
    .bit_order          = SID_PAL_SERIAL_BUS_BIT_ORDER_MSB_FIRST,
    .mode               = SL_SPI_PERIPHERAL_CLOCK_MODE,
  },
  .regional_config = {
    .radio_region         = RADIO_REGION,
    .reg_param_table_size = sizeof(radio_sx126x_regional_param) / sizeof(radio_sx126x_regional_param[0]),
    .reg_param_table      = radio_sx126x_regional_param,
  },
  .state_timings = {
    .sleep_to_full_power_us = 406,
    .full_power_to_sleep_us = 0,
    .rx_to_tx_us = 0,
    .tx_to_rx_us = 0,
#if defined(MODULE_KG100S)
    .tx_delay_us = 2380,
    .rx_delay_us = 1000,
#elif defined(EFR32XG21)
    .tx_delay_us = 1945,
    .rx_delay_us = 1000,
#else
    .tx_delay_us = 2800,
    .rx_delay_us = 1000,
#endif
  },
  .tcxo = {
    .ctrl = SX126X_TCXO_CTRL_NONE,
  },
  .internal_buffer = {
    .p    = radio_sx1262_buffer,
    .size = sizeof(radio_sx1262_buffer),
  },
};

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

const radio_sx126x_device_config_t *get_radio_cfg(void)
{
  return &radio_sx1262_cfg;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
#define SX126X_MAX_TRIM_VAL                0x2F
#define SX126X_DEFAULT_TRIM_VAL            0x12
#define SX126X_TRIM_VAL_NOT_PROGRAMMED     0xFF
#define SX126X_TRIM_VAL_NOT_PROGRAMMED_2   0x00

static int32_t radio_sx1262_get_trim_cap_val(uint16_t *trim)
{
  uint32_t val = 0xFFFFFFFF;

  sid_pal_mfg_store_read(MFG_STORE_APP_VALUE_SMTC_TRIM_CAP,
                         (uint8_t *)&val, MFG_STORE_APP_VALUE_SMTC_TRIM_CAP_SIZE);

  uint8_t xtb = (val >> 16) & 0xFF;
  uint8_t xta = (val >> 24) & 0xFF;

  if ((xtb == SX126X_TRIM_VAL_NOT_PROGRAMMED || xtb == SX126X_TRIM_VAL_NOT_PROGRAMMED_2)
      && xta != SX126X_TRIM_VAL_NOT_PROGRAMMED) {
    xtb = xta;     // some products write only one value that applies for both registers
  }

  if (xta == SX126X_TRIM_VAL_NOT_PROGRAMMED) {
    xta = SX126X_DEFAULT_TRIM_VAL;
  } else if (xta >= SX126X_MAX_TRIM_VAL) {
    xta = SX126X_MAX_TRIM_VAL;
  }

  if (xtb == SX126X_TRIM_VAL_NOT_PROGRAMMED) {
    xtb = SX126X_DEFAULT_TRIM_VAL;
  } else if (xtb >= SX126X_MAX_TRIM_VAL) {
    xtb = SX126X_MAX_TRIM_VAL;
  }

  *trim = (xta << 8 | xtb);

  return 0;
}

static int32_t radio_sx1262_pa_cfg(int8_t tx_power, radio_sx126x_pa_cfg_t *pa_cfg)
{
  int8_t pwr = tx_power;

  if (tx_power > RADIO_SX1262_MAX_TX_POWER) {
    pwr = RADIO_SX1262_MAX_TX_POWER;
  }

  if (tx_power < RADIO_SX1262_MIN_TX_POWER) {
    pwr = RADIO_SX1262_MIN_TX_POWER;
  }

  pa_cfg->pa_duty_cycle = RADIO_SX1262_PA_DUTY_CYCLE;
  pa_cfg->hp_max        = RADIO_SX1262_HP_MAX;
  pa_cfg->device_sel    = RADIO_SX1262_DEVICE_SEL;
  pa_cfg->pa_lut        = RADIO_SX1262_PA_LUT;
  pa_cfg->tx_power      = pwr;   // one to one mapping between tx params and tx power
  pa_cfg->ramp_time     = RADIO_SX126X_RAMP_40_US;

  return 0;
}

const struct sid_sub_ghz_links_config sub_ghz_link_config = {
  .enable_link_metrics = true,
  .sar_dcr = SAR_DCR_CONFIG,
  .registration_config = {
    .enable = true,
    .periodicity_s = UINT32_MAX,
  },
  .link2_max_tx_power_in_dbm = RADIO_MAX_TX_POWER_NA,
  .link3_max_tx_power_in_dbm = RADIO_MAX_TX_POWER_NA,
};

const struct sid_sub_ghz_links_config *app_get_sub_ghz_config(void)
{
  return &sub_ghz_link_config;
}

#if (MODULE_KG100S == 2)
/*
 * KG100S DVT use GPIO SL_PIN_KG100S_BAND_SEL pin to select rx band filter for US/EU band.
 * GPIO_0 Configuration:
 * 0 -- Rx Select US band (defaut)
 * 1 -- Rx Select EU band
 * 1 -- TX Path selection.
 */
static int32_t radio_band_sel_ctrl(bool tx_en)
{
    const halo_drv_semtech_ctx_t *ctx = sx126x_get_drv_ctx();
    int32_t err = SID_ERROR_NONE;

    do {
        if (sid_pal_gpio_set_direction(SL_PIN_KG100S_BAND_SEL, SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            err = SID_ERROR_GENERIC;
            break;
        }

        if (sid_pal_gpio_output_mode(SL_PIN_KG100S_BAND_SEL, SID_PAL_GPIO_OUTPUT_PUSH_PULL) != SID_ERROR_NONE) {
            err = SID_ERROR_GENERIC;
            break;
        }

        if (tx_en) {
            /* Radio TX */
            if (sid_pal_gpio_write(SL_PIN_KG100S_BAND_SEL, 1) != SID_ERROR_NONE) {
                err = SID_ERROR_GENERIC;
                break;
            }
        } else {
            /* Radio RX */
#if (RADIO_REGION == RADIO_REGION_NA)
            if (sid_pal_gpio_write(SL_PIN_KG100S_BAND_SEL, 1) != SID_ERROR_NONE) {
                err = SID_ERROR_GENERIC;
                break;
            }
#elif (RADIO_REGION == RADIO_REGION_EU)
            if (sid_pal_gpio_write(SL_PIN_KG100S_BAND_SEL, 1) != SID_ERROR_NONE) {
                err = SID_ERROR_GENERIC;
                break;
            }
#elif (RADIO_REGION == RADIO_REGION_NONE)
            // ToDO ,default use NA
            if (sid_pal_gpio_write(SL_PIN_KG100S_BAND_SEL, 1) != SID_ERROR_NONE) {
                err = SID_ERROR_GENERIC;
                break;
            }
#else
#error "radio region undefined"
#endif
        }
    } while (0);
    return err;
}
#endif

#if (MODULE_KG100S == 2)
/*
 * KG100S DVT1 use SX126X DIO3 to supply power for ANT SW.
 * Configuration:
 * 1. Set bit 3 of register@0x0580 (output enable on DIO3)
 * 2. Clear bit 3 of register@0x0583 (input disable on DIO3)
 * 3. Clear bit 3 of register@0x0584 (pull-up disable on DIO3) - optional
 * 4. Clear bit 3 of register@0x0585 (pull-down disable on DIO3) - optional
 * 5. Set bits [0 to 2] of register@0x0920 to the output voltage you need on DIO3
 *    (see Table 13-35: tcxoVoltage Configuration Definition in the related datasheet)
 *
 * Output programming:
 * - Set bit 3 of register@0x0920 to have DIO3 "high"
 * - Clear bit 3 of register@0x0920 to have DIO3 "low"
 */
static sx126x_status_t sx126x_dio3_gpio_set(const void *context)
{
  int32_t status = SX126X_STATUS_OK;
  uint8_t reg_val = 0x00;

  // set bit 3 of 0x0920
  status = sx126x_read_register(context, 0x0920, &reg_val, 1);
  reg_val |= 1 << 3;
  status = sx126x_write_register(context, 0x0920, &reg_val, 1);

  return status;
}

static sx126x_status_t sx126x_dio3_gpio_clear(const void *context)
{
  int32_t status = SX126X_STATUS_OK;
  uint8_t reg_val = 0x00;

  // set bit 3 of 0x0920
  status = sx126x_read_register(context, 0x0920, &reg_val, 1);
  reg_val &= ~(1 << 3);
  status = sx126x_write_register(context, 0x0920, &reg_val, 1);

  return status;
}

static sx126x_status_t sx126x_dio3_gpio_init(const void *context, uint8_t voltage)
{
  int32_t status = SX126X_STATUS_OK;
  uint8_t reg_val = 0x00;
  // set bit 3 of 0x0580
  status = sx126x_read_register(context, 0x0580, &reg_val, 1);
  reg_val |= 1 << 3;
  status = sx126x_write_register(context, 0x0580, &reg_val, 1);

  // clear bit 3 of 0x0583
  status = sx126x_read_register(context, 0x0583, &reg_val, 1);
  reg_val &= ~(1 << 3);
  status = sx126x_write_register(context, 0x0583, &reg_val, 1);

  // clear bit 3 of 0x0584
  status = sx126x_read_register(context, 0x0584, &reg_val, 1);
  reg_val &= ~(1 << 3);
  status = sx126x_write_register(context, 0x0584, &reg_val, 1);

  // clear bit 3 of 0x0585
  status = sx126x_read_register(context, 0x0585, &reg_val, 1);
  reg_val &= ~(1 << 3);
  status = sx126x_write_register(context, 0x0585, &reg_val, 1);

  // set bits 0-2 of 0x0920 to output 3.0V, VBAT - 200 mV limit
  reg_val = voltage;     // 0x6;   // 3.0V
  status = sx126x_write_register(context, 0x0920, &reg_val, 1);

  return status;
}

static int32_t sx126x_dio3_output_voltage(const void *ctx, uint8_t voltage)
{
  int32_t err = SID_ERROR_GENERIC;
  do {
    if (sx126x_dio3_gpio_init(ctx, voltage) != SX126X_STATUS_OK) {
      break;
    }
    // Set Output
    if (sx126x_dio3_gpio_set(ctx) != SX126X_STATUS_OK) {
      break;
    }
    err = SID_ERROR_NONE;
  } while (0);
  return err;
}

static int32_t radio_dio3_ctrl_voltage(uint8_t radio_state)
{
  const halo_drv_semtech_ctx_t *ctx = sx126x_get_drv_ctx();
  uint8_t voltage = RADIO_SX126X_TCXO_CTRL_3_0V;
  int32_t err = SID_ERROR_NONE;

  if (radio_state == 0) {
    /* INIT */
    SL_SID_LOG_PAL_INFO("sx126x_dio3_output_voltage %d", voltage);

    if ((err = sx126x_dio3_output_voltage(ctx, voltage)) != SID_ERROR_NONE) {
      SL_SID_LOG_PAL_ERROR("sx126x_dio3_output_voltage error");
    }
  } else if (radio_state == 1) {
    /* SLEEP */
    // Reset DIO3 to LOW state
    if (sx126x_dio3_gpio_clear(ctx) != SX126X_STATUS_OK) {
      err = SID_ERROR_GENERIC;
    }
  } else if (radio_state == 2) {
    /* WAKEUP */
    if ((err = sx126x_dio3_output_voltage(ctx, voltage)) != SID_ERROR_NONE) {
      SL_SID_LOG_PAL_ERROR("sx126x_dio3_output_voltage error");
    }
  }
  return err;
}
#endif

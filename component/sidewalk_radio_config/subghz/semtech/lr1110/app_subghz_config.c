/***************************************************************************//**
 * @file
 * @brief app_subghz_config.c
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *
 * This file supports radio configutations.
 * Some parts of the the code are based on Semtech reference driver
 * This code was modified by Semtech
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

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <sid_pal_serial_bus_efr32_spi_config.h>
#include "lr11xx_config.h"
#include <gpio.h>
#include "app_subghz_config.h"
#include <mfg_store_app_values.h>
#include <sid_900_cfg.h>
#include <sid_pal_log_ifc.h>

#include <sl_spidrv_exp_config.h>
#define SL_SPI_PERIPHERAL_ID SL_SPIDRV_EXP_PERIPHERAL
#define SL_SPI_PERIPHERAL_BITRATE SL_SPIDRV_EXP_BITRATE
#define SL_SPI_PERIPHERAL_CLOCK_MODE SL_SPIDRV_EXP_CLOCK_MODE

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

#define RADIO_ANT_GAIN( X ) ( ( X ) *100 )

#define UNUSED( x ) ( void ) ( x )

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */


#ifndef REGION_US915
#define REGION_US915
#endif

#define RADIO_REGION RADIO_REGION_NA

#define RADIO_LR11XX_MAX_TX_POWER 22
#define RADIO_LR11XX_MIN_TX_POWER -9

#define RADIO_MAX_TX_POWER_NA 20
#define RADIO_MAX_TX_POWER_EU 14

#define RADIO_LR11XX_SPI_BUFFER_SIZE 255
#define RADIO_RX_LNA_GAIN 0

#define NULL_STRUCT_INITIALIZER \
    {                           \
        0                       \
    }
#define INVALID_DT_GPIO NULL_STRUCT_INITIALIZER

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */
static int32_t radio_lr11xx_pa_cfg( int8_t tx_power, radio_lr11xx_pa_cfg_t* pa_cfg );

static uint8_t radio_lr11xx_buffer[RADIO_LR11XX_SPI_BUFFER_SIZE] = { 0 };

const uint8_t pa_duty_cycles[] = {
    //  -9,   -8,   -7,   -6,   -5,   -4,   -3,   -2,   -1,    0,    1,    2,    3,    4,    5,    6,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //   7,    8,   9,    10,   11,   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,   22
    0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0x04, 0x05, 0x07, 0x03, 0x04, 0x02, 0x05, 0x03, 0x04, 0x04
};

const uint8_t pa_hp_sels[] = {
    //  -9,   -8,   -7,   -6,   -5,   -4,   -3,   -2,   -1,    0,    1,    2,    3,    4,    5,    6,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //   7,    8,   9,    10,   11,   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,   22
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x05, 0x04, 0x07, 0x07, 0x07
};

const uint8_t powers[] = {
    //  9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5,  6,
    -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 3, 4, 7, 8, 9, 10,
    // 7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22
    12, 13, 14, 14, 13, 14, 14, 14, 14, 22, 22, 22, 22, 22, 21, 22
};

static const struct sid_pal_serial_bus_efr32_spi_config radio_spi_config = {
    .peripheral_id = SL_SPI_PERIPHERAL_ID,
};

static const struct sid_pal_serial_bus_factory radio_spi_factory = {
    .create = sid_pal_serial_bus_efr32_spi_create,
    .config = &radio_spi_config,
};

const radio_lr11xx_regional_param_t radio_lr11xx_regional_param[] = {
#if defined( REGION_ALL ) || defined( REGION_US915 )
    { .param_region     = RADIO_REGION_NA,
      .max_tx_power     = { RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA,
                        RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA },
      .cca_level_adjust = { 0, 0, 0, 0, 0, 0 },
      .ant_dbi          = RADIO_ANT_GAIN( 2.15 ) },
#endif
#if defined( REGION_ALL ) || defined( REGION_EU868 )
    { .param_region     = RADIO_REGION_EU,
      .max_tx_power     = { RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU,
                        RADIO_MAX_TX_POWER_EU, RADIO_MAX_TX_POWER_EU },
      .cca_level_adjust = { 0, 0, 0, 0, 0, 0 },
      .ant_dbi          = RADIO_ANT_GAIN( 2.15 ) },
#endif
};

__attribute__( ( weak ) ) void on_gnss_scan_done( const void* context );  // implement in application
__attribute__( ( weak ) ) void on_wifi_scan_done( const void* context );  // implement in application

void on_gnss_scan_done( const void* context )
{
    UNUSED( context );
}
void on_wifi_scan_done( const void* context )
{
    UNUSED( context );
}

__attribute__( ( weak ) ) void* const gnss_scan_done_context = NULL;
__attribute__( ( weak ) ) void* const wifi_scan_done_context = NULL;

static const radio_lr11xx_device_config_t radio_lr11xx_cfg =
{
  .regulator_mode = LR11XX_SYSTEM_REG_MODE_DCDC,
  .rx_boost = false,
  .lna_gain = RADIO_RX_LNA_GAIN,
  .bus_factory = &radio_spi_factory,

  .pa_cfg_callback = radio_lr11xx_pa_cfg,

  .wakeup_delay_us = 0,
  .lfclock_cfg = LR11XX_SYSTEM_LFCLK_XTAL,

#if defined( LR1121 )
  .tcxo_config = {
    .ctrl                   = LR11XX_TCXO_CTRL_NONE,
  },
#else
    .tcxo_config = {
    .ctrl = LR11XX_TCXO_CTRL_DIO3,
    .tune = LR11XX_SYSTEM_TCXO_CTRL_3_0V,
    .timeout = 14,  // = 427us. Measured by Logic Analyzer
},
#endif

  .rfswitch = {
      .enable = LR11XX_SYSTEM_RFSW0_HIGH | LR11XX_SYSTEM_RFSW1_HIGH | LR11XX_SYSTEM_RFSW2_HIGH,
      .standby = 0,
      .rx = LR11XX_SYSTEM_RFSW0_HIGH,
      .tx = LR11XX_SYSTEM_RFSW0_HIGH | LR11XX_SYSTEM_RFSW1_HIGH,
      .tx_hp = LR11XX_SYSTEM_RFSW1_HIGH,
      .tx_hf = 0,
      .gnss = LR11XX_SYSTEM_RFSW2_HIGH,
      .wifi = 0,
  },

  .rssi_no_signal_offset = 0,

  .mitigations = { /* prevents servicing of GNSS interrupt */
    .irq_noise_during_sleep = false,
    .lbd_clear_on_wakeup    = false,
  },

  .internal_buffer = {
    .p = radio_lr11xx_buffer,
    .size = sizeof(radio_lr11xx_buffer),
  },

  .state_timings = {   // sid_pal_radio_state_transition_timings_t
    .sleep_to_full_power_us = 643,  // 643us, Measured by Logic Analyzer.
    .full_power_to_sleep_us = 0,
    .rx_to_tx_us = 0,
    .tx_to_rx_us = 0,
    .tcxo_delay_us = 0,
#if defined(EFR32XG21)
    .tx_delay_us = 2290,
    .rx_delay_us = 1000,
#else
    .tx_delay_us = 3000,
    .rx_delay_us = 1000,
#endif
  },

  .bus_selector = {
    .client_selector    = SL_PIN_NSS,
    .speed_hz           = SL_SPI_PERIPHERAL_BITRATE,
    .bit_order          = SID_PAL_SERIAL_BUS_BIT_ORDER_MSB_FIRST,
    .mode               = SL_SPI_PERIPHERAL_CLOCK_MODE,
  },

  .regional_config = {
    .radio_region = RADIO_REGION,
    .reg_param_table_size = sizeof(radio_lr11xx_regional_param) / sizeof(radio_lr11xx_regional_param[0]),
    .reg_param_table = radio_lr11xx_regional_param,
  },

  .gpios = {
      .power      = SL_PIN_NRESET,
      .int1       = SL_PIN_DIO,
      .radio_busy = SL_PIN_BUSY,
      .rf_sw_ena  = HALO_GPIO_NOT_CONNECTED,
      .tx_bypass  = HALO_GPIO_NOT_CONNECTED,
      .txrx       = HALO_GPIO_NOT_CONNECTED,
#ifdef LR11XX_E707
      .led_rx     = SL_PIN_LED_RX,
      .led_tx     = SL_PIN_LED_TX,
      .led_sniff  = SL_PIN_LED_SNIFFING,
      .gnss_lna   = SL_PIN_GNSS_LNA,
#else
      .led_rx     = HALO_GPIO_NOT_CONNECTED,
      .led_tx     = HALO_GPIO_NOT_CONNECTED,
      .led_sniff  = HALO_GPIO_NOT_CONNECTED,
      .gnss_lna   = HALO_GPIO_NOT_CONNECTED,
#endif
  },

  .gnss_scan.post_hook = on_gnss_scan_done,
  .wifi_scan.post_hook = on_wifi_scan_done,
  .gnss_scan.arg       = gnss_scan_done_context,
  .wifi_scan.arg       = wifi_scan_done_context,
};

const struct sid_sub_ghz_links_config lr11xx_sub_ghz_link_config = {
  .enable_link_metrics = true,
  .sar_dcr = 100,
  .registration_config = {
    .enable = true,
    .periodicity_s = UINT32_MAX,
  },
};

const struct sid_sub_ghz_links_config* app_get_sub_ghz_config( void )
{
    return &lr11xx_sub_ghz_link_config;
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */


/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

const radio_lr11xx_device_config_t* get_radio_cfg( void )
{
    //radio_lr11xx_cfg.gnss_scan.post_hook = on_gnss_scan_done;
    //radio_lr11xx_cfg.wifi_scan.post_hook = on_wifi_scan_done;
    //radio_lr11xx_cfg.gnss_scan.arg       = gnss_scan_done_context;
    //radio_lr11xx_cfg.wifi_scan.arg       = wifi_scan_done_context;

    return &radio_lr11xx_cfg;
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static int32_t radio_lr11xx_pa_cfg( int8_t tx_power, radio_lr11xx_pa_cfg_t* pa_cfg )
{
    int8_t pwr = tx_power;

    if( tx_power > RADIO_LR11XX_MAX_TX_POWER )
    {
        pwr = RADIO_LR11XX_MAX_TX_POWER;
    }

    if( tx_power < RADIO_LR11XX_MIN_TX_POWER )
    {
        pwr = RADIO_LR11XX_MIN_TX_POWER;
    }

    if( pwr > 15 )
    {
        pa_cfg->pa_cfg.pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VBAT;
        pa_cfg->pa_cfg.pa_sel        = LR11XX_RADIO_PA_SEL_HP;
    }
    else
    {
        pa_cfg->pa_cfg.pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG;
        pa_cfg->pa_cfg.pa_sel        = LR11XX_RADIO_PA_SEL_LP;
    }
    pa_cfg->pa_cfg.pa_duty_cycle = pa_duty_cycles[pwr + 9];
    pa_cfg->pa_cfg.pa_hp_sel     = pa_hp_sels[pwr + 9];

    pa_cfg->ramp_time       = LR11XX_RADIO_RAMP_48_US;
    pa_cfg->tx_power_in_dbm = powers[pwr + 9];
    pa_cfg->enable_ext_pa   = false;

    return 0;
}

/* --- EOF ------------------------------------------------------------------ */

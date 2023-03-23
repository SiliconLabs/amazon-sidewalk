/***************************************************************************//**
 * @file app_cli_settings.c
 * @brief Application settings handler
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "app_cli_settings.h"
#include "app_process.h"
#include "sl_sidewalk_cli_settings.h"
#include "sid_api.h"
#include "sid_ble_link_config_ifc.h"
#include "app_ble_config.h"
#include "app_cli.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Functions Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static const app_settings_sidewalk_t app_settings_sidewalk_default = {
  .initialized_link = "NONE",
  .started_link = "NONE",
  .region = "US",
  .state = "NOT READY",
  .time = "0.0",
  .auto_connect = "Not Supported"
};

static const app_settings_radio_t app_settings_radio_default = {
  .wakeup_type = 0,
  .rssi = 0,
  .snr = 0
};

static const app_settings_ble_t app_settings_ble_default = {
  .random_mac = "PUBLIC",
  .output_power = 22
};

static const app_settings_fsk_t app_settings_fsk_default = {
  .data_rate = "50 Kbps",
  .min_freq = "902.2 MHz",
  .max_freq = "927.8 MHz",
  .region = "US",
  .power_profile = "0",
  .offset = 9,
  .range = 50,
  .bcn_interval = 10,
  .rx_window_count = 0,
  .rx_window_separation = 0,
  .rx_window_duration = 0
};

static const app_settings_css_t app_settings_css_default = {
  .bandwidth = "500 Khz",
  .min_freq = "902.5 MHz",
  .max_freq = "926.5 MHz",
  .region = "US",
  .power_profile = "0",
  .rx_window_separation = 5,
  .rx_window_count = 20
};

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

app_settings_sidewalk_t app_settings_sidewalk;
app_settings_radio_t app_settings_radio;
app_settings_ble_t app_settings_ble;
app_settings_fsk_t app_settings_fsk;
app_settings_css_t app_settings_css;

const char *app_settings_domain_str[] =
{
  "sidewalk",
  "radio",
  "ble",
  "fsk",
  "css",
  NULL,
};

const sl_app_saving_item_t app_saving_item_sidewalk = {
  .data = &app_settings_sidewalk,
  .data_size = sizeof(app_settings_sidewalk),
  .default_val = &app_settings_sidewalk_default
};

const sl_app_saving_item_t app_saving_item_radio = {
  .data = &app_settings_radio,
  .data_size = sizeof(app_settings_radio),
  .default_val = &app_settings_radio_default
};

const sl_app_saving_item_t app_saving_item_ble = {
  .data = &app_settings_ble,
  .data_size = sizeof(app_settings_ble),
  .default_val = &app_settings_ble_default
};

const sl_app_saving_item_t app_saving_item_fsk = {
  .data = &app_settings_fsk,
  .data_size = sizeof(app_settings_fsk),
  .default_val = &app_settings_fsk_default
};

const sl_app_saving_item_t app_saving_item_css = {
  .data = &app_settings_css,
  .data_size = sizeof(app_settings_css),
  .default_val = &app_settings_css_default
};

const sl_app_saving_item_t *saving_settings[] = {
  &app_saving_item_sidewalk,
  &app_saving_item_radio,
  &app_saving_item_ble,
  &app_saving_item_fsk,
  &app_saving_item_css,
  NULL
};

const sl_app_settings_entry_t app_settings_entries[] =
{
  {
    .key = "initialized_link",
    .domain = app_settings_domain_sidewalk,
    .value_size = LINK_STRING_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_sidewalk.initialized_link,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_initialized_link,
    .description = "Current initialized link: ble, fsk, css or any"
  },
  {
    .key = "started_link",
    .domain = app_settings_domain_sidewalk,
    .value_size = LINK_STRING_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_sidewalk.started_link,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_started_link,
    .description = "Current started link running: ble, fsk, css or any"
  },
  {
    .key = "region",
    .domain = app_settings_domain_sidewalk,
    .value_size = REGION_STRING_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_sidewalk.region,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = sl_app_settings_set_region,
    .get_handler = sl_app_settings_get_region,
    .description = "Regulatory domain for Sidewalk: not supported, for future use."
  },
  {
    .key = "state",
    .domain = app_settings_domain_sidewalk,
    .value_size = STATE_STRING_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_sidewalk.state,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_state,
    .description = "Device state: registered or not registered."
  },
  {
    .key = "time",
    .domain = app_settings_domain_sidewalk,
    .value_size = TIME_STRING_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_sidewalk.time,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_time,
    .description = "Endpoint time from sidewalk stack."
  },
  {
    .key = "auto_connect",
    .domain = app_settings_domain_sidewalk,
    .value_size = AUTOCONNECT_STRING_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_sidewalk.auto_connect,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = sl_app_settings_set_autoconnect,
    .get_handler = sl_app_settings_get_autoconnect,
    .description = "Not supported yet."
  },
  {
    .key = "wakeup_type",
    .domain = app_settings_domain_radio,
    .value_size = SL_APP_SETTINGS_VALUE_SIZE_UINT8,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_radio.wakeup_type,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = sl_app_settings_set_wakeup_type,
    .get_handler = sl_app_settings_get_wakeup_type,
    .description = "Decide events from which to wake-up, not used for now."
  },
  {
    .key = "rssi",
    .domain = app_settings_domain_radio,
    .value_size = SL_APP_SETTINGS_VALUE_SIZE_UINT8,
    .input = SL_APP_SETTINGS_INPUT_FLAG_SIGNED,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_SIGNED,
    .value = &app_settings_radio.rssi,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_rssi,
    .description = "Received signal strength indication from last *downlink* message received. In dBm."
  },
  {
    .key = "snr",
    .domain = app_settings_domain_radio,
    .value_size = SL_APP_SETTINGS_VALUE_SIZE_UINT8,
    .input = SL_APP_SETTINGS_INPUT_FLAG_SIGNED,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_SIGNED,
    .value = &app_settings_radio.snr,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_snr,
    .description = "Signal over Noise Ratio from last *downlink* received message."
  },
  {
    .key = "ble_mtu",
    .domain = app_settings_domain_ble,
    .value_size = SL_APP_SETTINGS_VALUE_SIZE_UINT32,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_ble.mtu,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_mtu_ble,
    .description = "Maximum transmission unit for BLE"
  },
  {
    .key = "random_mac",
    .domain = app_settings_domain_ble,
    .value_size = MAC_STRING_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_ble.random_mac,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_random_mac,
    .description = ""
                   "Use a Randomize MAC address. Possible value are: "
                   "PUBLIC, "
                   "RANDOM_PRIVATE_NON_RESOLVABLE, "
                   "STATIC_RANDOM, "
                   "RANDOM_PRIVATE_RESOLVABLE."
  },
  {
    .key = "output_power",
    .domain = app_settings_domain_ble,
    .value_size = SL_APP_SETTINGS_VALUE_SIZE_UINT16,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_ble.output_power,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_output_power,
    .description = "TX output power in dBm."
  },
  {
    .key = "fsk_data_rate",
    .domain = app_settings_domain_fsk,
    .value_size = FREQ_STRING_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_fsk.data_rate,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_string,
    .description = "FSK default data rate"
  },
  {
    .key = "fsk_min_freq",
    .domain = app_settings_domain_fsk,
    .value_size = FREQ_STRING_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_fsk.min_freq,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_string,
    .description = "FSK low frequency"
  },
  {
    .key = "fsk_max_freq",
    .domain = app_settings_domain_fsk,
    .value_size = FREQ_STRING_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_fsk.max_freq,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_string,
    .description = "FSK top frequency"
  },
  {
    .key = "fsk_region",
    .domain = app_settings_domain_fsk,
    .value_size = REGION_STRING_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_fsk.region,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_string,
    .description = "FSK region parameters"
  },
  {
    .key = "power_profile",
    .domain = app_settings_domain_fsk,
    .value_size = POWER_PROFILE_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_fsk.power_profile,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = sl_app_settings_set_fsk_power_profile,
    .get_handler = sl_app_settings_get_fsk_power_profile,
    .description = "Power profile currently in use: 1 or 2"
  },
  {
    .key = "fsk_mtu",
    .domain = app_settings_domain_fsk,
    .value_size = sizeof(size_t),
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_fsk.mtu,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_mtu_fsk,
    .description = "Maximum transmission unit for FSK."
  },
  {
    .key = "offset",
    .domain = app_settings_domain_fsk,
    .value_size = SL_APP_SETTINGS_VALUE_SIZE_UINT8,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_fsk.offset,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = sl_app_settings_set_offset,
    .get_handler = sl_app_settings_get_offset,
    .description = "First slots for RX opportunity window."
  },
  {
    .key = "range",
    .domain = app_settings_domain_fsk,
    .value_size = SL_APP_SETTINGS_VALUE_SIZE_UINT8,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_fsk.range,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = sl_app_settings_set_range,
    .get_handler = sl_app_settings_get_range,
    .description = "Number of slots between RX opportunity windows."
  },
  {
    .key = "beacon_interval",
    .domain = app_settings_domain_fsk,
    .value_size = SL_APP_SETTINGS_VALUE_SIZE_UINT8,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_fsk.bcn_interval,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_beacon_interval,
    .description = "Time interval between 2 beacon (in seconds)."
  },
  {
    .key = "rx_window_count",
    .domain = app_settings_domain_fsk,
    .value_size = SL_APP_SETTINGS_VALUE_SIZE_UINT16,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_fsk.rx_window_count,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_fsk_rx_window_count,
    .description = "Number of RX opportunities (0 Maximum number - 1 continuous RX)."
  },
  {
    .key = "rx_window_separation",
    .domain = app_settings_domain_fsk,
    .value_size = SL_APP_SETTINGS_VALUE_SIZE_UINT32,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_fsk.rx_window_separation,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_fsk_rx_window_separation,
    .description = "Time between between 2 RX opportunities (in ms)."
  },
  {
    .key = "rx_window_duration",
    .domain = app_settings_domain_fsk,
    .value_size = SL_APP_SETTINGS_VALUE_SIZE_UINT32,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_fsk.rx_window_duration,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = NULL,
    .description = "Duration of RX opportunities."
  },
  {
    .key = "css_bandwidth",
    .domain = app_settings_domain_css,
    .value_size = FREQ_STRING_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_css.bandwidth,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_string,
    .description = "CSS Bandwidth"
  },
  {
    .key = "css_min_freq",
    .domain = app_settings_domain_css,
    .value_size = FREQ_STRING_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_css.min_freq,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_string,
    .description = "CSS Low frequency"
  },
  {
    .key = "css_max_freq",
    .domain = app_settings_domain_css,
    .value_size = FREQ_STRING_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_css.max_freq,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_string,
    .description = "CSS Top frequency"
  },
  {
    .key = "css_region",
    .domain = app_settings_domain_css,
    .value_size = REGION_STRING_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_css.region,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_string,
    .description = "CSS Bandwidth"
  },
  {
    .key = "power_profile",
    .domain = app_settings_domain_css,
    .value_size = POWER_PROFILE_SIZE + 1,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_css.power_profile,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = sl_app_settings_set_css_power_profile,
    .get_handler = sl_app_settings_get_css_power_profile,
    .description = "Power profile currently in use: A, B or D"
  },
  {
    .key = "css_mtu",
    .domain = app_settings_domain_css,
    .value_size = sizeof(size_t),
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_css.mtu,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_mtu_css,
    .description = "Maximum transmission unit for CSS."
  },
  {
    .key = "rx_window_separation",
    .domain = app_settings_domain_css,
    .value_size = SL_APP_SETTINGS_VALUE_SIZE_UINT8,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_css.rx_window_separation,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = sl_app_settings_get_css_rx_window_separation,
    .description = "Period of RX windows after transmission - CSS power_profile A only."
  },
  {
    .key = "rx_window_count",
    .domain = app_settings_domain_css,
    .value_size = SL_APP_SETTINGS_VALUE_SIZE_UINT8,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = &app_settings_css.rx_window_count,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = sl_app_settings_set_css_rx_window_count,
    .get_handler = sl_app_settings_get_css_rx_window_count,
    .description = "Number of RX windows after transmission - CSS power_profile A only."
  },
  {
    .key = NULL,
    .domain = 0,
    .value_size = SL_APP_SETTINGS_VALUE_SIZE_NONE,
    .input = SL_APP_SETTINGS_INPUT_FLAG_DEFAULT,
    .output = SL_APP_SETTINGS_OUTPUT_FLAG_DEFAULT,
    .value = NULL,
    .input_enum_list = NULL,
    .output_enum_list = NULL,
    .set_handler = NULL,
    .get_handler = NULL,
    .description = NULL
  }
};

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

sl_status_t sl_app_settings_get_initialized_link(char *value_str,
                                                 const char *key_str,
                                                 const sl_app_settings_entry_t *entry)
{
  char s_link[LINK_STRING_SIZE + 1] = { 0 };

  switch (current_init_link) {
    case SID_LINK_TYPE_1:
      sprintf(s_link, "BLE");
      break;

    case SID_LINK_TYPE_2:
      sprintf(s_link, "FSK");
      break;

    case SID_LINK_TYPE_3:
      sprintf(s_link, "CSS");
      break;

    default:
      sprintf(s_link, "NONE");
      break;
  }
  // Write result to state attribute
  sl_app_settings_set_string(s_link, key_str, entry);

  // Finally display value to user
  sl_app_settings_get_string(value_str, key_str, entry);

  return SID_ERROR_NONE;
}

sl_status_t sl_app_settings_get_started_link(char *value_str,
                                             const char *key_str,
                                             const sl_app_settings_entry_t *entry)
{
  sl_status_t ret = SID_ERROR_NONE;
  char s_link[LINK_STRING_SIZE + 1] = { 0 };
  app_setting_cli_queue_t settings = { 0 };

  // Trigger logic to main sidewalk thread
  sl_app_trigger_get_status();

  // Wait for data from sidewalk thread
  if (xQueueReceive(g_cli_event_queue, &settings, pdMS_TO_TICKS(2000))) {
    switch (settings.current_status.detail.link_status_mask) {
      case SID_LINK_TYPE_1:
        sprintf(s_link, "BLE");
        break;

      case SID_LINK_TYPE_2:
        sprintf(s_link, "FSK");
        break;

      case SID_LINK_TYPE_3:
        sprintf(s_link, "CSS");
        break;

      default:
        sprintf(s_link, "NONE");
        break;
    }
    // Write result to state attribute
    sl_app_settings_set_string(s_link, key_str, entry);

    // Finally display value to user
    sl_app_settings_get_string(value_str, key_str, entry);
  } else {
    printf("Error while trying to get link\n");
    ret = SID_ERROR_GENERIC;
  }

  return ret;
}

sl_status_t sl_app_settings_get_region(char *value_str,
                                       const char *key_str,
                                       const sl_app_settings_entry_t *entry)
{
  return sl_app_settings_get_string(value_str, key_str, entry);
}

sl_status_t sl_app_settings_set_region(const char *value_str,
                                       const char *key_str,
                                       const sl_app_settings_entry_t *entry)
{
  sl_app_settings_set_string(value_str, key_str, entry);
  printf("Attribute not supported for now.\n");

  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_app_settings_get_state(char *value_str,
                                      const char *key_str,
                                      const sl_app_settings_entry_t *entry)
{
  sl_status_t ret = SID_ERROR_NONE;
  char s_state[STATE_STRING_SIZE + 1] = { 0 };
  app_setting_cli_queue_t settings = { 0 };

  // Trigger logic to main sidewalk thread
  sl_app_trigger_get_status();

  // Wait for data from sidewalk thread
  if (xQueueReceive(g_cli_event_queue, &settings, pdMS_TO_TICKS(2000))) {
    switch (settings.current_status.state) {
      case SID_STATE_READY:
        sprintf(s_state, "READY");
        break;

      case SID_STATE_NOT_READY:
        sprintf(s_state, "NOT_READY");
        break;

      case SID_STATE_ERROR:
        sprintf(s_state, "ERROR");
        break;

      case SID_STATE_SECURE_CHANNEL_READY:
        sprintf(s_state, "SECURE_CHANNEL_READY");
        break;

      default:
        // Invalid state, nothing to do
        break;
    }
    // Write result to state attribute
    sl_app_settings_set_string(s_state, key_str, entry);

    // Finally display value to user
    sl_app_settings_get_string(value_str, key_str, entry);
  } else {
    printf("Error while trying to get state\n");
    ret = SID_ERROR_GENERIC;
  }

  return ret;
}

sl_status_t sl_app_settings_get_time(char *value_str,
                                     const char *key_str,
                                     const sl_app_settings_entry_t *entry)
{
  char s_time[TIME_STRING_SIZE + 1] = { 0 };
  app_setting_cli_queue_t settings = { 0 };

  // Trigger logic to main sidewalk thread
  sl_app_trigger_get_time();

  // Wait for data from sidewalk thread
  if (xQueueReceive(g_cli_event_queue, &settings, pdMS_TO_TICKS(2000))) {
    // Write result to time attribute
    sprintf(s_time, "%lu.%lu", settings.current_time.tv_sec, settings.current_time.tv_nsec);
    sl_app_settings_set_string(s_time, key_str, entry);

    // Finally display value to user
    sl_app_settings_get_string(value_str, key_str, entry);
  } else {
    printf("Error while trying to get time\n");
  }

  return SID_ERROR_NONE;
}

sl_status_t sl_app_settings_get_autoconnect(char *value_str,
                                            const char *key_str,
                                            const sl_app_settings_entry_t *entry)
{
  sl_app_settings_get_string(value_str, key_str, entry);

  return SID_ERROR_NONE;
}

sl_status_t sl_app_settings_set_autoconnect(const char *value_str,
                                            const char *key_str,
                                            const sl_app_settings_entry_t *entry)
{
  (void)value_str;
  (void)key_str;
  (void)entry;
  printf("Attribute not supported for now.");

  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_app_settings_get_frequency(char *value_str,
                                          const char *key_str,
                                          const sl_app_settings_entry_t *entry)
{
  return sl_app_settings_get_string(value_str, key_str, entry);
}

sl_status_t sl_app_settings_set_wakeup_type(const char *value_str,
                                            const char *key_str,
                                            const sl_app_settings_entry_t *entry)
{
  (void)value_str;
  (void)key_str;
  (void)entry;
  printf("Attribute not supported for now.\n");

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_get_wakeup_type(char *value_str,
                                            const char *key_str,
                                            const sl_app_settings_entry_t *entry)
{
  return sl_app_settings_get_string(value_str, key_str, entry);
}

sl_status_t sl_app_settings_get_rssi(char *value_str,
                                     const char *key_str,
                                     const sl_app_settings_entry_t *entry)
{
  char s_rssi[10] = { 0 };

  sprintf(s_rssi, "%d", LAST_MESSG_RCVD_DESC.msg_desc_attr.rx_attr.rssi);
  sl_app_settings_set_integer(s_rssi, key_str, entry);
  sl_app_settings_get_integer(value_str, key_str, entry);

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_get_snr(char *value_str,
                                    const char *key_str,
                                    const sl_app_settings_entry_t *entry)
{
  char s_snr[10] = { 0 };


  sprintf(s_snr, "%d", LAST_MESSG_RCVD_DESC.msg_desc_attr.rx_attr.snr);
  sl_app_settings_set_integer(s_snr, key_str, entry);
  sl_app_settings_get_integer(value_str, key_str, entry);

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_get_mtu_fsk(char *value_str,
                                        const char *key_str,
                                        const sl_app_settings_entry_t *entry)
{
  char s_mtu[MTU_STRING_SIZE] = { 0 };
  app_setting_cli_queue_t settings = { 0 };

  sl_app_trigger_get_mtu(SID_LINK_TYPE_2);

  // Wait for data from sidewalk thread
  if (xQueueReceive(g_cli_event_queue, &settings, pdMS_TO_TICKS(2000))) {
    // Write result to time attribute
    sprintf(s_mtu, "%u", settings.mtu);
    sl_app_settings_set_integer(s_mtu, key_str, entry);

    // Finally display value to user
    sl_app_settings_get_integer(value_str, key_str, entry);
  } else {
    printf("Error while trying to get mtu\n");
  }

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_get_mtu_css(char *value_str,
                                        const char *key_str,
                                        const sl_app_settings_entry_t *entry)
{
  char s_mtu[MTU_STRING_SIZE] = { 0 };
  app_setting_cli_queue_t settings = { 0 };

  sl_app_trigger_get_mtu(SID_LINK_TYPE_3);

  // Wait for data from sidewalk thread
  if (xQueueReceive(g_cli_event_queue, &settings, pdMS_TO_TICKS(2000))) {
    // Write result to time attribute
    sprintf(s_mtu, "%u", settings.mtu);
    sl_app_settings_set_integer(s_mtu, key_str, entry);

    // Finally display value to user
    sl_app_settings_get_integer(value_str, key_str, entry);
  } else {
    printf("Error while trying to get mtu\n");
  }

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_get_mtu_ble(char *value_str,
                                        const char *key_str,
                                        const sl_app_settings_entry_t *entry)
{
  char s_mtu[MTU_STRING_SIZE] = { 0 };
  app_setting_cli_queue_t settings = { 0 };

  sl_app_trigger_get_mtu(SID_LINK_TYPE_1);

  // Wait for data from sidewalk thread
  if (xQueueReceive(g_cli_event_queue, &settings, pdMS_TO_TICKS(2000))) {
    // Write result to time attribute
    sprintf(s_mtu, "%u", settings.mtu);
    sl_app_settings_set_integer(s_mtu, key_str, entry);

    // Finally display value to user
    sl_app_settings_get_integer(value_str, key_str, entry);
  } else {
    printf("Error while trying to get mtu\n");
  }

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_get_random_mac(char *value_str,
                                           const char *key_str,
                                           const sl_app_settings_entry_t *entry)
{
  const sid_ble_link_config_t *ble_config = app_get_ble_config();
  char s_random_mac[MAC_STRING_SIZE] = "";

  switch (ble_config->config->mac_addr_type) {
    case SID_BLE_CFG_MAC_ADDRESS_TYPE_PUBLIC:
      sprintf(s_random_mac, "PUBLIC");
      break;

    case SID_BLE_CFG_MAC_ADDRESS_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE:
      sprintf(s_random_mac, "RANDOM_PRIVATE_NON_RESOLVABLE");
      break;

    case SID_BLE_CFG_MAC_ADDRESS_TYPE_STATIC_RANDOM:
      sprintf(s_random_mac, "STATIC_RANDOM");
      break;

    case SID_BLE_CFG_MAC_ADDRESS_TYPE_RANDOM_PRIVATE_RESOLVABLE:
      sprintf(s_random_mac, "RANDOM_PRIVATE_RESOLVABLE");
      break;

    default:
      printf("[CLI] Unsupported BLE_CFG for MAC address.\n");
      sprintf(s_random_mac, "UNKOWN");
      break;
  }

  sl_app_settings_set_string(s_random_mac, key_str, entry);

  sl_app_settings_get_string(value_str, key_str, entry);

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_get_output_power(char *value_str,
                                             const char *key_str,
                                             const sl_app_settings_entry_t *entry)
{
  sl_app_settings_get_integer(value_str, key_str, entry);

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_get_fsk_power_profile(char *value_str,
                                                  const char *key_str,
                                                  const sl_app_settings_entry_t *entry)
{
  char s_dev_prof_id[DEVICE_PROFILE_ID_SIZE] = { 0 };
  app_setting_cli_queue_t settings = { 0 };

  sl_app_trigger_sid_get_fsk_dev_prof_id();

  // Wait for data from sidewalk thread
  if (xQueueReceive(g_cli_event_queue, &settings, pdMS_TO_TICKS(2000))) {
    switch (settings.device_profile.unicast_params.device_profile_id) {
      case SID_LINK2_PROFILE_1:
        sprintf(s_dev_prof_id, "1");
        break;

      case SID_LINK2_PROFILE_2:
        sprintf(s_dev_prof_id, "2");
        break;

      default:
        sprintf(s_dev_prof_id, "0");
        break;
    }

    // Write result to dev profile attribute
    sl_app_settings_set_string(s_dev_prof_id, key_str, entry);

    // Finally display value to user
    sl_app_settings_get_string(value_str, key_str, entry);
  } else {
    printf("Error while trying to get dev_profile_id\n");
  }

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_set_fsk_power_profile(const char *value_str,
                                                  const char *key_str,
                                                  const sl_app_settings_entry_t *entry)
{
  sl_status_t ret = SID_ERROR_NONE;
  (void)key_str;
  (void)entry;

  // Check if value wanted to be set is correct
  if (!(strcmp(value_str, (char *) "1\0") == 0 || strcmp(value_str, (char *) "2\0") == 0)) {
    printf("[ERROR] Trying to set an incorrect value for FSK power profile: %s\n" \
           "Correct values are: '1' or '2'\n", value_str);
    ret = SL_STATUS_FAIL;
  } else { // User has chosen a valid argument ...
    // Discard the end of line character '\0'
    char *c_power_profile_id = (char *)&value_str[0];

    // Trigger sidewalk main thread for setting the stack parameters
    sl_app_trigger_sid_set_fsk_dev_prof_id(c_power_profile_id);
    ret = SL_STATUS_OK;
  }

  return ret;
}

sl_status_t sl_app_settings_get_css_power_profile(char *value_str,
                                                  const char *key_str,
                                                  const sl_app_settings_entry_t *entry)
{
  char s_dev_prof_id[DEVICE_PROFILE_ID_SIZE] = { 0 };
  app_setting_cli_queue_t settings = { 0 };

  sl_app_trigger_sid_get_css_dev_prof_id();

  // Wait for data from sidewalk thread
  if (xQueueReceive(g_cli_event_queue, &settings, pdMS_TO_TICKS(2000))) {
    switch (settings.device_profile.unicast_params.device_profile_id) {
      case SID_LINK3_PROFILE_A:
        sprintf(s_dev_prof_id, "A");
        break;

      case SID_LINK3_PROFILE_B:
        sprintf(s_dev_prof_id, "B");
        break;

      case SID_LINK3_PROFILE_D:
        sprintf(s_dev_prof_id, "D");
        break;

      default:
        sprintf(s_dev_prof_id, "0");
        break;
    }

    // Write result to dev profile attribute
    sl_app_settings_set_string(s_dev_prof_id, key_str, entry);

    // Finally display value to user
    sl_app_settings_get_string(value_str, key_str, entry);
  } else {
    printf("Error while trying to get dev_profile_id\n");
  }

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_set_css_power_profile(const char *value_str,
                                                  const char *key_str,
                                                  const sl_app_settings_entry_t *entry)
{
  sl_status_t ret = SID_ERROR_NONE;
  (void)key_str;
  (void)entry;

  // Check if value wanted to be set is correct
  if (!(strcmp(value_str, (char *) "A\0") == 0
        || strcmp(value_str, (char *) "B\0") == 0)) {
    printf("[ERROR] Trying to set an incorrect value for CSS power profile: %s\n" \
           "Correct values are: 'A' or 'B''\n", value_str);
    ret = SL_STATUS_FAIL;
  } else { // User has chosen a valid argument ...
    // Discard the end of line character '\0'
    char *c_power_profile_id = (char *)&value_str[0];

    // Trigger sidewalk main thread for setting the stack parameters
    sl_app_trigger_sid_set_css_dev_prof_id(c_power_profile_id);

    ret = SL_STATUS_OK;
  }

  return ret;
}

sl_status_t sl_app_settings_set_offset(const char *value_str,
                                       const char *key_str,
                                       const sl_app_settings_entry_t *entry)
{
  sl_app_settings_set_integer(value_str, key_str, entry);

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_get_offset(char *value_str,
                                       const char *key_str,
                                       const sl_app_settings_entry_t *entry)
{
  sl_app_settings_get_integer(value_str, key_str, entry);

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_set_range(const char *value_str,
                                      const char *key_str,
                                      const sl_app_settings_entry_t *entry)
{
  sl_app_settings_set_integer(value_str, key_str, entry);

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_get_range(char *value_str,
                                      const char *key_str,
                                      const sl_app_settings_entry_t *entry)
{
  sl_app_settings_get_integer(value_str, key_str, entry);

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_get_beacon_interval(char *value_str,
                                                const char *key_str,
                                                const sl_app_settings_entry_t *entry)
{
  sl_app_settings_get_integer(value_str, key_str, entry);

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_get_fsk_rx_window_count(char *value_str,
                                                const char *key_str,
                                                const sl_app_settings_entry_t *entry)
{
  char s_rx_window_count[DEVICE_PROFILE_ID_SIZE] = { 0 };
  app_setting_cli_queue_t settings = { 0 };

  sl_app_trigger_sid_get_fsk_dev_prof_id();

  // Wait for data from sidewalk thread
  if (xQueueReceive(g_cli_event_queue, &settings, pdMS_TO_TICKS(2000))) {
    sprintf(s_rx_window_count, "%d", settings.device_profile.unicast_params.rx_window_count);

    // Write result to dev profile attribute
    sl_app_settings_set_integer(s_rx_window_count, key_str, entry);

    // Finally display value to user
    sl_app_settings_get_integer(value_str, key_str, entry);
  } else {
    printf("Error while trying to get dev_profile_id\n");
  }

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_get_fsk_rx_window_separation(char *value_str,
                                                     const char *key_str,
                                                     const sl_app_settings_entry_t *entry)
{
  char s_rx_window_separation[DEVICE_PROFILE_ID_SIZE] = { 0 };
  app_setting_cli_queue_t settings = { 0 };

  sl_app_trigger_sid_get_fsk_dev_prof_id();

  // Wait for data from sidewalk thread
  if (xQueueReceive(g_cli_event_queue, &settings, pdMS_TO_TICKS(2000))) {
    sprintf(s_rx_window_separation, "%d", settings.device_profile.unicast_params.unicast_window_interval.sync_rx_interval_ms);

    // Write result to dev profile attribute
    sl_app_settings_set_integer(s_rx_window_separation, key_str, entry);

    // Finally display value to user
    sl_app_settings_get_integer(value_str, key_str, entry);
  } else {
    printf("Error while trying to get dev_profile_id\n");
  }

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_get_css_rx_window_separation(char *value_str,
                                          const char *key_str,
                                          const sl_app_settings_entry_t *entry)
{
  char s_rx_window_separation[DEVICE_PROFILE_ID_SIZE] = { 0 };
  app_setting_cli_queue_t settings = { 0 };

  sl_app_trigger_sid_get_css_dev_prof_id();

  // Wait for data from sidewalk thread
  if (xQueueReceive(g_cli_event_queue, &settings, pdMS_TO_TICKS(2000))) {
    sprintf(s_rx_window_separation, "%d", settings.device_profile.unicast_params.unicast_window_interval.async_rx_interval_ms);

    // Write result to dev profile attribute
    sl_app_settings_set_integer(s_rx_window_separation, key_str, entry);

    // Finally display value to user
    sl_app_settings_get_integer(value_str, key_str, entry);
  } else {
    printf("Error while trying to get dev_profile_id\n");
  }

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_get_css_rx_window_count(char *value_str,
                                          const char *key_str,
                                          const sl_app_settings_entry_t *entry)
{
  char s_rx_window_count[DEVICE_PROFILE_ID_SIZE] = { 0 };
  app_setting_cli_queue_t settings = { 0 };

  sl_app_trigger_sid_get_css_dev_prof_id();

  // Wait for data from sidewalk thread
  if (xQueueReceive(g_cli_event_queue, &settings, pdMS_TO_TICKS(2000))) {
    sprintf(s_rx_window_count, "%d", settings.device_profile.unicast_params.rx_window_count);

    // Write result to dev profile attribute
    sl_app_settings_set_integer(s_rx_window_count, key_str, entry);

    // Finally display value to user
    sl_app_settings_get_integer(value_str, key_str, entry);
  } else {
    printf("Error while trying to get dev_profile_id\n");
  }

  return SL_STATUS_OK;
}

sl_status_t sl_app_settings_set_css_rx_window_count(const char *value_str,
                                          const char *key_str,
                                          const sl_app_settings_entry_t *entry)
{
  sl_status_t ret = SID_ERROR_NONE;
  (void)key_str;
  (void)entry;

  // Convert value_str into integer
  uint32_t sid_rx_window = atoi(value_str);

  // Check if value wanted to be set is correct
  if (!((sid_rx_window == SID_RX_WINDOW_CNT_2)
        || (sid_rx_window == SID_RX_WINDOW_CNT_3)
        || (sid_rx_window == SID_RX_WINDOW_CNT_4)
        || (sid_rx_window == SID_RX_WINDOW_CNT_5))) {
    printf("[ERROR] Trying to set an incorrect value for \"CSS power profile A\" rx window count: %s\n"
           "Correct values are: SID_RX_WINDOW_CNT_x, where x can be 2, 3, 4, 5'\n",
           value_str);
    ret = SL_STATUS_FAIL;
  } else { // User has chosen a valid argument ...
    // Trigger sidewalk main thread for setting the stack parameters
    sl_app_trigger_sid_set_dev_prof_rx_win_cnt(sid_rx_window);

    ret = SL_STATUS_OK;
  }

  return ret;
}

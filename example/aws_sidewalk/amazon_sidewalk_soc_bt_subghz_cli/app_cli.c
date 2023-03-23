/***************************************************************************//**
 * @file
 * @brief app_cli.c
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
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
#include <string.h>
#include <stdint.h>

#include "app_cli.h"
#include "sl_cli.h"
#include "app_process.h"
#include "app_init.h"
#include "app_cli_settings.h"
#include "app_log.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// Command arguments
char *argument_value_str = NULL;
char *argument_value_str_2 = NULL;

// Global because context is not accessible by CLI component
uint32_t cli_arg_uint32_t;
int16_t cli_arg_int16_t;
uint16_t cli_arg_uint16_t;
uint8_t cli_arg_uint8_t;
char cli_arg_str[64];
char cli_arg_str_2[64];

// Queue for sending data to cli
QueueHandle_t g_cli_event_queue;

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Functions Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/******************************************************************************
 * CLI - sid send <message_type> <payload>
 * Send a custom message to the cloud (Message types get/set/notify/response)
 *****************************************************************************/
void cli_sid_send(sl_cli_command_arg_t *arguments)
{
  argument_value_str = sl_cli_get_command_string(arguments, 2);
  argument_value_str_2 = sl_cli_get_command_string(arguments, 3);

  sl_app_trigger_sid_send(argument_value_str, argument_value_str_2);
}

/******************************************************************************
 * CLI - sid reset
 * Deregister Sidewalk device and returns to factory settings
 *****************************************************************************/
void cli_sid_reset(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_app_trigger_sid_reset();
}

/******************************************************************************
 * CLI - sid init <link>
 * Initialize Sidewalk stack for chosen communication link (FSK/CSS/BLE)
 * This function can only be called once. You have to use deinit to call it again.
 *****************************************************************************/
void cli_sid_init(sl_cli_command_arg_t *arguments)
{
  argument_value_str = sl_cli_get_command_string(arguments, 2);

  sl_app_trigger_sid_init(argument_value_str);
}

/******************************************************************************
 * CLI - sid start <link>
 * Start Sidewalk stack for chosen communication link
 *****************************************************************************/
void cli_sid_start(sl_cli_command_arg_t *arguments)
{
  argument_value_str = sl_cli_get_command_string(arguments, 2);

  sl_app_trigger_sid_start(argument_value_str);
}

/******************************************************************************
 * CLI - sid stop <link>
 * Stop Sidewalk stack for chosen communication link
 *****************************************************************************/
void cli_sid_stop(sl_cli_command_arg_t *arguments)
{
  argument_value_str = sl_cli_get_command_string(arguments, 2);

  sl_app_trigger_sid_stop(argument_value_str);
}

/******************************************************************************
 * CLI - sid deinit
 * Deinitialize Sidewalk stack
 *****************************************************************************/
void cli_sid_deinit(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_app_trigger_sid_deinit();
}

/******************************************************************************
 * CLI - sid bleconnect
 * Initiate ble bcn connection request
 *****************************************************************************/
void cli_sid_ble_connect(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_app_trigger_ble_connection_request();
}

/******************************************************************************
 * Get - sidewalk time
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns None
 *****************************************************************************/
void get_sidewalk_time(app_context_t *app_context)
{
  struct sid_timespec curr_time = { 0 };
  app_setting_cli_queue_t cli_settings;

  sid_error_t ret = sid_get_time(app_context->sidewalk_handle, SID_GET_GPS_TIME, &curr_time);
  if (ret == SID_ERROR_NONE) {
    // Send back to CLI time through cli_queue event
    cli_settings.current_time = curr_time;
    xQueueSend(g_cli_event_queue, &cli_settings, 0);
  } else {
    // Send empty data
    xQueueSend(g_cli_event_queue, &cli_settings, 0);
  }
}

/******************************************************************************
 * Get - sidewalk status
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns None
 *****************************************************************************/
void get_sidewalk_status(app_context_t *app_context)
{
  app_setting_cli_queue_t cli_settings = { 0 };
  struct sid_status current_status;

  sid_error_t ret = sid_get_status(app_context->sidewalk_handle, &current_status);
  if (ret == SID_ERROR_NONE) {
    // Send back to CLI state through cli_queue event
    cli_settings.current_status = current_status;
    xQueueSend(g_cli_event_queue, &cli_settings, 0);
  } else {
    // Send error
    cli_settings.current_status.state = SID_STATE_ERROR;
    xQueueSend(g_cli_event_queue, &cli_settings, 0);
  }
}

/******************************************************************************
 * Get - sidewalk mtu
 *
 * @param[in] context The context which is applicable for the current application
 * @param[in] link_type Link type
 * @returns None
 *****************************************************************************/
void get_sidewalk_mtu(app_context_t *context, enum sid_link_type link_type)
{
  size_t mtu;
  app_setting_cli_queue_t cli_settings = { 0 };

  // Get current
  sid_error_t ret = sid_get_mtu(context->sidewalk_handle, link_type, &mtu);

  if (ret == SID_ERROR_NONE) {
    cli_settings.mtu = mtu;
    xQueueSend(g_cli_event_queue, &cli_settings, 0);
  } else {
    app_log_error("Error getting MTU: %d\n", ret);
    xQueueSend(g_cli_event_queue, &cli_settings, 0);
  }
}

/******************************************************************************
 * Get - FSK dev profile id
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns None
 *****************************************************************************/
void get_sidewalk_fsk_dev_prof_id(app_context_t *app_context)
{
  app_setting_cli_queue_t cli_settings = { 0 };
  uint8_t arg1 = SID_LINK2_PROFILE_1;
  struct sid_device_profile dev_cfg = { .unicast_params = { .device_profile_id = arg1 } };

  sid_error_t ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg, sizeof(dev_cfg));
  if (ret == SID_ERROR_NONE) {
    cli_settings.device_profile = dev_cfg;
    xQueueSend(g_cli_event_queue, &cli_settings, 0);
  } else {
    app_log_error("App - error getting device profile: %d\n", ret);
    xQueueSend(g_cli_event_queue, &cli_settings, 0);
  }
}

/******************************************************************************
 * Get - CSS dev profile id
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns None
 *****************************************************************************/
void get_sidewalk_css_dev_prof_id(app_context_t *app_context)
{
  app_setting_cli_queue_t cli_settings = { 0 };
  uint8_t arg1 = SID_LINK3_PROFILE_A;
  struct sid_device_profile dev_cfg = { .unicast_params = { .device_profile_id = arg1 } };

  sid_error_t ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg, sizeof(dev_cfg));
  if (ret == SID_ERROR_NONE) {
    cli_settings.device_profile = dev_cfg;
    xQueueSend(g_cli_event_queue, &cli_settings, 0);
  } else {
    app_log_error("App - error getting device profile: %d", ret);
    xQueueSend(g_cli_event_queue, &cli_settings, 0);
  }
}

/******************************************************************************
 * Set - FSK dev profile id
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns None
 *****************************************************************************/
void set_sidewalk_fsk_dev_prof_id(app_context_t *app_context)
{
  struct sid_device_profile dev_cfg;

  // Other parameters must be set for sid_option() SET operation to succeed
  switch (cli_arg_uint8_t) {
    case '1':
      dev_cfg.unicast_params.device_profile_id = SID_LINK2_PROFILE_1;
      dev_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE;
      break;

    case '2':
      dev_cfg.unicast_params.device_profile_id = SID_LINK2_PROFILE_2;
      dev_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE;
      dev_cfg.unicast_params.unicast_window_interval.sync_rx_interval_ms = SID_LINK2_RX_WINDOW_SEPARATION_1;
      break;

    default:
      // We should not reach here, arg have already been checked in settings.c
      break;
  }

  sid_error_t ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
                               &dev_cfg, sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    app_log_error("App - error setting fsk_dev_prof_id, sid_option failed and returned: %d\n", ret);
  }
}

/******************************************************************************
 * Set - CSS dev profile id
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns None
 *****************************************************************************/
void set_sidewalk_css_dev_prof_id(app_context_t *app_context)
{
  struct sid_device_profile dev_cfg = { 0 };

  // Other parameters must be set for sid_option() SET operation to succeed
  // Check Power Profile datasheet for allowed value
  switch (cli_arg_uint8_t) {
    case 'A':
      dev_cfg.unicast_params.device_profile_id = SID_LINK3_PROFILE_A;
      dev_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_2;
      dev_cfg.unicast_params.unicast_window_interval.async_rx_interval_ms = SID_LINK3_RX_WINDOW_SEPARATION_3;
      break;

    case 'B':
      dev_cfg.unicast_params.device_profile_id = SID_LINK3_PROFILE_B;
      dev_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE;
      dev_cfg.unicast_params.unicast_window_interval.async_rx_interval_ms = SID_LINK3_RX_WINDOW_SEPARATION_3;
      break;

    default:
      // We should not reach here, arg have already been checked in settings.c
      break;
  }

  sid_error_t ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
                               &dev_cfg, sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    app_log_error("App - error setting css_dev_prof_id, sid_option failed and returned: %d\n", ret);
  }
}

/*******************************************************************************
 * Trigger - sid send
 * @param[in] message_type_str
 * @param[in] message_str
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_send(char *message_type_str, char *message_str)
{
  memset(cli_arg_str, 0, sizeof(cli_arg_str));
  memcpy(cli_arg_str, message_type_str, strlen(message_type_str));
  memset(cli_arg_str_2, 0, sizeof(cli_arg_str_2));
  memcpy(cli_arg_str_2, message_str, strlen(message_str));
  queue_event(g_event_queue, EVENT_TYPE_SID_SEND, true);

  app_log_info("App - send user event\n");
}

/*******************************************************************************
 * Trigger - sid reset
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_reset(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_RESET, true);
  app_log_info("App - reset user event");
}

/*******************************************************************************
 * Trigger - sid init
 * @param[in] link_str
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_init(char *link_str)
{
  memset(cli_arg_str, 0, sizeof(cli_arg_str));
  memcpy(cli_arg_str, link_str, strlen(link_str));
  queue_event(g_event_queue, EVENT_TYPE_SID_INIT, true);

  app_log_info("App - init user event\n");
}

/*******************************************************************************
 * Trigger - sid start
 * @param[in] link_str
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_start(char *link_str)
{
  memset(cli_arg_str, 0, sizeof(cli_arg_str));
  memcpy(cli_arg_str, link_str, strlen(link_str));
  queue_event(g_event_queue, EVENT_TYPE_SID_START, true);

  app_log_info("App - start user event\n");
}

/*******************************************************************************
 * Trigger - sid stop
 * @param[in] link_str
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_stop(char *link_str)
{
  memset(cli_arg_str, 0, sizeof(cli_arg_str));
  memcpy(cli_arg_str, link_str, strlen(link_str));
  queue_event(g_event_queue, EVENT_TYPE_SID_STOP, true);

  app_log_info("App - stop user event\n");
}

/*******************************************************************************
 * Trigger - sid deinit
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_deinit(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_DEINIT, true);
  app_log_info("App - deinit user event\n");
}

/*******************************************************************************
 * Trigger - sid get CSS dev profile id
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_get_css_dev_prof_id(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_GET_CSS_DEV_PROF_ID, true);
  app_log_info("App - get css dev prof id user event\n");
}

/*******************************************************************************
 * Trigger - sid set CSS dev profile id
 * @param[in] value
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_css_dev_prof_id(char *value)
{
  cli_arg_uint8_t = *value;
  queue_event(g_event_queue, EVENT_TYPE_SID_SET_CSS_DEV_PROF_ID, true);

  app_log_info("App - set css dev prof id user event\n");
}

/*******************************************************************************
 * Trigger - sid get FSK dev profile id
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_get_fsk_dev_prof_id(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_GET_FSK_DEV_PROF_ID, true);
  app_log_info("App - get fsk dev prof id user event\n");
}

/*******************************************************************************
 * Trigger - sid set FSK dev profile id
 * @param[in] value
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_fsk_dev_prof_id(char *value)
{
  cli_arg_uint8_t = *value;
  queue_event(g_event_queue, EVENT_TYPE_SID_SET_FSK_DEV_PROF_ID, true);

  app_log_info("App - set fsk dev prof id user event\n");
}

/*******************************************************************************
 * Trigger - sid set dev profile id
 * @param[in] profile_id
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_dev_prof_id(uint8_t profile_id)
{
  cli_arg_uint8_t = profile_id;
  queue_event(g_event_queue, EVENT_TYPE_SID_SET_DEV_PROF_ID, true);

  app_log_info("App - set dev prof id user event\n");
}

/*******************************************************************************
 * Trigger - sid get dev prof rx window count
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_get_dev_prof_rx_win_cnt(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_GET_DEV_PROF_RX_WIN_CNT, true);
  app_log_info("App - get dev prof rx win cnt user event\n");
}

/*******************************************************************************
 * Trigger - sid set dev prof rx window count
 * @param[in] rx_win_cnt
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_dev_prof_rx_win_cnt(int16_t rx_win_cnt)
{
  cli_arg_int16_t = rx_win_cnt;
  queue_event(g_event_queue, EVENT_TYPE_SID_SET_DEV_PROF_RX_WIN_CNT, true);

  app_log_info("App - set dev prof rx win cnt user event\n");
}

/*******************************************************************************
 * Trigger - sid get dev prof rx interval in ms
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_get_dev_prof_rx_interv_ms(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_GET_DEV_PROF_RX_INTERV_MS, true);
  app_log_info("App - get dev prof rx interv ms user event");
}

/*******************************************************************************
 * Trigger - sid set dev prof rx interval in ms
 * @param[in] rx_interv_ms
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_dev_prof_rx_interv_ms(uint16_t rx_interv_ms)
{
  cli_arg_uint16_t = rx_interv_ms;
  queue_event(g_event_queue, EVENT_TYPE_SID_SET_DEV_PROF_RX_INTERV_MS, true);

  app_log_info("App - set dev prof rx interv ms user event\n");
}

/*******************************************************************************
 * Trigger - sid get dev prof wakeup type
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_get_dev_prof_wakeup_type(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_GET_DEV_PROF_WAKEUP_TYPE, true);
  app_log_info("App - get dev prof wakeup type user event\n");
}

/*******************************************************************************
 * Trigger - sid set dev prof wakeup type
 * @param[in] wakeup_type
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_dev_prof_wakeup_type(uint8_t wakeup_type)
{
  cli_arg_uint8_t = wakeup_type;
  queue_event(g_event_queue, EVENT_TYPE_SID_SET_DEV_PROF_WAKEUP_TYPE, true);

  app_log_info("App - set dev prof wakeup type user event\n");
}

/*******************************************************************************
 * Trigger - BLE connection request
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_ble_connection_request(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_BLE_CONNECTION_REQUEST, true);
  app_log_info("App - ble connection request user event\n");
}

/*******************************************************************************
 * Trigger send counter update
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_send_counter_update(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SEND_COUNTER_UPDATE, true);
  app_log_info("App - send counter update user event\n");
}

/*******************************************************************************
 * Trigger factory reset
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_factory_reset(void)
{
  queue_event(g_event_queue, EVENT_TYPE_FACTORY_RESET, true);
  app_log_info("App - factory reset user event\n");
}

/*******************************************************************************
 * Trigger send
 * @param[in] len Length of the argument
 * @returns None
 ******************************************************************************/
void sl_app_trigger_send(uint32_t len)
{
  cli_arg_uint32_t = len;
  queue_event(g_event_queue, EVENT_TYPE_SID_SEND, true);

  app_log_info("App - send user event\n");
}

/*******************************************************************************
 * Trigger get time
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_get_time(void)
{
  queue_event(g_event_queue, EVENT_TYPE_GET_TIME, true);
  app_log_info("App - get time user event\n");
}

/*******************************************************************************
 * Trigger get status
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_get_status(void)
{
  queue_event(g_event_queue, EVENT_TYPE_GET_STATUS, true);
  app_log_info("App - get status user event\n");
}

/*******************************************************************************
 * Trigger get mtu
 * @param[in] link_type Link type
 * @returns None
 ******************************************************************************/
void sl_app_trigger_get_mtu(enum sid_link_type link_type)
{
  switch (link_type) {
    case SID_LINK_TYPE_1:
      queue_event(g_event_queue, EVENT_TYPE_GET_MTU_BLE, true);
      break;

    case SID_LINK_TYPE_2:
      queue_event(g_event_queue, EVENT_TYPE_GET_MTU_FSK, true);
      break;

    case SID_LINK_TYPE_3:
      queue_event(g_event_queue, EVENT_TYPE_GET_MTU_CSS, true);
      break;

    default:
      // Link type is not valid, nothing to do
      break;
  }

  app_log_info("App - get mtu user event\n");
}

/*******************************************************************************
 * Set - dev profile id
 * @param[in] app_context
 * @param[in] id
 * @returns None
 ******************************************************************************/
void set_sidewalk_dev_prof_id(app_context_t *app_context, uint8_t id)
{
  struct sid_device_profile dev_cfg = {};
  sid_error_t ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg, sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    app_log_error("App - error getting device profile: %d\n", ret);
  } else {
    dev_cfg.unicast_params.device_profile_id = id;
    ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
                     &dev_cfg, sizeof(dev_cfg));
    if (ret != SID_ERROR_NONE) {
      app_log_error("App - error setting device profile: %d\n", ret);
    } else {
      app_log_info("App - device profile ID set\n");
    }
  }
}

/*******************************************************************************
 * Get - dev profile rx window count
 * @param[in] app_context
 * @returns None
 ******************************************************************************/
void get_sidewalk_dev_prof_rx_win_cnt(app_context_t *app_context)
{
  struct sid_device_profile dev_cfg = {};
  sid_error_t ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg, sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    app_log_error("App - error getting device profile: %d\n", ret);
  } else {
    app_log_info("App - device profile RX window count: %d\n", dev_cfg.unicast_params.rx_window_count);
  }
}

/*******************************************************************************
 * Set - dev profile rx window count
 * @param[in] app_context
 * @param[in] rx_win_cnt
 * @returns None
 ******************************************************************************/
void set_sidewalk_dev_prof_rx_win_cnt(app_context_t *app_context, uint16_t rx_win_cnt)
{
  struct sid_device_profile dev_cfg = {};
  sid_error_t ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg, sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    app_log_error("App - error getting device profile: %d\n", ret);
  } else {
    dev_cfg.unicast_params.rx_window_count = rx_win_cnt;
    ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
                     &dev_cfg, sizeof(dev_cfg));
    if (ret != SID_ERROR_NONE) {
      app_log_error("App - error setting device profile: %d\n", ret);
    } else {
      app_log_info("App - device profile RX window count set\n");
    }
  }
}

/*******************************************************************************
 * Get - dev profile rx interval in ms
 * @param[in] app_context
 * @returns None
 ******************************************************************************/
void get_sidewalk_dev_prof_rx_interv_ms(app_context_t *app_context)
{
  struct sid_device_profile dev_cfg = {};
  sid_error_t ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg, sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    app_log_error("App - error getting device profile: %d\n", ret);
  } else {
    // As both members of the union has same type (uint16_t) then it's not important which member is read.
    app_log_info("App - device profile RX window separation interval in ms: %d\n",
                 dev_cfg.unicast_params.unicast_window_interval.sync_rx_interval_ms);
  }
}

/*******************************************************************************
 * Set - dev profile rx interval ms
 * @param[in] app_context
 * @param[in] rx_interv_ms
 * @returns None
 ******************************************************************************/
void set_sidewalk_dev_prof_rx_interv_ms(app_context_t *app_context, uint16_t rx_interv_ms)
{
  struct sid_device_profile dev_cfg = {};
  sid_error_t ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg, sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    app_log_error("App - error getting device profile: %d\n", ret);
  } else {
    // As both members of the union has same type (uint16_t) then it's not important which member is read.
    dev_cfg.unicast_params.unicast_window_interval.sync_rx_interval_ms = rx_interv_ms;
    ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
                     &dev_cfg, sizeof(dev_cfg));
    if (ret != SID_ERROR_NONE) {
      app_log_error("App - error setting device profile: %d\n", ret);
    } else {
      app_log_info("App - device profile RX window separation interval in ms set\n");
    }
  }
}

/*******************************************************************************
 * Get - dev profile wakeup type
 * @param[in] app_context
 * @returns None
 ******************************************************************************/
void get_sidewalk_dev_prof_wakeup_type(app_context_t *app_context)
{
  struct sid_device_profile dev_cfg = {};
  sid_error_t ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg, sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    app_log_error("App - error getting device profile: %d\n", ret);
  } else {
    app_log_info("App - device profile wakeup type: %d\n", dev_cfg.unicast_params.wakeup_type);
  }
}

/*******************************************************************************
 * Set - dev profile wakeup type
 * @param[in] app_context
 * @param[in] wakeup_type
 * @returns None
 ******************************************************************************/
void set_sidewalk_dev_prof_wakeup_type(app_context_t *app_context, uint8_t wakeup_type)
{
  struct sid_device_profile dev_cfg = {};
  sid_error_t ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg, sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    app_log_error("App - error getting device profile: %d\n", ret);
  } else {
    dev_cfg.unicast_params.wakeup_type = wakeup_type;
    ret = sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
                     &dev_cfg, sizeof(dev_cfg));
    if (ret != SID_ERROR_NONE) {
      app_log_error("App - error setting device profile: %d\n", ret);
    } else {
      app_log_info("App - device profile wakeup type set\n");
    }
  }
}

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
#include "sl_sidewalk_log_app.h"
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
char *argument_value_str_3 = NULL;

// Global because context is not accessible by CLI component
uint32_t cli_arg_uint32_t = 0UL;
int16_t cli_arg_int16_t = 0;
uint16_t cli_arg_uint16_t = 0U;
uint8_t cli_arg_uint8_t = 0U;
struct sid_link_auto_connect_params cli_arg_sid_link_auto_connect_params = { 0 };
// note: this will limit the max size of uplink payload inputed from cli (BLE, FSK)
char cli_arg_str[64] = { 0 };
char cli_arg_str_2[256] = { 0 };
char cli_arg_str_3[16] = { 0 };

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
  int arg_count = sl_cli_get_argument_count(arguments);

  argument_value_str = sl_cli_get_command_string(arguments, 2);
  argument_value_str_2 = sl_cli_get_command_string(arguments, 3);

  argument_value_str_3 = NULL;
  if (arg_count == 3) {
    argument_value_str_3 = sl_cli_get_command_string(arguments, 4);
  }

  sl_app_trigger_sid_send(argument_value_str, argument_value_str_2, argument_value_str_3);
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

#if defined(SID_SDK_INTERNAL_CONFIG_ENABLE_DULT_QA)
/******************************************************************************
 * CLI - dult init
 * Initialize Sidewalk DULT-S
 * This function can only be called once. You have to use deinit to call it again.
 *****************************************************************************/
void cli_dult_init(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_app_trigger_dult_init();
}

/******************************************************************************
 * CLI - dult status
 * Get status of Sidewalk DULT-S
 *****************************************************************************/
void cli_dult_status(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_app_trigger_dult_status();
}

/******************************************************************************
 * CLI - dult deinit
 * Deinitialize Sidewalk DULT-S
 *****************************************************************************/
void cli_dult_deinit(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_app_trigger_dult_deinit();
}
#endif

/******************************************************************************
 * CLI - sid bleconnect
 * Initiate ble bcn connection request
 *****************************************************************************/
void cli_sid_ble_connect(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_app_trigger_ble_connection_request();
}

#if defined(SL_SIDEWALK_OTA_DFU_PRESENT)
/******************************************************************************
 * CLI - sid ota_dfu_init
 * Initiate OTA DFU service
 *****************************************************************************/
void cli_ota_dfu_init(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_app_trigger_ota_dfu_init_request();
}

/******************************************************************************
 * CLI - sid ota_dfu_deinit
 * Deinitiate OTA DFU service
 *****************************************************************************/
void cli_ota_dfu_deinit(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_app_trigger_ota_dfu_deinit_request();
}

/******************************************************************************
 * CLI - sid ota_dfu_cancel
 * Cancel the ongoing OTA DFU process
 *****************************************************************************/
void cli_ota_dfu_cancel(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_app_trigger_ota_dfu_cancel_request();
}

/******************************************************************************
 * CLI - sid ota_dfu_stat
 * Stats for the ongoing OTA DFU process
 *****************************************************************************/
void cli_ota_dfu_stat(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_app_trigger_ota_dfu_stat_request();
}

/******************************************************************************
 * CLI - sid ota_dfu_param
 * Parameters of the ongoing OTA DFU process
 *****************************************************************************/
void cli_ota_dfu_param(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_app_trigger_ota_dfu_param_request();
}

/******************************************************************************
 * CLI - sid ota_dfu_min_scratch_buf_size
 * Minimum scratch buffer size requirements of OTA DFU service
 *****************************************************************************/
void cli_ota_dfu_min_scratch_buf_size(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_app_trigger_ota_dfu_min_scratch_buf_size_request();
}
#endif

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

  if (!app_context) {
    return;
  }

  if (sid_get_time(app_context->sidewalk_handle,
                   SID_GET_GPS_TIME,
                   &curr_time) == SID_ERROR_NONE) {
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

  if (!app_context) {
    return;
  }

  if (sid_get_status(app_context->sidewalk_handle,
                     &current_status) == SID_ERROR_NONE) {
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

  if (!context) {
    return;
  }

  // Get current
  sid_error_t ret = sid_get_mtu(context->sidewalk_handle, link_type, &mtu);

  if (ret == SID_ERROR_NONE) {
    cli_settings.mtu = mtu;
    xQueueSend(g_cli_event_queue, &cli_settings, 0);
  } else {
    SL_SID_LOG_APP_ERROR("get MTU failed, error: %d", ret);
    xQueueSend(g_cli_event_queue, &cli_settings, 0);
  }
}

/******************************************************************************
 * Set - link connection policy
 *
 * @param[in] context The context which is applicable for the current application
 * @param[in] policy Link connection policy
 * @returns None
 *****************************************************************************/
void set_link_connection_policy(app_context_t *context, uint8_t policy)
{
  if (!context) {
    return;
  }

  sid_error_t ret = sid_option(context->sidewalk_handle,
                               SID_OPTION_SET_LINK_CONNECTION_POLICY,
                               &policy,
                               sizeof(enum sid_link_connection_policy));

  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk connection policy set failed, error: %d", ret);
  }
}

/******************************************************************************
 * Get - link connection policy
 *
 * @param[in] context The context which is applicable for the current application
 * @returns None
 *****************************************************************************/
void get_link_connection_policy(app_context_t *context)
{
  enum sid_link_connection_policy policy;
  app_setting_cli_queue_t cli_settings = { 0 };

  if (!context) {
    return;
  }

  sid_error_t ret = sid_option(context->sidewalk_handle,
                               SID_OPTION_GET_LINK_CONNECTION_POLICY,
                               &policy,
                               sizeof(enum sid_link_connection_policy));

  if (ret == SID_ERROR_NONE) {
    cli_settings.link_connection_policy = policy;
  } else {
    SL_SID_LOG_APP_ERROR("sidewalk connection policy get failed, error: %d", ret);
  }
  xQueueSend(g_cli_event_queue, &cli_settings, 0);
}

/******************************************************************************
 * Set - multi-link policy
 *
 * @param[in] context The context which is applicable for the current application
 * @param[in] policy Multi-link policy
 * @returns None
 *****************************************************************************/
void set_multi_link_policy(app_context_t *context, uint8_t policy)
{
  if (!context) {
    return;
  }

  sid_error_t ret = sid_option(context->sidewalk_handle,
                               SID_OPTION_SET_LINK_POLICY_MULTI_LINK_POLICY,
                               &policy,
                               sizeof(enum sid_link_multi_link_policy));

  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk multi-link policy set failed, error: %d", ret);
  }
}

/******************************************************************************
 * Get - multi-link connection policy
 *
 * @param[in] context The context which is applicable for the current application
 * @returns None
 *****************************************************************************/
void get_multi_link_policy(app_context_t *context)
{
  enum sid_link_multi_link_policy policy;
  app_setting_cli_queue_t cli_settings = { 0 };

  if (!context) {
    return;
  }

  sid_error_t ret = sid_option(context->sidewalk_handle,
                               SID_OPTION_GET_LINK_POLICY_MULTI_LINK_POLICY,
                               &policy,
                               sizeof(enum sid_link_multi_link_policy));

  if (ret == SID_ERROR_NONE) {
    cli_settings.multi_link_policy = policy;
  } else {
    SL_SID_LOG_APP_ERROR("sidewalk multi-link policy get failed, error: %d", ret);
  }
  xQueueSend(g_cli_event_queue, &cli_settings, 0);
}

/******************************************************************************
 * Set - auto connect parameters
 *
 * @param[in] context The context which is applicable for the current application
 * @param[in] params Auto connect parameters
 * @returns None
 *****************************************************************************/
void set_auto_connect_params(app_context_t *context, struct sid_link_auto_connect_params params)
{
  if (!context) {
    return;
  }
  sid_error_t ret = sid_option(context->sidewalk_handle,
                               SID_OPTION_SET_LINK_POLICY_AUTO_CONNECT_PARAMS,
                               &params,
                               sizeof(struct sid_link_auto_connect_params));

  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk auto connect policy set failed, error: %d", ret);
  }
}

/******************************************************************************
 * Get - auto connect parameters
 *
 * @param[in] context The context which is applicable for the current application
 * @returns None
 *****************************************************************************/
void get_auto_connect_params(app_context_t *context)
{
  app_setting_cli_queue_t cli_settings = { 0 };

  if (!context) {
    return;
  }

  cli_settings.auto_connect_params[0].link_type = SID_LINK_TYPE_1;
  sid_error_t ret = sid_option(context->sidewalk_handle,
                               SID_OPTION_GET_LINK_POLICY_AUTO_CONNECT_PARAMS,
                               &cli_settings.auto_connect_params[0],
                               sizeof(struct sid_link_auto_connect_params));
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk auto connect policy get failed (BLE), error: %d", ret);
  }
  cli_settings.auto_connect_params[1].link_type = SID_LINK_TYPE_2;
  ret = sid_option(context->sidewalk_handle,
                   SID_OPTION_GET_LINK_POLICY_AUTO_CONNECT_PARAMS,
                   &cli_settings.auto_connect_params[1],
                   sizeof(struct sid_link_auto_connect_params));
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk auto connect policy get failed (FSK), error: %d", ret);
  }
  cli_settings.auto_connect_params[2].link_type = SID_LINK_TYPE_3;
  ret = sid_option(context->sidewalk_handle,
                   SID_OPTION_GET_LINK_POLICY_AUTO_CONNECT_PARAMS,
                   &cli_settings.auto_connect_params[2],
                   sizeof(struct sid_link_auto_connect_params));
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk auto connect policy get failed (CSS), error: %d", ret);
  }

  xQueueSend(g_cli_event_queue, &cli_settings, 0);
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
  struct sid_device_profile dev_cfg = {
    .unicast_params = {
      .device_profile_id = arg1
    }
  };

  if (!app_context) {
    return;
  }

  sid_error_t ret = sid_option(app_context->sidewalk_handle,
                               SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg,
                               sizeof(dev_cfg));
  if (ret == SID_ERROR_NONE) {
    cli_settings.device_profile = dev_cfg;
    xQueueSend(g_cli_event_queue, &cli_settings, 0);
  } else {
    SL_SID_LOG_APP_ERROR("sidewalk device profile get failed, error: %d", ret);
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
  struct sid_device_profile dev_cfg = {
    .unicast_params = {
      .device_profile_id = arg1
    }
  };

  if (!app_context) {
    return;
  }

  sid_error_t ret = sid_option(app_context->sidewalk_handle,
                               SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg,
                               sizeof(dev_cfg));
  if (ret == SID_ERROR_NONE) {
    cli_settings.device_profile = dev_cfg;
    xQueueSend(g_cli_event_queue, &cli_settings, 0);
  } else {
    SL_SID_LOG_APP_ERROR("sidewalk device profile get failed, error: %d", ret);
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

  if (!app_context) {
    return;
  }

  // Other parameters must be set for sid_option() SET operation to succeed
  switch (cli_arg_uint8_t) {
    case '1':
      dev_cfg.unicast_params.device_profile_id = SID_LINK2_PROFILE_1;
      dev_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE;
      dev_cfg.unicast_params.wakeup_type = SID_TX_AND_RX_WAKEUP;
      break;

    case '2':
      dev_cfg.unicast_params.device_profile_id = SID_LINK2_PROFILE_2;
      dev_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE;
      dev_cfg.unicast_params.unicast_window_interval.sync_rx_interval_ms = SID_LINK2_RX_WINDOW_SEPARATION_1;
      dev_cfg.unicast_params.wakeup_type = SID_TX_AND_RX_WAKEUP;
      break;

    default:
      // We should not reach here, arg have already been checked in settings.c
      break;
  }

  sid_error_t ret = sid_option(app_context->sidewalk_handle,
                               SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
                               &dev_cfg,
                               sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk device profile set failed, error: %d", ret);
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

  if (!app_context) {
    return;
  }

  // Other parameters must be set for sid_option() SET operation to succeed
  // Check Power Profile datasheet for allowed value
  switch (cli_arg_uint8_t) {
    case 'A':
      dev_cfg.unicast_params.device_profile_id = SID_LINK3_PROFILE_A;
      dev_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_2;
      dev_cfg.unicast_params.unicast_window_interval.async_rx_interval_ms = SID_LINK3_RX_WINDOW_SEPARATION_3;
      dev_cfg.unicast_params.wakeup_type = SID_TX_AND_RX_WAKEUP;
      break;

    case 'B':
      dev_cfg.unicast_params.device_profile_id = SID_LINK3_PROFILE_B;
      dev_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE;
      dev_cfg.unicast_params.unicast_window_interval.async_rx_interval_ms = SID_LINK3_RX_WINDOW_SEPARATION_3;
      dev_cfg.unicast_params.wakeup_type = SID_TX_AND_RX_WAKEUP;
      break;

    default:
      // We should not reach here, arg have already been checked in settings.c
      break;
  }

  sid_error_t ret = sid_option(app_context->sidewalk_handle,
                               SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
                               &dev_cfg,
                               sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk device profile set failed, error: %d", ret);
  }
}

/*******************************************************************************
 * Trigger - sid send
 * @param[in] message_type_str
 * @param[in] message_str
 * @param[in] link_type
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_send(char *message_type_str, char *message_str, char *link_type)
{
  if (message_type_str == NULL || message_str == NULL) {
    SL_SID_LOG_APP_ERROR("cli message type or message is NULL");
    return;
  }
  if (strlen(message_type_str) > sizeof(cli_arg_str)) {
    SL_SID_LOG_APP_ERROR("cli message type max string size reached, strlen: %u, size: %u",
                         strlen(message_type_str), sizeof(cli_arg_str));
    return;
  }
  if (strlen(message_str) > sizeof(cli_arg_str_2)) {
    SL_SID_LOG_APP_ERROR("cli message type max string size reached, strlen: %u, size: %u",
                         strlen(message_str), sizeof(cli_arg_str_2));
    return;
  }
  memset(cli_arg_str, 0, sizeof(cli_arg_str));
  memset(cli_arg_str_2, 0, sizeof(cli_arg_str_2));
  memset(cli_arg_str_3, 0, sizeof(cli_arg_str_3));
  memcpy(cli_arg_str, message_type_str, strlen(message_type_str));
  memcpy(cli_arg_str_2, message_str, strlen(message_str));
  // optional arg
  if (link_type != NULL) {
    if (strlen(link_type) > sizeof(cli_arg_str_3)) {
      SL_SID_LOG_APP_ERROR("cli link type max string size reached, strlen: %u, size: %u",
                           strlen(link_type), sizeof(cli_arg_str_3));
      return;
    }
    memcpy(cli_arg_str_3, link_type, strlen(link_type));
  }
  queue_event(g_event_queue, EVENT_TYPE_SID_SEND);

  SL_SID_LOG_APP_INFO("send event");
}
/*******************************************************************************
 * Trigger - sid reset
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_reset(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_RESET);
  SL_SID_LOG_APP_INFO("factory reset event");
}

/*******************************************************************************
 * Trigger - sid init
 * @param[in] link_str
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_init(char *link_str)
{
  if (link_str == NULL) {
    SL_SID_LOG_APP_ERROR("cli link type is NULL");
    return;
  }
  memset(cli_arg_str, 0, sizeof(cli_arg_str));
  memcpy(cli_arg_str, link_str, strlen(link_str));
  queue_event(g_event_queue, EVENT_TYPE_SID_INIT);

  SL_SID_LOG_APP_INFO("sidewalk init event");
}

/*******************************************************************************
 * Trigger - sid start
 * @param[in] link_str
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_start(char *link_str)
{
  if (link_str == NULL) {
    SL_SID_LOG_APP_ERROR("cli link type is NULL");
    return;
  }
  memset(cli_arg_str, 0, sizeof(cli_arg_str));
  memcpy(cli_arg_str, link_str, strlen(link_str));
  queue_event(g_event_queue, EVENT_TYPE_SID_START);

  SL_SID_LOG_APP_INFO("sidewalk start event");
}

/*******************************************************************************
 * Trigger - sid stop
 * @param[in] link_str
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_stop(char *link_str)
{
  if (link_str == NULL) {
    SL_SID_LOG_APP_ERROR("cli link type is NULL");
    return;
  }
  memset(cli_arg_str, 0, sizeof(cli_arg_str));
  memcpy(cli_arg_str, link_str, strlen(link_str));
  queue_event(g_event_queue, EVENT_TYPE_SID_STOP);

  SL_SID_LOG_APP_INFO("sidewalk stop event");
}

/*******************************************************************************
 * Trigger - sid deinit
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_deinit(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_DEINIT);
  SL_SID_LOG_APP_INFO("sidewalk deinit event");
}

#if defined(SID_SDK_INTERNAL_CONFIG_ENABLE_DULT_QA)
/*******************************************************************************
 * Trigger - dult init
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_dult_init(void)
{
  queue_event(g_event_queue, EVENT_TYPE_DULT_INIT);
  SL_SID_LOG_APP_INFO("dult init event");
}

/*******************************************************************************
 * Trigger - dult start
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_dult_start(void)
{
  queue_event(g_event_queue, EVENT_TYPE_DULT_START);
  SL_SID_LOG_APP_INFO("dult start event");
}

/*******************************************************************************
 * Trigger - dult stop
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_dult_stop(void)
{
  queue_event(g_event_queue, EVENT_TYPE_DULT_STOP);
  SL_SID_LOG_APP_INFO("dult stop event");
}

/*******************************************************************************
 * Trigger - dult status
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_dult_status(void)
{
  queue_event(g_event_queue, EVENT_TYPE_DULT_STATUS);
  SL_SID_LOG_APP_INFO("dult status event");
}

/*******************************************************************************
 * Trigger - dult deinit
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_dult_deinit(void)
{
  queue_event(g_event_queue, EVENT_TYPE_DULT_DEINIT);
  SL_SID_LOG_APP_INFO("dult deinit event");
}
#endif

/*******************************************************************************
 * Trigger - sid get CSS dev profile id
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_get_css_dev_prof_id(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_GET_CSS_DEV_PROF_ID);
  SL_SID_LOG_APP_INFO("sidewalk get CSS device profile id event");
}

/*******************************************************************************
 * Trigger - sid set CSS dev profile id
 * @param[in] value
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_css_dev_prof_id(char *value)
{
  if (value == NULL) {
    return;
  }

  cli_arg_uint8_t = *value;
  queue_event(g_event_queue, EVENT_TYPE_SID_SET_CSS_DEV_PROF_ID);

  SL_SID_LOG_APP_INFO("sidewalk set CSS device profile id event");
}

/*******************************************************************************
 * Trigger - sid get FSK dev profile id
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_get_fsk_dev_prof_id(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_GET_FSK_DEV_PROF_ID);
  SL_SID_LOG_APP_INFO("sidewalk get FSK device profile id event");
}

/*******************************************************************************
 * Trigger - sid set FSK dev profile id
 * @param[in] value
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_fsk_dev_prof_id(char *value)
{
  if (value == NULL) {
    return;
  }

  cli_arg_uint8_t = *value;
  queue_event(g_event_queue, EVENT_TYPE_SID_SET_FSK_DEV_PROF_ID);

  SL_SID_LOG_APP_INFO("sidewalk set FSK device profile id event");
}

/*******************************************************************************
 * Trigger - sid set dev profile id
 * @param[in] profile_id
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_dev_prof_id(uint8_t profile_id)
{
  cli_arg_uint8_t = profile_id;
  queue_event(g_event_queue, EVENT_TYPE_SID_SET_DEV_PROF_ID);

  SL_SID_LOG_APP_INFO("sidewalk set device profile id event");
}

/*******************************************************************************
 * Trigger - sid get dev prof rx window count
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_get_dev_prof_rx_win_cnt(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_GET_DEV_PROF_RX_WIN_CNT);
  SL_SID_LOG_APP_INFO("sidewalk get device profile RX window count event");
}

/*******************************************************************************
 * Trigger - sid set dev prof rx window count
 * @param[in] rx_win_cnt
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_dev_prof_rx_win_cnt(int16_t rx_win_cnt)
{
  cli_arg_int16_t = rx_win_cnt;
  queue_event(g_event_queue, EVENT_TYPE_SID_SET_DEV_PROF_RX_WIN_CNT);

  SL_SID_LOG_APP_INFO("sidewalk set device profile RX window count event");
}

/*******************************************************************************
 * Trigger - sid get dev prof rx interval in ms
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_get_dev_prof_rx_interv_ms(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_GET_DEV_PROF_RX_INTERV_MS);
  SL_SID_LOG_APP_INFO("sidewalk get device profile RX interval ms event");
}

/*******************************************************************************
 * Trigger - sid set dev prof rx interval in ms
 * @param[in] rx_interv_ms
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_dev_prof_rx_interv_ms(uint16_t rx_interv_ms)
{
  cli_arg_uint16_t = rx_interv_ms;
  queue_event(g_event_queue, EVENT_TYPE_SID_SET_DEV_PROF_RX_INTERV_MS);

  SL_SID_LOG_APP_INFO("sidewalk set device profile RX interval ms event");
}

/*******************************************************************************
 * Trigger - sid get dev prof wakeup type
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_get_dev_prof_wakeup_type(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_GET_DEV_PROF_WAKEUP_TYPE);
  SL_SID_LOG_APP_INFO("sidewalk get device profile wakeup type event");
}

/*******************************************************************************
 * Trigger - sid set dev prof wakeup type
 * @param[in] wakeup_type
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_dev_prof_wakeup_type(uint8_t wakeup_type)
{
  cli_arg_uint8_t = wakeup_type;
  queue_event(g_event_queue, EVENT_TYPE_SID_SET_DEV_PROF_WAKEUP_TYPE);

  SL_SID_LOG_APP_INFO("sidewalk set device profile wakeup type event");
}

/*******************************************************************************
 * Trigger - BLE connection request
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_ble_connection_request(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_BLE_CONNECTION_REQUEST);
  SL_SID_LOG_APP_INFO("BLE connection request event");
}

#if defined(SL_SIDEWALK_OTA_DFU_PRESENT)
void sl_app_trigger_ota_dfu_init_request(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_OTA_DFU_INIT_REQUEST);
  SL_SID_LOG_APP_INFO("OTA DFU init request event");
}

void sl_app_trigger_ota_dfu_deinit_request(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_OTA_DFU_DEINIT_REQUEST);
  SL_SID_LOG_APP_INFO("OTA DFU deinit request event");
}

void sl_app_trigger_ota_dfu_cancel_request(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_OTA_DFU_CANCEL_REQUEST);
  SL_SID_LOG_APP_INFO("OTA DFU cancel request event");
}

void sl_app_trigger_ota_dfu_stat_request(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_OTA_DFU_STAT_REQUEST);
  SL_SID_LOG_APP_INFO("OTA DFU stat request event");
}

void sl_app_trigger_ota_dfu_param_request(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_OTA_DFU_PARAM_REQUEST);
  SL_SID_LOG_APP_INFO("OTA DFU param request event");
}

void sl_app_trigger_ota_dfu_min_scratch_buf_size_request(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_OTA_DFU_MIN_SCRATCH_BUF_SIZE_REQUEST);
  SL_SID_LOG_APP_INFO("OTA DFUu min scratch buffer size request event");
}
#endif

/*******************************************************************************
 * Trigger get time
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_get_time(void)
{
  queue_event(g_event_queue, EVENT_TYPE_GET_TIME);
  SL_SID_LOG_APP_INFO("get time event");
}

/*******************************************************************************
 * Trigger get status
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_get_status(void)
{
  queue_event(g_event_queue, EVENT_TYPE_GET_STATUS);
  SL_SID_LOG_APP_INFO("get status event");
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
      queue_event(g_event_queue, EVENT_TYPE_GET_MTU_BLE);
      break;

    case SID_LINK_TYPE_2:
      queue_event(g_event_queue, EVENT_TYPE_GET_MTU_FSK);
      break;

    case SID_LINK_TYPE_3:
      queue_event(g_event_queue, EVENT_TYPE_GET_MTU_CSS);
      break;

    default:
      // Link type is not valid, nothing to do
      break;
  }

  SL_SID_LOG_APP_INFO("get MTU event");
}

/*******************************************************************************
 * Trigger set link connection policy
 * @param[in] policy Link connection policy
 * @returns None
 ******************************************************************************/
void sl_app_trigger_set_link_connection_policy(enum sid_link_connection_policy policy)
{
  cli_arg_uint8_t = (uint8_t)policy;
  queue_event(g_event_queue, EVENT_TYPE_SET_LINK_CONNECTION_POLICY);

  SL_SID_LOG_APP_INFO("set link connection policy event");
}

/*******************************************************************************
 * Trigger get link connection policy
 * @returns None
 ******************************************************************************/
void sl_app_trigger_get_link_connection_policy(void)
{
  queue_event(g_event_queue, EVENT_TYPE_GET_LINK_CONNECTION_POLICY);

  SL_SID_LOG_APP_INFO("get link connection policy event");
}

/*******************************************************************************
 * Trigger set multi-link policy
 * @param[in] policy Multi-link policy
 * @returns None
 ******************************************************************************/
void sl_app_trigger_set_multi_link_policy(enum sid_link_multi_link_policy policy)
{
  cli_arg_uint8_t = (uint8_t)policy;
  queue_event(g_event_queue, EVENT_TYPE_SET_MULTI_LINK_POLICY);

  SL_SID_LOG_APP_INFO("set multi-link policy event");
}

/*******************************************************************************
 * Trigger get multi-link policy
 * @returns None
 ******************************************************************************/
void sl_app_trigger_get_multi_link_policy(void)
{
  queue_event(g_event_queue, EVENT_TYPE_GET_MULTI_LINK_POLICY);

  SL_SID_LOG_APP_INFO("get multi-link policy event");
}

/*******************************************************************************
 * Trigger set auto connect parameters
 * @param[in] params Auto connect parameters
 * @returns None
 ******************************************************************************/
void sl_app_trigger_set_auto_connect_params(struct sid_link_auto_connect_params params)
{
  cli_arg_sid_link_auto_connect_params = params;
  queue_event(g_event_queue, EVENT_TYPE_SET_AUTO_CONNECT_PARAMS);

  SL_SID_LOG_APP_INFO("set auto connect params event");
}

/*******************************************************************************
 * Trigger get auto connect parameters
 * @returns None
 ******************************************************************************/
void sl_app_trigger_get_auto_connect_params(void)
{
  queue_event(g_event_queue, EVENT_TYPE_GET_AUTO_CONNECT_PARAMS);

  SL_SID_LOG_APP_INFO("get auto connect params event");
}

/*******************************************************************************
 * Set - dev profile id
 * @param[in] app_context
 * @param[in] id
 * @returns None
 ******************************************************************************/
void set_sidewalk_dev_prof_id(app_context_t *app_context, uint8_t id)
{
  struct sid_device_profile dev_cfg = { 0 };

  if (!app_context) {
    return;
  }

  sid_error_t ret = sid_option(app_context->sidewalk_handle,
                               SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg,
                               sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk device profile get failed, error: %d", ret);
  } else {
    dev_cfg.unicast_params.device_profile_id = id;
    ret = sid_option(app_context->sidewalk_handle,
                     SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
                     &dev_cfg,
                     sizeof(dev_cfg));
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("sidewalk device profile set failed, error: %d", ret);
    } else {
      SL_SID_LOG_APP_INFO("sidewalk device profile set");
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
  struct sid_device_profile dev_cfg = { 0 };

  if (!app_context) {
    return;
  }

  sid_error_t ret = sid_option(app_context->sidewalk_handle,
                               SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg,
                               sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk device profile get failed, error: %d", ret);
  } else {
    SL_SID_LOG_APP_INFO("sidewalk device profile RX window count: %d",
                        dev_cfg.unicast_params.rx_window_count);
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
  struct sid_device_profile dev_cfg = { 0 };

  if (!app_context) {
    return;
  }

  sid_error_t ret = sid_option(app_context->sidewalk_handle,
                               SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg,
                               sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk device profile get failed, error: %d", ret);
  } else {
    dev_cfg.unicast_params.rx_window_count = rx_win_cnt;
    ret = sid_option(app_context->sidewalk_handle,
                     SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
                     &dev_cfg,
                     sizeof(dev_cfg));
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("sidewalk device profile set failed, error: %d", ret);
    } else {
      SL_SID_LOG_APP_INFO("sidewalk device profile RX window count set");
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
  struct sid_device_profile dev_cfg = { 0 };

  if (!app_context) {
    return;
  }

  sid_error_t ret = sid_option(app_context->sidewalk_handle,
                               SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg,
                               sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk device profile get failed, error: %d", ret);
  } else {
    // As both members of the union has same type (uint16_t) then it's not important which member is read.
    SL_SID_LOG_APP_INFO("sidewalk device profile RX window separation interval in ms: %d",
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
  struct sid_device_profile dev_cfg = { 0 };

  if (!app_context) {
    return;
  }

  sid_error_t ret = sid_option(app_context->sidewalk_handle,
                               SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg,
                               sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk device profile get failed, error: %d", ret);
  } else {
    // As both members of the union has same type (uint16_t) then it's not important which member is read.
    dev_cfg.unicast_params.unicast_window_interval.sync_rx_interval_ms = rx_interv_ms;
    ret = sid_option(app_context->sidewalk_handle,
                     SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
                     &dev_cfg,
                     sizeof(dev_cfg));
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("sidewalk device profile set failed, error: %d", ret);
    } else {
      SL_SID_LOG_APP_INFO("sidewalk device profile RX window separation interval in ms set");
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
  struct sid_device_profile dev_cfg = { 0 };

  if (!app_context) {
    return;
  }

  sid_error_t ret = sid_option(app_context->sidewalk_handle,
                               SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg,
                               sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk device profile get failed, error: %d", ret);
  } else {
    SL_SID_LOG_APP_INFO("sidewalk device profile wakeup type: %d",
                        dev_cfg.unicast_params.wakeup_type);
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
  struct sid_device_profile dev_cfg = { 0 };

  if (!app_context) {
    return;
  }

  sid_error_t ret = sid_option(app_context->sidewalk_handle,
                               SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg,
                               sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk device profile get failed, error: %d", ret);
  } else {
    dev_cfg.unicast_params.wakeup_type = wakeup_type;
    ret = sid_option(app_context->sidewalk_handle,
                     SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
                     &dev_cfg,
                     sizeof(dev_cfg));
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("sidewalk device profile set failed, error: %d", ret);
    } else {
      SL_SID_LOG_APP_INFO("sidewalk device profile wakeup type set");
    }
  }
}

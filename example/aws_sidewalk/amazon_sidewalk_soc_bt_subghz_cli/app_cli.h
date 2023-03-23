/***************************************************************************//**
 * @file
 * @brief app_cli.h
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
#ifndef APP_CLI_H
#define APP_CLI_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdint.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "sl_cli_types.h"
#include "app_init.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

extern char *argument_value_str;
extern char *argument_value_str_2;

extern uint32_t cli_arg_uint32_t;
extern int16_t cli_arg_int16_t;
extern uint16_t cli_arg_uint16_t;
extern uint8_t cli_arg_uint8_t;
extern char cli_arg_str[64];
extern char cli_arg_str_2[64];

extern QueueHandle_t g_cli_event_queue;

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/*******************************************************************************
 * CLI - send
 *
 * @param[in] arguments CLI arguments
 * @returns None
 ******************************************************************************/
void cli_sid_send(sl_cli_command_arg_t *arguments);

/*******************************************************************************
 * CLI - reset
 *
 * @param[in] arguments CLI arguments
 * @returns None
 ******************************************************************************/
void cli_sid_reset(sl_cli_command_arg_t *arguments);

/*******************************************************************************
 * CLI - init
 *
 * @param[in] arguments CLI arguments
 * @returns None
 ******************************************************************************/
void cli_sid_init(sl_cli_command_arg_t *arguments);

/*******************************************************************************
 * CLI - start
 *
 * @param[in] arguments CLI arguments
 * @returns None
 ******************************************************************************/
void cli_sid_start(sl_cli_command_arg_t *arguments);

/*******************************************************************************
 * CLI - stop
 *
 * @param[in] arguments CLI arguments
 * @returns None
 ******************************************************************************/
void cli_sid_stop(sl_cli_command_arg_t *arguments);

/*******************************************************************************
 * CLI - deinit
 *
 * @param[in] arguments CLI arguments
 * @returns None
 ******************************************************************************/
void cli_sid_deinit(sl_cli_command_arg_t *arguments);

/*******************************************************************************
 * CLI - ble connect
 *
 * @param[in] arguments CLI arguments
 * @returns None
 ******************************************************************************/
void cli_sid_ble_connect(sl_cli_command_arg_t *arguments);

/*******************************************************************************
 * Function to get sidewalk time
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
void get_sidewalk_time(app_context_t *app_context);

/*******************************************************************************
 * Function to get sidewalk status
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
void get_sidewalk_status(app_context_t *app_context);

/*******************************************************************************
 * Function to get sidewalk mtu
 *
 * @param[in] app_context The context which is applicable for the current application
 * @param[in] sid_link_type Link type
 * @returns None
 ******************************************************************************/
void get_sidewalk_mtu(app_context_t *app_context, enum sid_link_type);

/*******************************************************************************
 * Function to get sidewalk CSS dev profile id
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
void get_sidewalk_css_dev_prof_id(app_context_t *app_context);

/*******************************************************************************
 * Function to get sidewalk FSK dev profile id
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
void get_sidewalk_fsk_dev_prof_id(app_context_t *app_context);

/*******************************************************************************
 * Function to set sidewalk CSS dev profile id
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
void set_sidewalk_css_dev_prof_id(app_context_t *app_context);

/*******************************************************************************
 * Function to set sidewalk FSK dev profile id
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
void set_sidewalk_fsk_dev_prof_id(app_context_t *app_context);

/*******************************************************************************
 * Function to trigger sid send
 *
 * @param[in] message_type_str
 * @param[in] message_str
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_send(char *message_type_str, char *message_str);

/*******************************************************************************
 * Function to trigger sid reset
 *
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_reset(void);

/*******************************************************************************
 * Function to trigger sid init
 *
 * @param[in] link_str
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_init(char *link_str);

/*******************************************************************************
 * Function to trigger sid start
 *
 * @param[in] link_str
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_start(char *link_str);

/*******************************************************************************
 * Function to trigger sid stop
 *
 * @param[in] link_str
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_stop(char *link_str);

/*******************************************************************************
 * Function to trigger sid deinit
 *
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_deinit(void);

/*******************************************************************************
 * Function to trigger sid get CSS dev profile id
 *
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_get_css_dev_prof_id(void);

/*******************************************************************************
 * Function to trigger sid set CSS dev profile id
 *
 * @param[in] value
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_css_dev_prof_id(char *value);

/*******************************************************************************
 * Function to trigger sid get FSK dev prof id
 *
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_get_fsk_dev_prof_id(void);

/*******************************************************************************
 * Function to trigger sid set FSK dev profile id
 *
 * @param[in] value
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_fsk_dev_prof_id(char *value);

/*******************************************************************************
 * Function to trigger sid set dev profile id
 *
 * @param[in] profile_id
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_dev_prof_id(uint8_t profile_id);

/*******************************************************************************
 * Function to trigger sid get dev profile rx window count
 *
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_get_dev_prof_rx_win_cnt(void);

/*******************************************************************************
 * Function to trigger sid set dev profile rx window count
 *
 * @param[in] rx_win_cnt
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_dev_prof_rx_win_cnt(int16_t rx_win_cnt);

/*******************************************************************************
 * Function to trigger sid get dev profile rx interval in ms
 *
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_get_dev_prof_rx_interv_ms(void);

/*******************************************************************************
 * Function to trigger sid set dev profile rx interval in ms
 *
 * @param[in] rx_interv_ms
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_dev_prof_rx_interv_ms(uint16_t rx_interv_ms);

/*******************************************************************************
 * Function to trigger sid get dev profile wakeup type
 *
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_get_dev_prof_wakeup_type(void);

/*******************************************************************************
 * Function to trigger sid set dev profile wakeup type
 *
 * @param[in] wakeup_type
 * @returns None
 ******************************************************************************/
void sl_app_trigger_sid_set_dev_prof_wakeup_type(uint8_t wakeup_type);

/*******************************************************************************
 * Function to trigger BLE connection request
 *
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_ble_connection_request(void);

/*******************************************************************************
 * Application function to update counter and send
 *
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_send_counter_update(void);

/*******************************************************************************
 * Application function to trigger Factory reset
 *
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_factory_reset(void);

/*******************************************************************************
 * Application function to send
 *
 * @param[in] len
 * @returns None
 ******************************************************************************/
void sl_app_trigger_send(uint32_t len);

/*******************************************************************************
 * Application function to trigger get time
 *
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_get_time(void);

/*******************************************************************************
 * Application function to trigger get sidewalk status
 *
 * @param[in] void
 * @returns None
 ******************************************************************************/
void sl_app_trigger_get_status(void);

/*******************************************************************************
 * Application function to trigger get MTU
 *
 * @param[in] link_type
 * @returns None
 ******************************************************************************/
void sl_app_trigger_get_mtu(enum sid_link_type link_type);

/*******************************************************************************
 * Function to set sidewalk dev prof id
 *
 * @param[in] app_context The context which is applicable for the current application
 * @param[in] id Profile ID
 * @returns None
 ******************************************************************************/
void set_sidewalk_dev_prof_id(app_context_t *app_context, uint8_t id);

/*******************************************************************************
 * Function to get dev prof rx win cnt
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
void get_sidewalk_dev_prof_rx_win_cnt(app_context_t *app_context);

/*******************************************************************************
 * Function to set dev prof rx win cnt
 *
 * @param[in] app_context The context which is applicable for the current application
 * @param[in] rx_win_cnt Rx window counter
 * @returns None
 ******************************************************************************/
void set_sidewalk_dev_prof_rx_win_cnt(app_context_t *app_context, uint16_t rx_win_cnt);

/*******************************************************************************
 * Function to get dev prof rx interv in ms
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
void get_sidewalk_dev_prof_rx_interv_ms(app_context_t *app_context);

/*******************************************************************************
 * Function to set dev prof rx interv in ms
 *
 * @param[in] app_context The context which is applicable for the current application
 * @param[in] rx_interv_ms Rx interval in ms
 * @returns None
 ******************************************************************************/
void set_sidewalk_dev_prof_rx_interv_ms(app_context_t *app_context, uint16_t rx_interv_ms);

/*******************************************************************************
 * Function to get prof wakeup type
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
void get_sidewalk_dev_prof_wakeup_type(app_context_t *app_context);

/*******************************************************************************
 * Function to set prof wakeup type
 *
 * @param[in] app_context The context which is applicable for the current application
 * @param[in] wakeup_type Wakeup type -> 0: No wakeup - 1: TX only - 2: RX only - 3: Both
 * @returns None
 ******************************************************************************/
void set_sidewalk_dev_prof_wakeup_type(app_context_t *app_context, uint8_t wakeup_type);

#ifdef __cplusplus
}
#endif

#endif // APP_CLI_H

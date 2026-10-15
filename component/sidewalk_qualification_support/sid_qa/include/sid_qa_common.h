/*
 * Copyright 2023-2025 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_QA_COMMON_H
#define SID_QA_COMMON_H

#include <sid_asd_cli.h>
#include <sid_api.h>
#include <sid_event_queue_ifc.h>

#include <sid_qa.h>
#include <sid_pal_radio_ifc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_QUEUE_LEN 10
#define CLI_MAX_DATA_LEN 256
#define CLI_MAX_HEX_STR_LEN (CLI_MAX_DATA_LEN * 2)
#define CLI_MAX_SEND_PARAMS 20

#define CLI_CMD_OPT_LINK_BLE 1
#define CLI_CMD_OPT_LINK_FSK 2
#define CLI_CMD_OPT_LINK_LORA 3
#define CLI_CMD_OPT_LINK_BLE_LORA 4
#define CLI_CMD_OPT_LINK_BLE_FSK 5
#define CLI_CMD_OPT_LINK_FSK_LORA 6
#define CLI_CMD_OPT_LINK_BLE_FSK_LORA 7
#define CLI_CMD_OPT_LINK_ANY 8

#define CLI_CMD_OPT_GPS_TIME 0

#define CLI_CMD_OPT_CON_REQ_CLEAR 0
#define CLI_CMD_OPT_CON_REQ_SET 1

#define CLI_CMD_OPT_MSG_TYPE_GET 0
#define CLI_CMD_OPT_MSG_TYPE_SET 1
#define CLI_CMD_OPT_MSG_TYPE_NTF 2
#define CLI_CMD_OPT_MSG_TYPE_RSP 3

#define CLI_BUF_SIZE 250
#define CLI_BYTES_PER_LINE 16

#define CMD_PRINT_RESULT(res) cmd_print_result(res, "\0")

#define CMD_PRINT_RESULT_PARAMS(res, _ft, ...) cmd_print_result(res, _ft, ##__VA_ARGS__)

#define MEAS_TIME_DEFAULT 60
#define MEAS_START_DELAY_DEFAULT 10
#define MEAS_UL_TIME_DEFAULT 1
#define MEAS_PARAM_VALUE_MAX 255
#define MEAS_TIME_MAX_S 4000

enum event_type { EVENT_TYPE_SIDEWALK, EVENT_MEAS_UL_SEND, EVENT_MEAS_STOP, EVENT_UP_TEST, EVENT_MEAS_SET_CONN };

enum app_state {
    STATE_INIT,
    STATE_SIDEWALK_READY,
    STATE_SIDEWALK_SECURE_CHANNEL_READY,
    STATE_SIDEWALK_NOT_READY,
};

struct app_context {
    struct sid_event_queue event_queue;
    struct sid_event_queue event_queue_full_process;   // TODO: Remove and integrate both
    struct sid_handle **sidewalk_handle;
    enum app_state state;
    uint8_t counter;
    struct sid_event_callbacks *extra_callbacks;
};

struct cli_config {
    struct sid_config *sid_cfg;
    struct app_context *app_cxt;
    uint32_t send_link_type;
    uint32_t rsp_msg_id;
    reboot_func_t reboot_cmd;
    set_sub_ghz_cfg_t set_sub_ghz_cfg;
    struct sid_device_info *device_info_cfg;
};

enum pwr_meas_type {
    PWR_MEAS_TYPE_FSK_LORA,
    PWR_MEAS_TYPE_BLE,
    PWR_MEAS_TYPE_FSK_LORA_SET_CONN_ONLY,
    PWR_MEAS_TYPE_MAX,
};

struct pwr_meas_config {
    bool active;
    enum pwr_meas_type mode;
    bool ble_only_flag;
    uint32_t ul_pkt_cnt;
    uint32_t msg_sent_cnt;
    size_t ul_pkt_len;
    sid_pal_timer_t timer_ul;     // uplink generation timer
    sid_pal_timer_t timer_meas;   // measurment mode timer
    sid_pal_timer_t timer_set_conn;
    struct sid_qa_pwr_meas_if *platform_if;
    sid_pal_radio_rx_packet_t rx_packet;
};

struct uplink_msg_info {
    uint16_t msg_id;
    uint8_t link_type_sent;
    uint8_t link_mode_sent;
    uint16_t time_delta_from_first_pkt_ms;
    uint16_t time_delta_on_msg_sent_from_first_pkt_ms;
    uint16_t time_delta_on_transport_ack_from_first_pkt_ms;
    sid_error_t sent_status;
    bool ack_received;
};

struct uplink_test {
    bool start;
    struct sid_timespec first_pkt_time;
    uint8_t ack_req;
    uint8_t retries;
    uint8_t msg_type;
    uint8_t total_iterations;
    uint8_t iteration;
    uint16_t msg_size;
    uint16_t ttl_secs;
    uint32_t link_mask;
    uint8_t link_mode;
    uint16_t link_1_connected_in_ms;
    uint16_t link_2_connected_in_ms;
    uint16_t link_3_connected_in_ms;
    uint8_t test_mode;
    uint16_t sent_success_link_1;
    uint16_t sent_success_link_2;
    uint16_t sent_success_link_3;
    uint16_t acks_received_link_1;
    uint16_t acks_received_link_2;
    uint16_t acks_received_link_3;
    uint16_t sent_failed_with_timeout;
    uint16_t sent_failed_with_other_errors;
    uint16_t sent_with_mismatch_in_link_types;
    uint16_t successfully_addedd_to_queue;
    uint16_t failed_to_add_to_queue;
    uint16_t acks_requested;
    uint16_t num_downlinks_received_link_1;
    uint16_t num_downlinks_received_link_2;
    uint16_t num_downlinks_received_link_3;
    uint16_t num_duplicates_link_1;
    uint16_t num_duplicates_link_2;
    uint16_t num_duplicates_link_3;
    uint16_t test_periodicity_secs;
    struct uplink_msg_info msg_info[50];
    struct sid_deferred_event uplink_test_event;
};

struct flood_helper_args {
    struct sid_handle *handle;
    struct sid_msg *msg;
    struct sid_msg_desc *desc;
    uint32_t flood_period;
    uint32_t delay_secs;
};

struct flood_test {
    uint8_t flood_msg;
    uint32_t flood_period;
    uint32_t delay_secs;
    uint32_t counter;
    uint32_t msg_idx;
    struct sid_msg msg_helper;
    struct sid_msg_desc desc_helper;
    struct sid_deferred_event flood_test_event;
};

enum data_type {
    DATA_TYPE_NORMAL = 1,
    DATA_TYPE_HEX_STRING = 2,
    DATA_TYPE_NORMAL_WITH_MSG_TYPE = 3,
    DATA_TYPE_HEX_STRING_WITH_MSG_TYPE = 4,
    DATA_TYPE_HEX_STRING_WITH_MSG_TYPE_WITH_DEST = 6
};

void cmd_print_result(sid_error_t res, char const *p_fmt, ...);

void sid_sbdt_register_cli_commands(struct app_context *context);
void sid_dult_register_cli_commands(struct app_context *context);
#if defined(SID_SDK_CONFIG_ENABLE_LOCATION) && SID_SDK_CONFIG_ENABLE_LOCATION
void sid_location_register_cli_commands(struct app_context *context);
#endif
void sid_gwscan_register_cli_commands(struct app_context *context);
void sid_sbdt_on_msg_recieved(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context);
void sid_sbdt_on_msg_sent(const struct sid_msg_desc *msg_desc, void *context);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_QA_COMMON_H */

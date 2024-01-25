/***************************************************************************//**
 * @file sl_sidewalk_app_msg_dev_mgmt.h
 * @brief sidewalk application message component - device management
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

#ifndef SL_SID_APP_MSG_DEV_MGMT_H
#define SL_SID_APP_MSG_DEV_MGMT_H

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/

#include "sl_sidewalk_app_msg_core.h"

/*******************************************************************************
 *** DEFINES
 ******************************************************************************/

typedef enum {
  SL_SID_APP_MSG_DEV_MGMT_VAL_RST_SOFT = 1,   // 0b01 (software reset)
  SL_SID_APP_MSG_DEV_MGMT_VAL_RST_HARD        // 0b10 (factory reset)
} sl_sid_app_msg_dev_mgmt_rst_type_t;

/*******************************************************************************
 *** MACROS AND TYPEDEFS
 ******************************************************************************/

// Device management command IDs (encoded in 7 bits)
typedef enum {
  // reset device
  //    1. cloud -> dev (set) --- dev -> cloud (ack)
  //       set_t:  sli_sid_app_msg_dev_mgmt_rst_dev_set_t
  //       ack_t:  sl_sid_app_msg_ack_msg_t
  //    2. dev -> cloud (ntfy)
  //       ntfy_t: sli_sid_app_msg_dev_mgmt_rst_dev_ntfy_t
  SLI_SID_APP_MSG_CMD_ID_DEV_MGMT_RST_DEV = 0,
  // device capability
  //    1. cloud -> dev (get) --- dev -> cloud (resp)
  //       get_t:  NA
  //       resp_t: sli_sid_app_msg_dev_mgmt_dev_cap_resp_t
  //    2. dev -> cloud (set) --- cloud -> dev (ack)
  //       set_t:  sli_sid_app_msg_dev_mgmt_dev_cap_set_t
  //       ack_t:  sl_sid_app_msg_ack_msg_t
  SLI_SID_APP_MSG_CMD_ID_DEV_MGMT_DEV_CAPABILITY,
  // battery status
  //    1. cloud -> dev (get) --- dev -> cloud (resp)
  //       get_t:  NA
  //       resp_t: sli_sid_app_msg_dev_mgmt_bat_st_resp_t
  //    2. dev -> cloud (ntfy)
  //       ntfy_t: sli_sid_app_msg_dev_mgmt_bat_st_ntfy_t
  SLI_SID_APP_MSG_CMD_ID_DEV_MGMT_BATTERY_STATUS,
  // temperature sensor status
  //    1. cloud -> dev (get) --- dev -> cloud (resp)
  //       get_t:  NA
  //       resp_t: sli_sid_app_msg_dev_mgmt_temp_sensor_st_resp_t
  //    2. dev -> cloud (ntfy)
  //       ntfy_t: sli_sid_app_msg_dev_mgmt_temp_sensor_st_ntfy_t
  SLI_SID_APP_MSG_CMD_ID_DEV_MGMT_TEMP_SENSOR_STATUS,
  // button press
  //    1. cloud -> dev (set) --- dev -> cloud (ack)
  //       set_t:  sli_sid_app_msg_dev_mgmt_button_press_set_t
  //       ack_t:  sl_sid_app_msg_ack_msg_t
  //    2. dev -> cloud (ntfy)
  //       ntfy_t: sli_sid_app_msg_dev_mgmt_button_press_ntfy_t
  SLI_SID_APP_MSG_CMD_ID_DEV_MGMT_BUTTON_PRESS,
  // toggle led
  //    1. cloud -> dev (set) --- dev -> cloud (ack)
  //       set_t:  sli_sid_app_msg_dev_mgmt_toggle_led_set_t
  //       ack_t:  sl_sid_app_msg_ack_msg_t
  //    2. dev -> cloud (ntfy)
  //       ntfy_t: sli_sid_app_msg_dev_mgmt_toggle_led_ntfy_t
  SLI_SID_APP_MSG_CMD_ID_DEV_MGMT_TOGGLE_LED
} sli_sid_app_msg_cmd_id_dev_mgmt_t;

// Protocol level structures
// reset device (set)
typedef struct {
  uint32_t in_millisecs;
  uint8_t reset_type: 2;  // sl_sid_app_msg_dev_mgmt_rst_type_t
  uint8_t rfu: 6;
} SL_ATTRIBUTE_PACKED sli_sid_app_msg_dev_mgmt_rst_dev_set_t;

// reset device (ntfy)
typedef sli_sid_app_msg_dev_mgmt_rst_dev_set_t sli_sid_app_msg_dev_mgmt_rst_dev_ntfy_t;

// reset device send parameters for application access
typedef sli_sid_app_msg_dev_mgmt_rst_dev_set_t sl_sid_app_msg_dev_mgmt_rst_dev_param_send_t;

// device capability (resp)
typedef struct {
  uint32_t led0:1;
  uint32_t led1:1;
  uint32_t btn0:1;
  uint32_t btn1:1;
  uint32_t glcd:1;
  uint32_t sensor_humidity:1;
  uint32_t sensor_temp:1;
  uint32_t rfu:25;
} SL_ATTRIBUTE_PACKED sli_sid_app_msg_dev_mgmt_dev_cap_resp_t;

// device capability (set)
typedef sli_sid_app_msg_dev_mgmt_dev_cap_resp_t sli_sid_app_msg_dev_mgmt_dev_cap_set_t;

// battery status (resp)
typedef struct {
  uint8_t percent;
} SL_ATTRIBUTE_PACKED sli_sid_app_msg_dev_mgmt_bat_st_resp_t;

// battery status (ntfy)
typedef sli_sid_app_msg_dev_mgmt_bat_st_resp_t sli_sid_app_msg_dev_mgmt_bat_st_ntfy_t;

// temperature sensor status (resp)
typedef struct {
  uint8_t degrees_fractional;
  int8_t degrees_decimal;
} SL_ATTRIBUTE_PACKED sli_sid_app_msg_dev_mgmt_temp_sensor_st_resp_t;

// temperature sensor status (ntfy)
typedef sli_sid_app_msg_dev_mgmt_temp_sensor_st_resp_t sli_sid_app_msg_dev_mgmt_temp_sensor_st_ntfy_t;

// button press (set)
typedef struct {
  uint8_t button: 3;
  uint8_t duration: 2;  // APP_BUTTON_PRESS_DURATION_* defines in app_button_press.h
  uint8_t rfu: 3;
} SL_ATTRIBUTE_PACKED sli_sid_app_msg_dev_mgmt_button_press_set_t;

// button press (ntfy)
typedef sli_sid_app_msg_dev_mgmt_button_press_set_t sli_sid_app_msg_dev_mgmt_button_press_ntfy_t;

// button press send parameters for application access
typedef sli_sid_app_msg_dev_mgmt_button_press_set_t sl_sid_app_msg_dev_mgmt_button_press_param_send_t;

// toggle led (set)
typedef struct {
  uint8_t led;
} SL_ATTRIBUTE_PACKED sli_sid_app_msg_dev_mgmt_toggle_led_set_t;

// toggle led (ntfy)
typedef struct {
  uint8_t led;
  uint8_t state;  // sl_led_state_t
} SL_ATTRIBUTE_PACKED sli_sid_app_msg_dev_mgmt_toggle_led_ntfy_t;

// toggle led send parameters for application access
typedef sli_sid_app_msg_dev_mgmt_toggle_led_ntfy_t sl_sid_app_msg_dev_mgmt_toggle_led_param_send_t;

// Application level structures
typedef struct {
  sl_sid_app_msg_handler_t hdl;
  sl_sid_app_msg_ack_msg_t param_ack;
  sl_sid_app_msg_dev_mgmt_rst_dev_param_send_t param_send;
} sl_sid_app_msg_dev_mgmt_rst_dev_ctx_t;

typedef struct {
  sl_sid_app_msg_handler_t hdl;
  sl_sid_app_msg_ack_msg_t param_ack;
  sl_sid_app_msg_dev_mgmt_button_press_param_send_t param_send;
  bool is_emulation;
} sl_sid_app_msg_dev_mgmt_button_press_ctx_t;

typedef struct {
  sl_sid_app_msg_handler_t hdl;
  sl_sid_app_msg_ack_msg_t param_ack;
  sl_sid_app_msg_dev_mgmt_toggle_led_param_send_t param_send;
} sl_sid_app_msg_dev_mgmt_toggle_led_ctx_t;

/*******************************************************************************
 *** PUBLIC FUNCTIONS
 ******************************************************************************/

sl_sid_app_msg_st_t sli_sid_app_msg_dev_mgmt_cmd_handler(sl_sid_app_msg_t *msg);

/***************************************************************************//**
 * @brief
 *   Prepares corresponding application message to be sent.
 * 
 * @note
 *   Application has to declare the application message and pass its reference
 *   to the function. Allocated memory of the declared message will be used for
 *   serialization.
 *
 * @param[in] ctx Related application message context
 * @param[out] send_app_msg Application message to be sent
 * 
 * @return
 *   Status code
 ******************************************************************************/
sl_sid_app_msg_st_t sl_sid_app_msg_dev_mgmt_rst_dev_prepare_send(
  sl_sid_app_msg_dev_mgmt_rst_dev_ctx_t *ctx, sl_sid_app_msg_t *send_app_msg);

/***************************************************************************//**
 * @brief
 *   Prepares corresponding application message to be sent.
 * 
 * @note
 *   Application has to declare the application message and pass its reference
 *   to the function. Allocated memory of the declared message will be used for
 *   serialization.
 *
 * @param[in] ctx Related application message context
 * @param[out] send_app_msg Application message to be sent
 * 
 * @return
 *   Status code
 ******************************************************************************/
sl_sid_app_msg_st_t sl_sid_app_msg_dev_mgmt_button_press_prepare_send(
  sl_sid_app_msg_dev_mgmt_button_press_ctx_t *ctx, sl_sid_app_msg_t *send_app_msg);

/***************************************************************************//**
 * @brief
 *   Prepares corresponding application message to be sent.
 * 
 * @note
 *   Application has to declare the application message and pass its reference
 *   to the function. Allocated memory of the declared message will be used for
 *   serialization.
 *
 * @param[in] ctx Related application message context
 * @param[out] send_app_msg Application message to be sent
 * 
 * @return
 *   Status code
 ******************************************************************************/
sl_sid_app_msg_st_t sl_sid_app_msg_dev_mgmt_toggle_led_prepare_send(
  sl_sid_app_msg_dev_mgmt_toggle_led_ctx_t *ctx, sl_sid_app_msg_t *send_app_msg);

/***************************************************************************//**
 * @brief
 *   Callback function that is called when the corresponding application message
 *   is received. It has to be implemented by the application.
 * 
 * @note
 *   Context has to be copied for further operation(s) as it is no longer valid
 *   after the execution of the callback.
 *
 * @param[in] ctx Corresponding application message context
 ******************************************************************************/
void sl_sid_app_msg_dev_mgmt_rst_dev_cb(sl_sid_app_msg_dev_mgmt_rst_dev_ctx_t *ctx);

/***************************************************************************//**
 * @brief
 *   Callback function that is called when the corresponding application message
 *   is received. It has to be implemented by the application.
 * 
 * @note
 *   Context has to be copied for further operation(s) as it is no longer valid
 *   after the execution of the callback.
 *
 * @param[in] ctx Corresponding application message context
 ******************************************************************************/
void sl_sid_app_msg_dev_mgmt_button_press_cb(sl_sid_app_msg_dev_mgmt_button_press_ctx_t *ctx);

/***************************************************************************//**
 * @brief
 *   Callback function that is called when the corresponding application message
 *   is received. It has to be implemented by the application.
 * 
 * @note
 *   Context has to be copied for further operation(s) as it is no longer valid
 *   after the execution of the callback.
 *
 * @param[in] ctx Corresponding application message context
 ******************************************************************************/
void sl_sid_app_msg_dev_mgmt_toggle_led_cb(sl_sid_app_msg_dev_mgmt_toggle_led_ctx_t *ctx);

#endif  // SL_SID_APP_MSG_DEV_MGMT_H

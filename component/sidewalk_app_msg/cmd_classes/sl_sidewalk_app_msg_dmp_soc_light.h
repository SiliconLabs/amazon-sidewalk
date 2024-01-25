/***************************************************************************//**
 * @file sl_sidewalk_app_msg_dmp_soc_light.h
 * @brief sidewalk application message component - DMP SOC Light application
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

#ifndef SL_SID_APP_MSG_DMP_SOC_LIGHT_H
#define SL_SID_APP_MSG_DMP_SOC_LIGHT_H

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/

#include "sl_sidewalk_app_msg_core.h"

/*******************************************************************************
 *** MACROS AND TYPEDEFS
 ******************************************************************************/

// DMP SOC Light command IDs (encoded in 7 bits)
typedef enum {
  // BLE start/stop
  //    1. cloud -> dev (set) --- dev -> cloud (ack)
  //       set_t:  NA
  //       ack_t:  sl_sid_app_msg_ack_msg_t
  //               optional: ble state
  //    2. dev -> cloud (ntfy)
  //       ntfy_t: sli_sid_app_msg_dmp_soc_light_ble_start_stop_ntfy_t
  SLI_SID_APP_MSG_CMD_ID_DMP_SOC_LIGHT_BLE_START_STOP = 0,
  // Update counter
  //    1. cloud -> dev (set) --- dev -> cloud (ack)
  //       set_t:  NA
  //       ack_t:  sl_sid_app_msg_ack_msg_t
  //               optional: current counter value
  //    2. dev -> cloud (ntfy)
  //       ntfy_t: sli_sid_app_msg_dmp_soc_light_update_counter_ntfy_t
  SLI_SID_APP_MSG_CMD_ID_DMP_SOC_LIGHT_UPDATE_COUNTER
} sli_sid_app_msg_cmd_id_dmp_soc_light_t;

// Protocol level structures
// BLE start/stop (ntfy)
typedef struct {
  uint8_t state;
} SL_ATTRIBUTE_PACKED sli_sid_app_msg_dmp_soc_light_ble_start_stop_ntfy_t;

// BLE start/stop send parameters for application access
typedef sli_sid_app_msg_dmp_soc_light_ble_start_stop_ntfy_t sl_sid_app_msg_dmp_soc_light_ble_start_stop_param_send_t;

// update counter (ntfy)
typedef struct {
  uint8_t counter;
} SL_ATTRIBUTE_PACKED sli_sid_app_msg_dmp_soc_light_update_counter_ntfy_t;

// Update counter send parameters for application access
typedef sli_sid_app_msg_dmp_soc_light_update_counter_ntfy_t sl_sid_app_msg_dmp_soc_light_update_counter_param_send_t;

// Application level structures
typedef struct {
  sl_sid_app_msg_handler_t hdl;
  sl_sid_app_msg_ack_msg_t param_ack;
  sl_sid_app_msg_dmp_soc_light_ble_start_stop_param_send_t param_send;
} sl_sid_app_msg_dmp_soc_light_ble_start_stop_ctx_t;

typedef struct {
  sl_sid_app_msg_handler_t hdl;
  sl_sid_app_msg_ack_msg_t param_ack;
  sl_sid_app_msg_dmp_soc_light_update_counter_param_send_t param_send;
} sl_sid_app_msg_dmp_soc_light_update_counter_ctx_t;

/*******************************************************************************
 *** PUBLIC FUNCTIONS
 ******************************************************************************/

sl_sid_app_msg_st_t sli_sid_app_msg_dmp_soc_light_cmd_handler(sl_sid_app_msg_t *msg);

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
sl_sid_app_msg_st_t sl_sid_app_msg_dmp_soc_light_ble_start_stop_prepare_send(
  sl_sid_app_msg_dmp_soc_light_ble_start_stop_ctx_t *ctx, sl_sid_app_msg_t *send_app_msg);

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
sl_sid_app_msg_st_t sl_sid_app_msg_dmp_soc_light_update_counter_prepare_send(
  sl_sid_app_msg_dmp_soc_light_update_counter_ctx_t *ctx, sl_sid_app_msg_t *send_app_msg);

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
void sl_sid_app_msg_dmp_soc_light_ble_start_stop_cb(sl_sid_app_msg_dmp_soc_light_ble_start_stop_ctx_t *ctx);

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
void sl_sid_app_msg_dmp_soc_light_update_counter_cb(sl_sid_app_msg_dmp_soc_light_update_counter_ctx_t *ctx);

#endif  // SL_SID_APP_MSG_DMP_SOC_LIGHT_H

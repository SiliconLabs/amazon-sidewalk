/***************************************************************************//**
 * @file sl_sidewalk_app_msg_sid.h
 * @brief sidewalk application message component - Sidewalk specific
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

#ifndef SL_SID_APP_MSG_SID_H
#define SL_SID_APP_MSG_SID_H

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/

#include "sl_sidewalk_app_msg_core.h"

/*******************************************************************************
 *** MACROS AND TYPEDEFS
 ******************************************************************************/

// Sidewalk specific command IDs (encoded in 7 bits)
typedef enum {
  // MTU
  //    1. cloud -> dev (get) --- dev -> cloud (resp)
  //       get_t:  sli_sid_app_msg_sid_mtu_get_t
  //       resp_t: sli_sid_app_msg_sid_mtu_resp_t
  //    2. dev -> cloud (ntfy)
  //       ntfy_t: sli_sid_app_msg_sid_mtu_ntfy_t
  SLI_SID_APP_MSG_CMD_ID_SID_MTU = 0,
  // time
  //    1. cloud -> dev (get) --- dev -> cloud (resp)
  //       get_t:  NA
  //       resp_t: sli_sid_app_msg_sid_time_resp_t
  //    2. dev -> cloud (ntfy)
  //       ntfy_t: sli_sid_app_msg_sid_time_ntfy_t
  SLI_SID_APP_MSG_CMD_ID_SID_TIME
} sli_sid_app_msg_cmd_id_sid_t;

// Protocol level structures
// MTU (get)
typedef struct {
  uint8_t link_type;
} SL_ATTRIBUTE_PACKED sli_sid_app_msg_sid_mtu_get_t;

// MTU (resp)
typedef struct {
  uint16_t mtu;
} SL_ATTRIBUTE_PACKED sli_sid_app_msg_sid_mtu_resp_t;

// MTU (ntfy)
typedef sli_sid_app_msg_sid_mtu_resp_t sli_sid_app_msg_sid_mtu_ntfy_t;

// MTU receive parameters for application access
typedef sli_sid_app_msg_sid_mtu_get_t sl_sid_app_msg_sid_mtu_param_rcv_t;

// MTU send parameters for application access
typedef sli_sid_app_msg_sid_mtu_ntfy_t sl_sid_app_msg_sid_mtu_param_send_t;

// time (resp)
typedef struct {
  uint32_t sec;
  uint32_t nsec;
} SL_ATTRIBUTE_PACKED sli_sid_app_msg_sid_time_resp_t;

// time (ntfy)
typedef sli_sid_app_msg_sid_time_resp_t sli_sid_app_msg_sid_time_ntfy_t;

// Time send parameters for application access
typedef sli_sid_app_msg_sid_time_ntfy_t sl_sid_app_msg_sid_time_param_send_t;

// Application level structures
typedef struct {
  sl_sid_app_msg_handler_t hdl;
  sl_sid_app_msg_ack_msg_t param_ack;
  sl_sid_app_msg_sid_mtu_param_rcv_t param_rcv;
  sl_sid_app_msg_sid_mtu_param_send_t param_send;
} sl_sid_app_msg_sid_mtu_ctx_t;

typedef struct {
  sl_sid_app_msg_handler_t hdl;
  sl_sid_app_msg_ack_msg_t param_ack;
  sl_sid_app_msg_sid_time_param_send_t param_send;
} sl_sid_app_msg_sid_time_ctx_t;

/*******************************************************************************
 *** PUBLIC FUNCTIONS
 ******************************************************************************/

sl_sid_app_msg_st_t sli_sid_app_msg_sid_cmd_handler(sl_sid_app_msg_t *msg);

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
sl_sid_app_msg_st_t sl_sid_app_msg_sid_mtu_prepare_send(
  sl_sid_app_msg_sid_mtu_ctx_t *ctx, sl_sid_app_msg_t *send_app_msg);

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
sl_sid_app_msg_st_t sl_sid_app_msg_sid_time_prepare_send(
  sl_sid_app_msg_sid_time_ctx_t *ctx, sl_sid_app_msg_t *send_app_msg);

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
void sl_sid_app_msg_sid_mtu_cb(sl_sid_app_msg_sid_mtu_ctx_t *ctx);

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
void sl_sid_app_msg_sid_time_cb(sl_sid_app_msg_sid_time_ctx_t *ctx);

#endif  // SL_SID_APP_MSG_SID_H

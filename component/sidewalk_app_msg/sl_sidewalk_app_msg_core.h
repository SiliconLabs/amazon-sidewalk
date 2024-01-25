/***************************************************************************//**
 * @file sl_sidewalk_app_msg_core.h
 * @brief sidewalk application message component
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

#ifndef SL_SID_APP_MSG_CORE_H
#define SL_SID_APP_MSG_CORE_H

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h> // for `CHAR_BIT`
#include "sid_api.h"
#include "sl_common.h"
#include "sl_sidewalk_app_msg_cmd_cls.h"
#include "sid_pal_crypto_ifc.h"

/*******************************************************************************
 *** DEFINES
 ******************************************************************************/

#define SLI_SID_APP_MSG_PROTO_VER               (1)     // increment every msg structure change

#define SL_SID_APP_MSG_APP_ACK_VAL              (0xAA)  // global ACK value
#define SL_SID_APP_MSG_APP_NACK_VAL             (0xCC)  // global NACK value

#define SLI_SID_APP_MSG_MAX_MTU_SIZE            (255)   // BLE

#define SLI_SID_APP_MSG_PROTO_VER_BITS          (4)
#define SLI_SID_APP_MSG_CMD_CLS_BITS            (4)
#define SLI_SID_APP_MSG_CMD_ID_BITS             (7)
#define SLI_SID_APP_MSG_OP_BITS                 (3)
#define SLI_SID_APP_MSG_SEQ_BITS                (6)
#define SLI_SID_APP_MSG_LEN_BITS                (8)
#define SLI_SID_APP_MSG_HEADER_LEN_BYTES        ((\
          SLI_SID_APP_MSG_PROTO_VER_BITS +      \
          SLI_SID_APP_MSG_CMD_CLS_BITS +        \
          SLI_SID_APP_MSG_CMD_ID_BITS +         \
          SLI_SID_APP_MSG_OP_BITS +             \
          SLI_SID_APP_MSG_SEQ_BITS +            \
          SLI_SID_APP_MSG_LEN_BITS) / CHAR_BIT)
#define SLI_SID_APP_MSG_MSG_LEN                 (SLI_SID_APP_MSG_MAX_MTU_SIZE - SLI_SID_APP_MSG_HEADER_LEN_BYTES)

/*******************************************************************************
 *** MACROS AND TYPEDEFS
 ******************************************************************************/

#define SLI_SID_APP_MSG_GENERATE_SEQ_NO(seq, seq_size)          \
    if (sid_pal_crypto_rand(seq, seq_size) != SID_ERROR_NONE) { \
      *seq = 0;                                                 \
    }
#define SLI_SID_APP_MSG_IS_FAILED(ret) ((ret != SL_SID_APP_MSG_ERR_ST_SUCCESS) ? true : false)
#define SLI_SID_APP_MSG_IS_PARAM_INVALID(p) (!p)
#define SLI_SID_APP_MSG_RETURN_ST_IF_FAILED(ret)  \
  if (SLI_SID_APP_MSG_IS_FAILED(ret)) {           \
    return ret;                                   \
  }
#define SLI_SID_APP_MSG_RETURN_ST_IF_PARAM_INVALID_1(p1)  \
  if (SLI_SID_APP_MSG_IS_PARAM_INVALID(p1)) {             \
    return SL_SID_APP_MSG_ERR_ST_APP_INVALID_IN_PARAM;    \
  }
#define SLI_SID_APP_MSG_RETURN_ST_IF_PARAM_INVALID_2(p1, p2)                          \
  if (SLI_SID_APP_MSG_IS_PARAM_INVALID(p1) || SLI_SID_APP_MSG_IS_PARAM_INVALID(p2)) { \
    return SL_SID_APP_MSG_ERR_ST_APP_INVALID_IN_PARAM;                                \
  }
#define SLI_SID_APP_MSG_RETURN_ST_IF_PARAM_INVALID_3(p1, p2, p3)                                                              \
  if (SLI_SID_APP_MSG_IS_PARAM_INVALID(p1) || SLI_SID_APP_MSG_IS_PARAM_INVALID(p2) || SLI_SID_APP_MSG_IS_PARAM_INVALID(p3)) { \
    return SL_SID_APP_MSG_ERR_ST_APP_INVALID_IN_PARAM;                                                                        \
  }

#define SLI_SID_APP_MSG_PREP_SEND_ACK_FUNC(cmd_cls, cmd_id) \
  sli_sid_app_msg_prepare_send(                             \
    send_app_msg,                                           \
    cmd_cls,                                                \
    cmd_id,                                                 \
    ctx->hdl.operation,                                     \
    ctx->hdl.sequence,                                      \
    (void *)&ctx->param_ack,                                \
    sizeof(ctx->param_ack))

#define SLI_SID_APP_MSG_PREP_SEND_PARAM_FUNC(cmd_cls, cmd_id) \
  sli_sid_app_msg_prepare_send(                               \
    send_app_msg,                                             \
    cmd_cls,                                                  \
    cmd_id,                                                   \
    ctx->hdl.operation,                                       \
    ctx->hdl.sequence,                                        \
    (void *)&ctx->param_send,                                 \
    sizeof(ctx->param_send))

#define SLI_SID_APP_MSG_CLR_PROCESSING_FLAG(ctx)  \
  ctx->hdl.processing = false

// Error status
typedef enum {
  SL_SID_APP_MSG_ERR_ST_SUCCESS = 0,
  // packet errors
  SL_SID_APP_MSG_ERR_ST_PKT_WRONG_PROTO_VER,
  SL_SID_APP_MSG_ERR_ST_PKT_WRONG_CMD_CLS,
  SL_SID_APP_MSG_ERR_ST_PKT_WRONG_CMD_ID,
  SL_SID_APP_MSG_ERR_ST_PKT_WRONG_OP,
  SL_SID_APP_MSG_ERR_ST_PKT_WRONG_SEQ,
  SL_SID_APP_MSG_ERR_ST_PKT_WRONG_LEN,
  // application errors
  SL_SID_APP_MSG_ERR_ST_APP_INVALID_IN_PARAM,
  SL_SID_APP_MSG_ERR_ST_APP_WRONG_OP,
  SL_SID_APP_MSG_ERR_ST_APP_CMD_HDL_NOT_IMPL
} sl_sid_app_msg_st_t ;

// Operations (encoded in 2 bits)
typedef enum {
  // set
  //    * requires ack from the receiver with the same sequence no
  //    * sequence no is generated by the sender
  SL_SID_APP_MSG_OP_SET = 0,
  // get
  //    * requires response from the receiver with the same sequence no
  //    * sequence no is generated by the sender
  SL_SID_APP_MSG_OP_GET,
  // notify
  //    * push message that does not requires ack nor response
  //    * sequence no is generated by the sender
  SL_SID_APP_MSG_OP_NTFY,
  // response
  //    * response message to the corresponding get message
  //    * sequence no must be the same as the sender
  SL_SID_APP_MSG_OP_RESP,
  // ack/nack
  //    * response message to the corresponding set message
  //    * sequence no must be the same as the sender
  //    * ack message is sent with the status of the corresponding operation
  //    * nack message is sent with the error status of the corresponding operation
  SL_SID_APP_MSG_OP_ACK
} sl_sid_app_msg_op_t;

// TLV message structure
typedef struct {
  struct {
    // byte 0
    uint8_t proto_ver: SLI_SID_APP_MSG_PROTO_VER_BITS; // LSB  SLI_SID_APP_MSG_PROTO_VER_BITS  bits
    uint8_t cmd_cls: SLI_SID_APP_MSG_CMD_CLS_BITS;     // MSB  SLI_SID_APP_MSG_CMD_CLS_BITS    bits
    // byte 1-2
    uint16_t cmd_id: SLI_SID_APP_MSG_CMD_ID_BITS;      // LSB  SLI_SID_APP_MSG_CMD_ID_BITS     bits
    uint16_t op: SLI_SID_APP_MSG_OP_BITS;              // LSB  SLI_SID_APP_MSG_OP_BITS         bits
    uint16_t seq: SLI_SID_APP_MSG_SEQ_BITS;            // MSB  SLI_SID_APP_MSG_SEQ_BITS        bits
  } SL_ATTRIBUTE_PACKED tag;
  // byte 3
  uint8_t length;
  // byte 4-n
  uint8_t value[SLI_SID_APP_MSG_MSG_LEN];
} SL_ATTRIBUTE_PACKED sl_sid_app_msg_t;

// Message handler
typedef struct {
  sl_sid_app_msg_op_t operation;
  uint8_t sequence;
  bool processing;
} sl_sid_app_msg_handler_t;

// Ack message structure
typedef struct {
  uint8_t ack_nack;   // SL_SID_APP_MSG_APP_ACK_VAL or SL_SID_APP_MSG_APP_NACK_VAL
  uint16_t optional;  // optional field is left to the command implementation in case of utility
} SL_ATTRIBUTE_PACKED sl_sid_app_msg_ack_msg_t;

/*******************************************************************************
 *** PUBLIC FUNCTIONS
 ******************************************************************************/

sl_sid_app_msg_st_t sli_sid_app_msg_prepare_send(
  sl_sid_app_msg_t *app_msg, uint8_t cmd_cls, uint8_t cmd_id, sl_sid_app_msg_op_t op, uint8_t seq, void *val, uint16_t val_len);

/***************************************************************************//**
 * @brief
 *   Converts the application message to be sent over the sidewalk network into
 *   a sidewalk message.
 * 
 *   This is an application level API that must be called prior to each
 *   sidewalk message transmission.
 *
 * @param[in] app_msg Application message
 * @param[out] sid_msg Sidewalk message to be sent
 *
 * @return
 *   Status code
 ******************************************************************************/
sl_sid_app_msg_st_t sl_sid_app_msg_prepare_sid_msg(sl_sid_app_msg_t *app_msg, struct sid_msg *sid_msg);

/***************************************************************************//**
 * @brief
 *   Converts the received sidewalk message into an application message and
 *   calls the appropriate command handler which eventually triggers the
 *   appropriate command callback implemented by the application.
 * 
 *   This is an application level API that must be called following to each
 *   successful sidewalk message reception.
 *
 * @param[in] rcvd_sid_msg Received sidewalk message
 *
 * @return
 *   Status code
 ******************************************************************************/
sl_sid_app_msg_st_t sl_sid_app_msg_handler(const struct sid_msg *rcvd_sid_msg);

#endif  // SL_SID_APP_MSG_CORE_H

/***************************************************************************//**
 * @file sl_sidewalk_app_msg_core.c
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

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/

#include "sl_sidewalk_app_msg_core.h"
#include "sl_sidewalk_app_msg_dev_mgmt.h"
#include "sl_sidewalk_app_msg_dmp_soc_light.h"
#include "sl_sidewalk_app_msg_sid.h"

/*******************************************************************************
 *** STATIC FUNCTION PROTOTYPES
 ******************************************************************************/

static sl_sid_app_msg_st_t serialize(sl_sid_app_msg_t *msg, uint8_t **pkt, uint16_t *pkt_len);
static sl_sid_app_msg_st_t deserialize(sl_sid_app_msg_t **msg, uint8_t *pkt, uint16_t pkt_len);
static sl_sid_app_msg_st_t validate(sl_sid_app_msg_t *msg);
static sl_sid_app_msg_st_t receive(sl_sid_app_msg_t **rcvd_app_msg, const struct sid_msg *rcvd_sid_msg);
static inline sl_sid_app_msg_st_t sli_sid_app_msg_prepare_op_for_send(
  sl_sid_app_msg_op_t op_rcv, sl_sid_app_msg_op_t *op_send);

/*******************************************************************************
 *** GLOBAL FUNCTIONS
 ******************************************************************************/

sl_sid_app_msg_st_t sl_sid_app_msg_handler(const struct sid_msg *rcvd_sid_msg)
{
  sl_sid_app_msg_t *rcvd_app_msg;

  SLI_SID_APP_MSG_RETURN_ST_IF_FAILED(receive(&rcvd_app_msg, rcvd_sid_msg));

  switch (rcvd_app_msg->tag.cmd_cls) {
    case SLI_SID_APP_MSG_CMD_CLS_DEV_MGMT:
      SLI_SID_APP_MSG_RETURN_ST_IF_FAILED(sli_sid_app_msg_dev_mgmt_cmd_handler(rcvd_app_msg));
      break;

    case SLI_SID_APP_MSG_CMD_CLS_SID:
      SLI_SID_APP_MSG_RETURN_ST_IF_FAILED(sli_sid_app_msg_sid_cmd_handler(rcvd_app_msg));
      break;

    case SLI_SID_APP_MSG_CMD_CLS_DMP_SOC_LIGHT:
      SLI_SID_APP_MSG_RETURN_ST_IF_FAILED(sli_sid_app_msg_dmp_soc_light_cmd_handler(rcvd_app_msg));
      break;

    default:
      return SL_SID_APP_MSG_ERR_ST_PKT_WRONG_CMD_CLS;
  }

  return SL_SID_APP_MSG_ERR_ST_SUCCESS;
}

sl_sid_app_msg_st_t sl_sid_app_msg_prepare_sid_msg(sl_sid_app_msg_t *app_msg, struct sid_msg *sid_msg)
{
  SLI_SID_APP_MSG_RETURN_ST_IF_PARAM_INVALID_2(app_msg, sid_msg);

  SLI_SID_APP_MSG_RETURN_ST_IF_FAILED(validate(app_msg));
  uint8_t *pkt;
  uint16_t pkt_len;
  SLI_SID_APP_MSG_RETURN_ST_IF_FAILED(serialize(app_msg, &pkt, &pkt_len));
  sid_msg->data = (void *)pkt;
  sid_msg->size = (size_t)pkt_len;

  return SL_SID_APP_MSG_ERR_ST_SUCCESS;
}

sl_sid_app_msg_st_t sli_sid_app_msg_prepare_send(
  sl_sid_app_msg_t *app_msg, uint8_t cmd_cls, uint8_t cmd_id, sl_sid_app_msg_op_t op, uint8_t seq, void *val, uint16_t val_len)
{
  SLI_SID_APP_MSG_RETURN_ST_IF_PARAM_INVALID_2(app_msg, val);

  sl_sid_app_msg_op_t op_send;
  SLI_SID_APP_MSG_RETURN_ST_IF_FAILED(sli_sid_app_msg_prepare_op_for_send(op, &op_send));

  if (op_send == SL_SID_APP_MSG_OP_NTFY || op_send == SL_SID_APP_MSG_OP_SET || op_send == SL_SID_APP_MSG_OP_GET) {
    uint8_t seq_gen;
    SLI_SID_APP_MSG_GENERATE_SEQ_NO(&seq_gen, sizeof(uint8_t));
    app_msg->tag.seq = seq_gen;
  } else if (op_send == SL_SID_APP_MSG_OP_ACK || op_send == SL_SID_APP_MSG_OP_RESP) {
    app_msg->tag.seq = seq;
  } else {
    return SL_SID_APP_MSG_ERR_ST_PKT_WRONG_OP;
  }
  app_msg->tag.proto_ver = SLI_SID_APP_MSG_PROTO_VER;
  app_msg->tag.cmd_cls = cmd_cls;
  app_msg->tag.cmd_id = cmd_id;
  app_msg->tag.op = op_send;
  app_msg->length = val_len;
  memcpy((void *)app_msg->value, (const void *)val, app_msg->length);

  return SL_SID_APP_MSG_ERR_ST_SUCCESS;
}

/*******************************************************************************
 *** STATIC FUNCTIONS
 ******************************************************************************/
static sl_sid_app_msg_st_t serialize(sl_sid_app_msg_t *msg, uint8_t **pkt, uint16_t *pkt_len)
{
  SLI_SID_APP_MSG_RETURN_ST_IF_PARAM_INVALID_3(msg, pkt, pkt_len);

  *pkt = (uint8_t *)&msg[0];
  *pkt_len = msg->length + SLI_SID_APP_MSG_HEADER_LEN_BYTES;

  if (*pkt_len > SLI_SID_APP_MSG_MAX_MTU_SIZE) {
    return SL_SID_APP_MSG_ERR_ST_PKT_WRONG_LEN;
  }

  return SL_SID_APP_MSG_ERR_ST_SUCCESS;
}

static sl_sid_app_msg_st_t deserialize(sl_sid_app_msg_t **msg, uint8_t *pkt, uint16_t pkt_len)
{
  SLI_SID_APP_MSG_RETURN_ST_IF_PARAM_INVALID_2(msg, pkt);

  *msg = (sl_sid_app_msg_t *)&pkt[0];

  if ((pkt_len > SLI_SID_APP_MSG_MAX_MTU_SIZE) ||
      (pkt_len != ((*msg)->length + SLI_SID_APP_MSG_HEADER_LEN_BYTES))) {
    return SL_SID_APP_MSG_ERR_ST_PKT_WRONG_LEN;
  }

  return SL_SID_APP_MSG_ERR_ST_SUCCESS;
}

static sl_sid_app_msg_st_t validate(sl_sid_app_msg_t *msg)
{
  SLI_SID_APP_MSG_RETURN_ST_IF_PARAM_INVALID_1(msg);

  if (msg->tag.proto_ver != SLI_SID_APP_MSG_PROTO_VER) {
    return SL_SID_APP_MSG_ERR_ST_PKT_WRONG_PROTO_VER;
  }

  if (msg->tag.cmd_cls >= (1 << SLI_SID_APP_MSG_CMD_CLS_BITS)) {
    return SL_SID_APP_MSG_ERR_ST_PKT_WRONG_CMD_CLS;
  }

  if (msg->tag.cmd_id >= (1 << SLI_SID_APP_MSG_CMD_ID_BITS)) {
    return SL_SID_APP_MSG_ERR_ST_PKT_WRONG_CMD_ID;
  }

  if (msg->tag.op >= (1 << SLI_SID_APP_MSG_OP_BITS)) {
    return SL_SID_APP_MSG_ERR_ST_PKT_WRONG_OP;
  }

  if (msg->tag.seq >= (1 << SLI_SID_APP_MSG_SEQ_BITS)) {
    return SL_SID_APP_MSG_ERR_ST_PKT_WRONG_SEQ;
  }

  return SL_SID_APP_MSG_ERR_ST_SUCCESS;
}

static sl_sid_app_msg_st_t receive(sl_sid_app_msg_t **app_msg, const struct sid_msg *sid_msg)
{
  SLI_SID_APP_MSG_RETURN_ST_IF_PARAM_INVALID_2(app_msg, sid_msg);

  SLI_SID_APP_MSG_RETURN_ST_IF_FAILED(deserialize(app_msg, (uint8_t *)sid_msg->data, (uint16_t)sid_msg->size));
  SLI_SID_APP_MSG_RETURN_ST_IF_FAILED(validate(*app_msg));

  return SL_SID_APP_MSG_ERR_ST_SUCCESS;
}

static inline sl_sid_app_msg_st_t sli_sid_app_msg_prepare_op_for_send(
  sl_sid_app_msg_op_t op_rcv, sl_sid_app_msg_op_t *op_send)
{
  if (op_rcv == SL_SID_APP_MSG_OP_NTFY) {
    *op_send = SL_SID_APP_MSG_OP_NTFY;
  } else if (op_rcv == SL_SID_APP_MSG_OP_SET) {
    *op_send = SL_SID_APP_MSG_OP_ACK;
  } else if (op_rcv == SL_SID_APP_MSG_OP_GET) {
    *op_send = SL_SID_APP_MSG_OP_RESP;
  } else {
    return SL_SID_APP_MSG_ERR_ST_APP_WRONG_OP;
  }

  return SL_SID_APP_MSG_ERR_ST_SUCCESS;
}

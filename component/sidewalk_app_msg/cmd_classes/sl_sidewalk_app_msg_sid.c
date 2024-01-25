/***************************************************************************//**
 * @file sl_sidewalk_app_msg_sid.c
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

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/

#include "sl_sidewalk_app_msg_sid.h"

/*******************************************************************************
 *** GLOBAL FUNCTIONS
 ******************************************************************************/

sl_sid_app_msg_st_t sli_sid_app_msg_sid_cmd_handler(sl_sid_app_msg_t *msg)
{
  sl_sid_app_msg_st_t status = SL_SID_APP_MSG_ERR_ST_SUCCESS;

  switch (msg->tag.cmd_id) {
    case SLI_SID_APP_MSG_CMD_ID_SID_MTU:
      {
        if (msg->tag.op != SL_SID_APP_MSG_OP_GET) {
          return SL_SID_APP_MSG_ERR_ST_APP_WRONG_OP;
        }

        sl_sid_app_msg_sid_mtu_ctx_t ctx = {
          .param_rcv.link_type = ((sli_sid_app_msg_sid_mtu_get_t *)msg->value)->link_type,
          .hdl.operation = msg->tag.op,
          .hdl.sequence = msg->tag.seq
        };
        sl_sid_app_msg_sid_mtu_cb(&ctx);
      }
      break;

    case SLI_SID_APP_MSG_CMD_ID_SID_TIME:
      {
        if (msg->tag.op != SL_SID_APP_MSG_OP_GET) {
          return SL_SID_APP_MSG_ERR_ST_APP_WRONG_OP;
        }

        sl_sid_app_msg_sid_time_ctx_t ctx = {
          .hdl.operation = msg->tag.op,
          .hdl.sequence = msg->tag.seq
        };
        sl_sid_app_msg_sid_time_cb(&ctx);
      }
      break;

    default:
      status = SL_SID_APP_MSG_ERR_ST_APP_CMD_HDL_NOT_IMPL;
      break;
  }

  return status;
}

sl_sid_app_msg_st_t sl_sid_app_msg_sid_mtu_prepare_send(
  sl_sid_app_msg_sid_mtu_ctx_t *ctx, sl_sid_app_msg_t *send_app_msg)
{
  SLI_SID_APP_MSG_CLR_PROCESSING_FLAG(ctx);

  if (ctx->hdl.operation == SL_SID_APP_MSG_OP_GET || ctx->hdl.operation == SL_SID_APP_MSG_OP_NTFY) {
    return SLI_SID_APP_MSG_PREP_SEND_PARAM_FUNC(SLI_SID_APP_MSG_CMD_CLS_SID, SLI_SID_APP_MSG_CMD_ID_SID_MTU);
  } else {
    return SL_SID_APP_MSG_ERR_ST_APP_WRONG_OP;
  }
}

sl_sid_app_msg_st_t sl_sid_app_msg_sid_time_prepare_send(
  sl_sid_app_msg_sid_time_ctx_t *ctx, sl_sid_app_msg_t *send_app_msg)
{
  SLI_SID_APP_MSG_CLR_PROCESSING_FLAG(ctx);

  if (ctx->hdl.operation == SL_SID_APP_MSG_OP_GET || ctx->hdl.operation == SL_SID_APP_MSG_OP_NTFY) {
    return SLI_SID_APP_MSG_PREP_SEND_PARAM_FUNC(SLI_SID_APP_MSG_CMD_CLS_SID, SLI_SID_APP_MSG_CMD_ID_SID_TIME);
  } else {
    return SL_SID_APP_MSG_ERR_ST_APP_WRONG_OP;
  }
}

SL_WEAK void sl_sid_app_msg_sid_mtu_cb(sl_sid_app_msg_sid_mtu_ctx_t *ctx) { (void)ctx; }

SL_WEAK void sl_sid_app_msg_sid_time_cb(sl_sid_app_msg_sid_time_ctx_t *ctx) { (void)ctx; }

/***************************************************************************//**
 * @file sl_sidewalk_app_msg_dev_mgmt.c
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

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/

#include "sl_sidewalk_app_msg_dev_mgmt.h"

/*******************************************************************************
 *** GLOBAL FUNCTIONS
 ******************************************************************************/

sl_sid_app_msg_st_t sli_sid_app_msg_dev_mgmt_cmd_handler(sl_sid_app_msg_t *msg)
{
  sl_sid_app_msg_st_t status = SL_SID_APP_MSG_ERR_ST_SUCCESS;

  switch (msg->tag.cmd_id) {
    case SLI_SID_APP_MSG_CMD_ID_DEV_MGMT_RST_DEV:
      {
        if (msg->tag.op != SL_SID_APP_MSG_OP_SET) {
          return SL_SID_APP_MSG_ERR_ST_APP_WRONG_OP;
        }

        sl_sid_app_msg_dev_mgmt_rst_dev_ctx_t ctx = {
          .param_send.in_millisecs = ((sli_sid_app_msg_dev_mgmt_rst_dev_set_t *)msg->value)->in_millisecs,
          .param_send.reset_type = ((sli_sid_app_msg_dev_mgmt_rst_dev_set_t *)msg->value)->reset_type,
          .hdl.operation = msg->tag.op,
          .hdl.sequence = msg->tag.seq
        };
        sl_sid_app_msg_dev_mgmt_rst_dev_cb(&ctx);
      }
      break;

    case SLI_SID_APP_MSG_CMD_ID_DEV_MGMT_BUTTON_PRESS:
      {
        if (msg->tag.op != SL_SID_APP_MSG_OP_SET) {
          return SL_SID_APP_MSG_ERR_ST_APP_WRONG_OP;
        }

        sl_sid_app_msg_dev_mgmt_button_press_ctx_t ctx = {
          .param_send.button = ((sli_sid_app_msg_dev_mgmt_button_press_set_t *)msg->value)->button,
          .param_send.duration = ((sli_sid_app_msg_dev_mgmt_button_press_set_t *)msg->value)->duration,
          .is_emulation = true,
          .hdl.operation = msg->tag.op,
          .hdl.sequence = msg->tag.seq
        };
        sl_sid_app_msg_dev_mgmt_button_press_cb(&ctx);
      }
      break;

    case SLI_SID_APP_MSG_CMD_ID_DEV_MGMT_TOGGLE_LED:
      {
        if (msg->tag.op != SL_SID_APP_MSG_OP_SET) {
          return SL_SID_APP_MSG_ERR_ST_APP_WRONG_OP;
        }

        sl_sid_app_msg_dev_mgmt_toggle_led_ctx_t ctx = {
          .param_send.led = ((sli_sid_app_msg_dev_mgmt_toggle_led_set_t *)msg->value)->led,
          .hdl.operation = msg->tag.op,
          .hdl.sequence = msg->tag.seq
        };
        sl_sid_app_msg_dev_mgmt_toggle_led_cb(&ctx);
      }
      break;

    default:
      status = SL_SID_APP_MSG_ERR_ST_APP_CMD_HDL_NOT_IMPL;
      break;
  }

  return status;
}

sl_sid_app_msg_st_t sl_sid_app_msg_dev_mgmt_rst_dev_prepare_send(
  sl_sid_app_msg_dev_mgmt_rst_dev_ctx_t *ctx, sl_sid_app_msg_t *send_app_msg)
{
  SLI_SID_APP_MSG_CLR_PROCESSING_FLAG(ctx);

  if (ctx->hdl.operation == SL_SID_APP_MSG_OP_SET) {
    return SLI_SID_APP_MSG_PREP_SEND_ACK_FUNC(SLI_SID_APP_MSG_CMD_CLS_DEV_MGMT, SLI_SID_APP_MSG_CMD_ID_DEV_MGMT_RST_DEV);
  } else if (ctx->hdl.operation == SL_SID_APP_MSG_OP_NTFY) {
    return SLI_SID_APP_MSG_PREP_SEND_PARAM_FUNC(SLI_SID_APP_MSG_CMD_CLS_DEV_MGMT, SLI_SID_APP_MSG_CMD_ID_DEV_MGMT_RST_DEV);
  } else {
    return SL_SID_APP_MSG_ERR_ST_APP_WRONG_OP;
  }
}

sl_sid_app_msg_st_t sl_sid_app_msg_dev_mgmt_button_press_prepare_send(
  sl_sid_app_msg_dev_mgmt_button_press_ctx_t *ctx, sl_sid_app_msg_t *send_app_msg)
{
  SLI_SID_APP_MSG_CLR_PROCESSING_FLAG(ctx);

  if (ctx->hdl.operation == SL_SID_APP_MSG_OP_SET) {
    return SLI_SID_APP_MSG_PREP_SEND_ACK_FUNC(SLI_SID_APP_MSG_CMD_CLS_DEV_MGMT, SLI_SID_APP_MSG_CMD_ID_DEV_MGMT_BUTTON_PRESS);
  } else if (ctx->hdl.operation == SL_SID_APP_MSG_OP_NTFY) {
    return SLI_SID_APP_MSG_PREP_SEND_PARAM_FUNC(SLI_SID_APP_MSG_CMD_CLS_DEV_MGMT, SLI_SID_APP_MSG_CMD_ID_DEV_MGMT_BUTTON_PRESS);
  } else {
    return SL_SID_APP_MSG_ERR_ST_APP_WRONG_OP;
  }
}

sl_sid_app_msg_st_t sl_sid_app_msg_dev_mgmt_toggle_led_prepare_send(
  sl_sid_app_msg_dev_mgmt_toggle_led_ctx_t *ctx, sl_sid_app_msg_t *send_app_msg)
{
  SLI_SID_APP_MSG_CLR_PROCESSING_FLAG(ctx);

  if (ctx->hdl.operation == SL_SID_APP_MSG_OP_SET) {
    return SLI_SID_APP_MSG_PREP_SEND_ACK_FUNC(SLI_SID_APP_MSG_CMD_CLS_DEV_MGMT, SLI_SID_APP_MSG_CMD_ID_DEV_MGMT_TOGGLE_LED);
  } else if (ctx->hdl.operation == SL_SID_APP_MSG_OP_NTFY) {
    return SLI_SID_APP_MSG_PREP_SEND_PARAM_FUNC(SLI_SID_APP_MSG_CMD_CLS_DEV_MGMT, SLI_SID_APP_MSG_CMD_ID_DEV_MGMT_TOGGLE_LED);
  } else {
    return SL_SID_APP_MSG_ERR_ST_APP_WRONG_OP;
  }
}

SL_WEAK void sl_sid_app_msg_dev_mgmt_rst_dev_cb(sl_sid_app_msg_dev_mgmt_rst_dev_ctx_t *ctx) { (void)ctx; }

SL_WEAK void sl_sid_app_msg_dev_mgmt_button_press_cb(sl_sid_app_msg_dev_mgmt_button_press_ctx_t *ctx) { (void)ctx; }

SL_WEAK void sl_sid_app_msg_dev_mgmt_toggle_led_cb(sl_sid_app_msg_dev_mgmt_toggle_led_ctx_t *ctx) { (void)ctx; }
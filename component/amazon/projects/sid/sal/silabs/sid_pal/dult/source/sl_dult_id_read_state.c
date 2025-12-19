/***************************************************************************//**
 * @file  sl_dult_id_read_state.c
 * @brief Implementation of DULT Identifier Read State
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sli_dult.h"
#include "sl_sleeptimer.h"
#include <string.h>

#define DULT_IDENTIFIER_READ_STATE_TIMEOUT_MS                                  \
  SL_DULT_CONFIG_IDENTIFIER_READ_STATE_TIMEOUT_MS

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

static void dult_id_read_state_close(sl_sleeptimer_timer_handle_t *handle,
                                     void *data);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/**
 * @brief Enters DULT Read Identifier State
 *
 *  3.12.4.2. Identifier Payload
 *  The identifier read state MUST be enabled for 5 minutes once the user action
 *  on the accessory is successfully performed. When the accessory is in this
 * mode, it MUST respond with Get_Identifier_Response opcode and Identifier
 * Payload operand.
 *
 * @returns sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_identifier_payload_enter(uint8_t handle)
{
  sl_status_t status;
  sli_dult_ctx_t *dult_ctx;

  status = sli_dult_get_context(handle, &dult_ctx);
  if (status != SL_STATUS_OK) {
    DULT_LOG_ERROR("Failed to get dult context: %#lx", status);
    return status;
  }

  DULT_LOG_INFO("%s: network_id: %d", __func__, dult_ctx->info.network_id);

  do {
    if (dult_ctx->near_owner_state != SL_DULT_STATE_SEPARATED) {
      DULT_LOG_INFO("Identifier read state is not allowed in state %d",
                    dult_ctx->near_owner_state);
      break;
    }
    if (dult_ctx->id_read_ctx.read_state_enabled) {
      status = SL_STATUS_IN_PROGRESS;
      DULT_LOG_INFO("%s: Identifier read state is already in progress",
                    __func__);
      break;
    }
    status =
        sl_sleeptimer_start_timer_ms(&dult_ctx->id_read_ctx.read_state_timer,
                                     DULT_IDENTIFIER_READ_STATE_TIMEOUT_MS,
                                     dult_id_read_state_close,
                                     dult_ctx,
                                     0,
                                     0);
    if (status != SL_STATUS_OK) {
      DULT_LOG_ERROR("%s: Failed to start timer: %ld", __func__, status);
      break;
    }
    dult_ctx->id_read_ctx.read_state_enabled = true;
    DULT_LOG_INFO("%s success", __func__);
  } while (false);

  return status;
}

sl_status_t sli_dult_identifier_payload_stop(sli_dult_ctx_t *dult_ctx)
{
  bool running;
  sl_status_t status = sl_sleeptimer_is_timer_running(&dult_ctx->id_read_ctx.read_state_timer, &running);
  SLI_DULT_ASSERT_ST(status);
  if (running) {
    status = sl_sleeptimer_stop_timer(&dult_ctx->id_read_ctx.read_state_timer);
    SLI_DULT_ASSERT_ST(status);
  }
  dult_ctx->id_read_ctx.read_state_enabled = false;
  
  return status;
}

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

static void dult_id_read_state_close(sl_sleeptimer_timer_handle_t *handle,
                                     void *data)
{
  /* Avoid warning */
  (void) (handle);
  
  if (data == NULL) {
    DULT_LOG_ERROR("Timer callback called with NULL context");
    return;
  }
  
  sli_dult_ctx_t *dult_ctx                 = (sli_dult_ctx_t *) data;
  dult_ctx->id_read_ctx.read_state_enabled = false;
  DULT_LOG_INFO(
      "DULT Identifier Read State exited due to timeout, network_id: %d",
      dult_ctx->info.network_id);
}

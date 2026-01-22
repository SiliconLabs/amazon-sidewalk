/***************************************************************************//**
 * @file  sl_dult_near_owner_state.c
 * @brief Implementation of DULT Near Owner state
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

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/**
 * @brief Set the current owner proximity state
 *
 * @param state Indicates if the device is near owner or separated
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_set_near_owner_state(uint8_t handle,
                                         sl_dult_near_owner_state_t state)
{
  sl_status_t status;
  sli_dult_ctx_t *dult_ctx;

  if (state != SL_DULT_STATE_NEAR_OWNER && state != SL_DULT_STATE_SEPARATED) {
    DULT_LOG_ERROR("%s: Invalid input: %d", __func__, state);
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = sli_dult_get_context(handle, &dult_ctx);
  if (status != SL_STATUS_OK) {
    DULT_LOG_ERROR("%s: Failed to get dult context: %#lx", __func__, status);
    return status;
  }

  if (dult_ctx->near_owner_state != state) {
    // Near owner state has changed
    dult_ctx->near_owner_state = state;

    sli_dult_motion_near_owner_changed_notify(dult_ctx);
    sli_dult_adv_near_owner_changed_notify(dult_ctx);

    // Notify the application
    if (dult_ctx->callback.on_near_owner_state_change) {
      dult_ctx->callback.on_near_owner_state_change(state,
                                                    dult_ctx->callback.context);
    } else {
      DULT_LOG_ERROR("%s: No callback function registered", __func__);
    }
  }

  return SL_STATUS_OK;
}

/**
 * @brief Get the current owner proximity state
 *
 * @param handle DULT handle to query
 * @param state Pointer to store the current owner proximity state
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_get_near_owner_state(uint8_t handle, sl_dult_near_owner_state_t *state)
{
  sli_dult_ctx_t *dult_ctx;
  sl_status_t status;

  if (state == NULL) {
    DULT_LOG_ERROR("%s: Invalid input: NULL state pointer", __func__);
    return SL_STATUS_NULL_POINTER;
  }

  status = sli_dult_get_context(handle, &dult_ctx);
  if (status != SL_STATUS_OK) {
    DULT_LOG_ERROR("%s: Failed to get dult context: %#lx", __func__, status);
    return status;
  }

  *state = dult_ctx->near_owner_state;
  return SL_STATUS_OK;
}

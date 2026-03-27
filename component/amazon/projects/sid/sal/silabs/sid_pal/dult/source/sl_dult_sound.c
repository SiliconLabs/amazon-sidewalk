/***************************************************************************//**
 * @file  sl_dult_sound.c
 * @brief Implementation of DULT Sound state
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

#define DULT_PLAY_SOUND_DURATION_MIN_MS 5000

static sl_status_t sound_state_change_notify(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t sc = SL_STATUS_OK;

  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (dult_ctx->sound.active == false) {
    /* Sound stop*/
    if (dult_ctx->sound.state == SLI_DULT_SOUND_STATE_START_ACK
        || dult_ctx->sound.state == SLI_DULT_SOUND_STATE_STOP_REQUEST) {
      // External stop / GATT stop
      DULT_LOG_DEBUG("%s: Sound stop notify, src: %d",
                     __func__,
                     dult_ctx->sound.src);
      sc = sli_dult_non_owner_send_sound_cmpl(dult_ctx);
      if (sc != SL_STATUS_OK) {
        DULT_LOG_ERROR("Failed to send sound stop indication: 0x%#lx", sc);
        return sc;
      }
    }
    dult_ctx->sound.state = SLI_DULT_SOUND_STATE_IDLE;
  } else {
    /* Sound start*/
    if (dult_ctx->sound.state == SLI_DULT_SOUND_STATE_START_ACK
        || dult_ctx->sound.state == SLI_DULT_SOUND_STATE_STOP_REQUEST) {
      DULT_LOG_DEBUG("Sound started in unexpected DULT sound state");
      DULT_LOG_DEBUG("Change state to External took over");
      dult_ctx->sound.state = SLI_DULT_SOUND_STATE_EXTERNAL_TAKEOVER;
      return SL_STATUS_OK;
    }
    if (dult_ctx->sound.state == SLI_DULT_SOUND_STATE_START_REQUEST
        && dult_ctx->sound.src == SL_DULT_NON_OWNER_FIND_EVENT_SRC_BT_GATT) {
      DULT_LOG_DEBUG("%s: Sound start notify, src: %d",
                     __func__,
                     dult_ctx->sound.src);
      dult_ctx->sound.state = SLI_DULT_SOUND_STATE_START_ACK;

      sc = sli_dult_sound_cmd_resp(dult_ctx,
                                   SLI_DULT_OC_NON_OWNER_SOUND_START,
                                   SL_STATUS_OK);
      if (sc != SL_STATUS_OK) {
        DULT_LOG_ERROR("Sound command response failed: 0x%#lx", sc);
        return sc;
      }
      DULT_LOG_DEBUG("GATT sound start success");
    }
    if (dult_ctx->sound.state == SLI_DULT_SOUND_STATE_START_REQUEST
        && dult_ctx->sound.src != SL_DULT_NON_OWNER_FIND_EVENT_SRC_BT_GATT) {
      // External took over
      DULT_LOG_DEBUG("External took over");
      sc = sli_dult_sound_cmd_resp(dult_ctx,
                                   SLI_DULT_OC_NON_OWNER_SOUND_START,
                                   SL_STATUS_INVALID_STATE);
      if (sc != SL_STATUS_OK) {
        DULT_LOG_ERROR("Sound start failed: 0x%#lx", sc);
        return sc;
      }
      dult_ctx->sound.state = SLI_DULT_SOUND_STATE_IDLE;
      DULT_LOG_DEBUG("Sound state reset");
    }
  }
  return sc;
}
/**
 * @brief Send sound command response
 * @param[in] dult_ctx The pointer to DULT context
 */
void sli_dult_bt_sound_init(sli_dult_ctx_t *dult_ctx)
{
  if (dult_ctx == NULL) {
    return;
  }

  dult_ctx->sound.is_initialized = true;
  dult_ctx->sound.state          = SLI_DULT_SOUND_STATE_IDLE;
}

/**
 * @brief Reset sound context to the state after initialization
 * @param[in] dult_ctx The pointer to DULT context
 */
void sli_dult_bt_sound_reset(sli_dult_ctx_t *dult_ctx)
{
  if (dult_ctx == NULL) {
    return;
  }

  dult_ctx->sound.state      = SLI_DULT_SOUND_STATE_IDLE;
  dult_ctx->sound.active     = false;
  dult_ctx->sound.start_time = 0;
  dult_ctx->sound.src        = 0;
}

/**
 * @brief Sound start handle
 * @param[in] dult_ctx The pointer to DULT context
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sli_dult_bt_sound_start(sli_dult_ctx_t *dult_ctx)
{
  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // Sound start only available when the accessory is in the separated state
  if (dult_ctx->near_owner_state != SL_DULT_STATE_SEPARATED) {
    return sli_dult_sound_cmd_resp(dult_ctx,
                                   SLI_DULT_OC_NON_OWNER_SOUND_START,
                                   SL_STATUS_FAIL);
  }

  if (!dult_ctx->sound.is_initialized) {
    return sli_dult_sound_cmd_resp(dult_ctx,
                                   SLI_DULT_OC_NON_OWNER_SOUND_START,
                                   SL_STATUS_NOT_INITIALIZED);
  }

  sl_dult_non_owner_find_event_t event = {
      .type   = SL_DULT_NON_OWNER_FIND_EVENT_TYPE_SOUND,
      .src    = SL_DULT_NON_OWNER_FIND_EVENT_SRC_BT_GATT,
      .action = SL_DULT_NON_OWNER_FIND_EVENT_ACTION_START,
  };
  if (dult_ctx->sound.state != SLI_DULT_SOUND_STATE_IDLE) {
    return sli_dult_sound_cmd_resp(dult_ctx,
                                   SLI_DULT_OC_NON_OWNER_SOUND_START,
                                   SL_STATUS_INVALID_STATE);
  }
  dult_ctx->sound.state      = SLI_DULT_SOUND_STATE_START_REQUEST;
  dult_ctx->sound.start_time = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
  if (dult_ctx->callback.on_non_owner_find_event != NULL) {
    dult_ctx->callback.on_non_owner_find_event(&event,
                                               dult_ctx->callback.context);
  }
  DULT_LOG_DEBUG("Sound state: %d", dult_ctx->sound.state);
  return SL_STATUS_OK;
}

/**
 * @brief Sound stop handle
 * @param[in] dult_ctx The pointer to DULT context
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sli_dult_bt_sound_stop(sli_dult_ctx_t *dult_ctx)
{
  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  uint32_t current_time = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
  if (!dult_ctx->sound.is_initialized) {
    return sli_dult_sound_cmd_resp(dult_ctx,
                                   SLI_DULT_OC_NON_OWNER_SOUND_STOP,
                                   SL_STATUS_NOT_INITIALIZED);
  }

  // The sound maker MUST play sound for a minimum duration of 5 seconds
  if ((current_time - dult_ctx->sound.start_time) < DULT_PLAY_SOUND_DURATION_MIN_MS) {
    return sli_dult_sound_cmd_resp(dult_ctx,
                                   SLI_DULT_OC_NON_OWNER_SOUND_STOP,
                                   SL_STATUS_INVALID_STATE);
  }
  // Sound stop only available when the accessory is in the separated state
  if (dult_ctx->near_owner_state != SL_DULT_STATE_SEPARATED) {
    return sli_dult_sound_cmd_resp(dult_ctx,
                                   SLI_DULT_OC_NON_OWNER_SOUND_STOP,
                                   SL_STATUS_FAIL);
  }
  if (dult_ctx->sound.state != SLI_DULT_SOUND_STATE_START_ACK) {
    return sli_dult_sound_cmd_resp(dult_ctx,
                                   SLI_DULT_OC_NON_OWNER_SOUND_STOP,
                                   SL_STATUS_INVALID_STATE);
  }
  sl_dult_non_owner_find_event_t event = {
      .type   = SL_DULT_NON_OWNER_FIND_EVENT_TYPE_SOUND,
      .src    = SL_DULT_NON_OWNER_FIND_EVENT_SRC_BT_GATT,
      .action = SL_DULT_NON_OWNER_FIND_EVENT_ACTION_STOP,
  };
  dult_ctx->sound.state = SLI_DULT_SOUND_STATE_STOP_REQUEST;
  if (dult_ctx->callback.on_non_owner_find_event != NULL) {
    dult_ctx->callback.on_non_owner_find_event(&event,
                                               dult_ctx->callback.context);
  }
  DULT_LOG_DEBUG("Sound state: %d", dult_ctx->sound.state);
  return SL_STATUS_OK;
}

sl_status_t sli_dult_motion_sound_start(sli_dult_ctx_t *dult_ctx)
{
  if (dult_ctx == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (dult_ctx->sound.state != SLI_DULT_SOUND_STATE_IDLE) {
    return SL_STATUS_INVALID_STATE;
  }
  sl_dult_non_owner_find_event_t event = {
      .type   = SL_DULT_NON_OWNER_FIND_EVENT_TYPE_SOUND,
      .src    = SL_DULT_NON_ONWER_FIND_EVENT_SRC_MOTION_DETECTION,
      .action = SL_DULT_NON_OWNER_FIND_EVENT_ACTION_START,
  };
  if (dult_ctx->callback.on_non_owner_find_event != NULL) {
    dult_ctx->callback.on_non_owner_find_event(&event,
                                               dult_ctx->callback.context);
  }
  return SL_STATUS_OK;
}

sl_status_t sl_dult_update_find_event_status(uint8_t handle,
                                             bool sound_active,
                                             const sl_dult_non_owner_find_event_src_t src,
                                             const sl_dult_non_owner_find_event_type_t type)
{
  if (type != SL_DULT_NON_OWNER_FIND_EVENT_TYPE_SOUND) {
    return SL_STATUS_OK;
  }
  sli_dult_ctx_t *dult_ctx;
  sl_status_t sc = sli_dult_get_context(handle, &dult_ctx);
  if (sc != SL_STATUS_OK) {
    DULT_LOG_ERROR("%s: Invalid dult handle: 0x%#lx", __func__, sc);
    return sc;
  }

  if (dult_ctx->info.capabilities &
      SL_DULT_ACCESSORY_CAPABILITY_MOTION_DETECTOR_UT) {
    // Notify motion detection about sound status
    sli_dult_motion_detection_sound_notify(dult_ctx, sound_active);
  }

  /* DULT Non-owner control service handle sound update action */
  if (dult_ctx->sound.state == SLI_DULT_SOUND_STATE_IDLE) {
    /*In idle state, we don't need handle this action*/
    DULT_LOG_DEBUG("%s: Sound state is idle", __func__);
    return SL_STATUS_OK;
  }

  if (sound_active == dult_ctx->sound.active) {
    if (src == dult_ctx->sound.src) {
      DULT_LOG_DEBUG("%s: Sound state has not changed", __func__);
      return SL_STATUS_OK;
    }
    if (!dult_ctx->sound.active) {
      DULT_LOG_DEBUG("%s: unnecessary source change when sound is not active",
                     __func__);
      return SL_STATUS_OK;
    }
  }
  dult_ctx->sound.active = sound_active;
  dult_ctx->sound.src    = src;
  sc                     = sound_state_change_notify(dult_ctx);

  return sc;
}

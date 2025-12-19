/***************************************************************************//**
 * @file  sl_dult_motion_detection.c
 * @brief Implementation of DULT Motion detection
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
#include "sl_bt_api.h"
#include "sli_dult_adaptation.h"

/*******************************************************************************
 *****************************   DEFINITIONS   *********************************
 ******************************************************************************/

/**
 * @brief
 * Sampling rate used in state SL_DULT_MOTION_DETECTION_STATE_SAMPLING_SLOW.
 * Described as T_SEPARATED_UT_SAMPLING_RATE1 in the specification.
 * Unit: ms
 */
#define SEPARATED_SLOW_SAMPLING_MS (10 * 1000)

/**
 * @brief
 * Sampling rate used in state SL_DULT_MOTION_DETECTION_STATE_SAMPLING_FAST.
 * Described as T_SEPARATED_UT_SAMPLING_RATE2 in the specification.
 * Unit: ms
 */
#define SEPARATED_FAST_SAMPLING_MS (500)

#define FAST_SAMPLING_TIMEOUT_MS (20 * 1000)

/**
 * @brief
 * Back of time, described as T_SEPARATED_UT_BACKOFF  in the specification.
 * Unit: ms
 */
#define SEPARATED_BACKOFF_MS (SL_DULT_CONFIG_SEPARATED_BACKOFF_MINUTE * 60 * 1000)

/**
 * @brief
 * The minimum value of T_SEPARATED_UT_TIMEOUT
 * Unit: minute
 */
#define SEPARATED_TIMEOUT_MIN_MINUTE SL_DULT_CONFIG_SEPARATED_TIMEOUT_MIN_MINUTE

/**
 * @brief
 * The maximum value of T_SEPARATED_UT_TIMEOUT
 * Unit: minute
 */
#define SEPARATED_TIMEOUT_MAX_MINUTE SL_DULT_CONFIG_SEPARATED_TIMEOUT_MAX_MINUTE

#define SEPARATED_TIMEOUT_DIFF_MINUTE                                          \
  (SEPARATED_TIMEOUT_MAX_MINUTE - SEPARATED_TIMEOUT_MIN_MINUTE)

#define MAX_SOUND_COUNT_BEFORE_BACKOFF 10

typedef enum {
  MOTION_POLL_STATE_UNINITIALIZED,
  MOTION_POLL_STATE_STOPPED,
  MOTION_POLL_STATE_PASSIVE,
  MOTION_POLL_STATE_PASSIVE_SOUND_PLAYING,
  MOTION_POLL_STATE_ACTIVE,
  MOTION_POLL_STATE_ACTIVE_SOUND_PLAYING,
} motion_poll_state_t;

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

static void state_reset(sli_dult_ctx_t *dult_ctx);
static void backoff_setup(sli_dult_ctx_t *dult_ctx);
static void scheduler_rotation_timer(sli_dult_ctx_t *ctx);
static void separated_mode_transition_handle(sli_dult_ctx_t *dult_ctx);
static void near_owner_mode_transition_handle(sli_dult_ctx_t *dult_ctx);
static void timer_motion_enable_callback(void *ctx);
static void timer_poll_callback(void *ctx);
static void timer_motion_poll_duration_callback(void *ctx);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/**
 * @brief Update the status of motion detection
 *
 * The motion detection implementation shoule be followed by 3.13.2.1. Implementation
 *
 * @param[in] status True if motion has been detected else false
 * @return[out] sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_update_motion_detection(uint8_t handle, bool status)
{
  sli_dult_ctx_t *dult_ctx;
  sl_status_t sc = SL_STATUS_OK;

  sc = sli_dult_get_context(handle, &dult_ctx);
  if (sc != SL_STATUS_OK) {
    DULT_LOG_ERROR("%s: Invalid dult handle: 0x%#lx", __func__, sc);
    return sc;
  }

  if (!dult_ctx->motion_ctx.is_enabled) {
    return SL_STATUS_INVALID_STATE;
  }

  if (status && (dult_ctx->motion_ctx.state == MOTION_POLL_STATE_PASSIVE ||
                 dult_ctx->motion_ctx.state == MOTION_POLL_STATE_ACTIVE)) {
    sc = sli_dult_motion_sound_start(dult_ctx);
  }

  return sc;
}

/**
 * @brief Enable/Disable trigger non-owner find event based on motion detection
 *
 * The motion detection implementation shoule be followed by 3.13.2.1. Implementation
 *
 * @param[in] set True to enable motion detection, false to disable motion detection
 * @return[out] sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sl_dult_set_motion_detection(uint8_t handle, bool set)
{
  sli_dult_ctx_t *dult_ctx;
  sl_status_t sc;

  sc = sli_dult_get_context(handle, &dult_ctx);
  if (SL_STATUS_OK != sc) {
    DULT_LOG_ERROR("%s: Invalid dult handle: 0x%#lx", __func__, sc);
    return sc;
  }
  if (!dult_ctx->motion_ctx.is_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (NULL == dult_ctx->callback.on_motion_detection_event ||
      NULL == dult_ctx->callback.on_non_owner_find_event) {
    DULT_LOG_ERROR("%s: Callback not registered", __func__);
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (dult_ctx->motion_ctx.is_enabled == set) {
    return SL_STATUS_INVALID_STATE;
  }
  dult_ctx->motion_ctx.is_enabled = set;

  if (set) {
    scheduler_rotation_timer(dult_ctx);
  } else {
    state_reset(dult_ctx);
  }

  return SL_STATUS_OK;
}

/*******************************************************************************
 **********************   GLOBAL INTERNAL FUNCTIONS   **************************
 ******************************************************************************/

void sli_dult_motion_detection_init(sli_dult_ctx_t *dult_ctx)
{
  sli_dult_adaptation_timer_create(&dult_ctx->motion_ctx.timer.motion_enable,
                                   true,
                                   timer_motion_enable_callback);
  sli_dult_adaptation_timer_create(&dult_ctx->motion_ctx.timer.poll,
                                   false,
                                   timer_poll_callback);
  sli_dult_adaptation_timer_create(
      &dult_ctx->motion_ctx.timer.motion_poll_duration,
      true,
      timer_motion_poll_duration_callback);
  dult_ctx->motion_ctx.is_enabled     = false;
  dult_ctx->motion_ctx.state          = MOTION_POLL_STATE_STOPPED;
  dult_ctx->motion_ctx.is_initialized = true;
}

/**
 * @brief Reset motion detection context to the state after initialization
 * @param[in] dult_ctx The pointer to DULT context
 * @return sl_status_t SL_STATUS_OK in case of success
 */
sl_status_t sli_dult_motion_detection_reset(sli_dult_ctx_t *dult_ctx){
  state_reset(dult_ctx);
  dult_ctx->motion_ctx.is_enabled     = false;
  return SL_STATUS_OK;
}

void sli_dult_motion_near_owner_changed_notify(sli_dult_ctx_t *dult_ctx)
{
  if (!dult_ctx->motion_ctx.is_enabled) {
    return;
  }

  scheduler_rotation_timer(dult_ctx);
}

void sli_dult_motion_detection_sound_notify(sli_dult_ctx_t *dult_ctx,
                                            bool started)
{
  sl_status_t sc;
  sli_dult_timer_t *poll_duration_timer =
      &dult_ctx->motion_ctx.timer.motion_poll_duration;
  sli_dult_timer_t *poll_timer = &dult_ctx->motion_ctx.timer.poll;

  if (!dult_ctx->motion_ctx.is_enabled) {
    return;
  }
  if (started) {
    if (dult_ctx->motion_ctx.state != MOTION_POLL_STATE_PASSIVE &&
        dult_ctx->motion_ctx.state != MOTION_POLL_STATE_ACTIVE) {
      return;
    }
    // Stop periodic polling timer
    sc = sli_dult_adaptation_timer_stop(poll_timer);
    SLI_DULT_ASSERT_ST(sc);
    if (dult_ctx->motion_ctx.state == MOTION_POLL_STATE_PASSIVE) {
      dult_ctx->motion_ctx.state = MOTION_POLL_STATE_PASSIVE_SOUND_PLAYING;
    } else {
      dult_ctx->motion_ctx.state = MOTION_POLL_STATE_ACTIVE_SOUND_PLAYING;
    }
  } else {
    if (dult_ctx->motion_ctx.state != MOTION_POLL_STATE_PASSIVE_SOUND_PLAYING &&
        dult_ctx->motion_ctx.state != MOTION_POLL_STATE_ACTIVE_SOUND_PLAYING) {
      return;
    }
    dult_ctx->motion_ctx.sound_count++;
    if (dult_ctx->motion_ctx.sound_count >= MAX_SOUND_COUNT_BEFORE_BACKOFF) {
      DULT_LOG_DEBUG("Played %d sound, start backoff",
                     MAX_SOUND_COUNT_BEFORE_BACKOFF);
      backoff_setup(dult_ctx);
      return;
    }
    if (dult_ctx->motion_ctx.state == MOTION_POLL_STATE_PASSIVE_SOUND_PLAYING) {
      DULT_LOG_DEBUG("Start %u ms timer for motion poll duration",
                      FAST_SAMPLING_TIMEOUT_MS);
      sc = sli_dult_adaptation_timer_start(poll_duration_timer,
                                           FAST_SAMPLING_TIMEOUT_MS,
                                           dult_ctx);
      SLI_DULT_ASSERT_ST(sc);
    }

    sc = sli_dult_adaptation_timer_start(poll_timer,
                                         SEPARATED_FAST_SAMPLING_MS,
                                         dult_ctx);
    SLI_DULT_ASSERT_ST(sc);
    dult_ctx->motion_ctx.state = MOTION_POLL_STATE_ACTIVE;
  }
}

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

static void state_reset(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t sc;
  sli_dult_timer_t *timer;

  dult_ctx->motion_ctx.state       = MOTION_POLL_STATE_STOPPED;
  dult_ctx->motion_ctx.sound_count = 0;

  timer = &dult_ctx->motion_ctx.timer.motion_enable;
  if (sli_dult_adaptation_is_timer_running(timer)) {
    sc = sli_dult_adaptation_timer_stop(timer);
    SLI_DULT_ASSERT_ST(sc);
  }
  timer = &dult_ctx->motion_ctx.timer.poll;
  if (sli_dult_adaptation_is_timer_running(timer)) {
    sc = sli_dult_adaptation_timer_stop(timer);
    SLI_DULT_ASSERT_ST(sc);
  }
  timer = &dult_ctx->motion_ctx.timer.motion_poll_duration;
  if (sli_dult_adaptation_is_timer_running(timer)) {
    sc = sli_dult_adaptation_timer_stop(timer);
    SLI_DULT_ASSERT_ST(sc);
  }
}

static void backoff_setup(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t sc;
  state_reset(dult_ctx);
  sc =
      sli_dult_adaptation_timer_start(&dult_ctx->motion_ctx.timer.motion_enable,
                                      SEPARATED_BACKOFF_MS,
                                      dult_ctx);
  SLI_DULT_ASSERT_ST(sc);
}

static void scheduler_rotation_timer(sli_dult_ctx_t *ctx)
{
  if (!ctx->motion_ctx.is_enabled) {
    return;
  }

  switch (ctx->near_owner_state) {
    case SL_DULT_STATE_SEPARATED:
      separated_mode_transition_handle(ctx);
      break;
    case SL_DULT_STATE_NEAR_OWNER:
      near_owner_mode_transition_handle(ctx);
      break;
    default:
      break;
  }
}

static void separated_mode_transition_handle(sli_dult_ctx_t *dult_ctx)
{
  sl_status_t sc = SL_STATUS_OK;
  uint16_t separated_timeout_seed;
  size_t separated_timeout_seed_len = 0;
  uint32_t separated_timeout_minute;

  sc = sl_bt_system_get_random_data(sizeof(separated_timeout_seed),
                                    sizeof(separated_timeout_seed),
                                    &separated_timeout_seed_len,
                                    (uint8_t *) &separated_timeout_seed);
  SLI_DULT_ASSERT_ST(sc);
  separated_timeout_minute = SEPARATED_TIMEOUT_DIFF_MINUTE;
  separated_timeout_minute *= separated_timeout_seed;
  separated_timeout_minute /= UINT16_MAX;
  separated_timeout_minute += SEPARATED_TIMEOUT_MIN_MINUTE;

  // Start timer wait for separated timeout
  sc =
      sli_dult_adaptation_timer_start(&dult_ctx->motion_ctx.timer.motion_enable,
                                      separated_timeout_minute * 60 * 1000,
                                      dult_ctx);
  SLI_DULT_ASSERT_ST(sc);

  DULT_LOG_DEBUG("Waiting for separated timeout: %lu minutes",
                 separated_timeout_minute);
}

static void near_owner_mode_transition_handle(sli_dult_ctx_t *dult_ctx)
{
  state_reset(dult_ctx);
}

/***************************************************************************//**
 * Timer callbacks
 ******************************************************************************/

static void timer_motion_enable_callback(void *ctx)
{
  sl_status_t sc;
  sli_dult_ctx_t *dult_ctx = (sli_dult_ctx_t *) ctx;

  // Check for NULL context pointer
  if (ctx == NULL) {
    DULT_LOG_ERROR("timer_motion_enable_callback: NULL context pointer");
    return;
  }

  DULT_LOG_DEBUG("Timeout reached, done waiting, start polling");
  timer_poll_callback(ctx);
  sc = sli_dult_adaptation_timer_start(&dult_ctx->motion_ctx.timer.poll,
                                       SEPARATED_SLOW_SAMPLING_MS,
                                       ctx);
  SLI_DULT_ASSERT_ST(sc);
  dult_ctx->motion_ctx.state = MOTION_POLL_STATE_PASSIVE;
}

static void timer_poll_callback(void *ctx)
{
  sli_dult_ctx_t *dult_ctx = (sli_dult_ctx_t *) ctx;
  sl_dult_motion_detection_event_action_t evt_action =
      SL_DULT_MOTION_DETECTION_EVENT_TYPE_GET_MOTION_STATUS;

  // Check for NULL context pointer
  if (ctx == NULL) {
    DULT_LOG_ERROR("timer_poll_callback: NULL context pointer");
    return;
  }

  // Check for NULL motion detection event callback
  if (dult_ctx->callback.on_motion_detection_event == NULL) {
    DULT_LOG_ERROR("timer_poll_callback: NULL motion detection event callback");
    return;
  }

  dult_ctx->callback.on_motion_detection_event(&evt_action,
                                               dult_ctx->callback.context);
}

static void timer_motion_poll_duration_callback(void *ctx)
{
  // Check for NULL context pointer
  if (ctx == NULL) {
    DULT_LOG_ERROR("timer_motion_poll_duration_callback: NULL context pointer");
    return;
  }

  DULT_LOG_DEBUG("Poll duration timeout reached, start backoff");
  backoff_setup(ctx);
}

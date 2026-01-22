/***************************************************************************//**
 * @file  sl_dult_adaptation.c
 * @brief  DULT Platform Adaptation source file.
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
#include "sl_status.h"
#include "app_rta.h"
#include "sli_dult_adaptation.h"
#include "sli_dult.h"

typedef enum {
  QUEUE_ITEM_TYPE_DISPATCH,
  QUEUE_ITEM_TYPE_TIMER
} queue_item_type_t;

typedef struct {
  queue_item_type_t type;
  union {
    sli_dult_timer_t *timer;
    struct {
      sli_dult_adaptation_task_t task;
      uint8_t data[SLI_DULT_ADAPTATION_DATA_BUF_SIZE];
      uint16_t data_size;
    } dispatch;
  } data;
} __attribute__((packed)) queue_item_t;

// -----------------------------------------------------------------------------
// Forward declaration of private functions

static void sli_dult_step(void);
static void on_runtime_error(app_rta_error_t error, sl_status_t result);
static void internal_timer_cb(sl_sleeptimer_timer_handle_t *timer, void *data);

static app_rta_context_t ctx = APP_RTA_INVALID_CONTEXT;

// -----------------------------------------------------------------------------
// Internal functions

sl_status_t sli_dult_adaptation_init(void)
{
  sl_status_t sc;
  app_rta_config_t config = {
    .requirement.runtime = true,
    .requirement.guard   = true,
    .requirement.signal  = false,
    .requirement.queue   = true,
    .step                = sli_dult_step,
    .priority            = APP_RTA_PRIORITY_NORMAL,
    .stack_size          = 2048,
    .error               = on_runtime_error,
    .wait_for_guard      = 10,
    .queue_size          = 10,
    .queue_element_size  = sizeof(queue_item_t)};

  sc = app_rta_create_context(&config, &ctx);
  if (sc != SL_STATUS_OK) {
    DULT_LOG_DEBUG("Failed to create RTA context: %lu", sc);
  }
  return sc;
}

// Step function
static void sli_dult_step(void)
{
  queue_item_t item;
  sl_status_t sc;
  size_t size;
  // Get next queue item and acquire guard
  sc = app_rta_queue_read_and_acquire(ctx, (uint8_t *) &item, &size);
  if (sc == SL_STATUS_OK) {
    if (item.type == QUEUE_ITEM_TYPE_DISPATCH) {
      if (item.data.dispatch.task) {
        item.data.dispatch.task(item.data.dispatch.data,
                                item.data.dispatch.data_size);
      }
    } else if (item.type == QUEUE_ITEM_TYPE_TIMER) {
      if (item.data.timer->callback) {
        item.data.timer->callback(item.data.timer->data);
      }
    }
    // Release guard
    app_rta_release(ctx);
  }
}

static void internal_timer_cb(sl_sleeptimer_timer_handle_t *sleeptimer,
                              void *data)
{
  sl_status_t sc;
  queue_item_t item;
  sli_dult_timer_t *timer = NULL;

  (void) sleeptimer;

  if (data == NULL) {
    return;
  }

  timer = (sli_dult_timer_t *) data;
  item.type = QUEUE_ITEM_TYPE_TIMER;
  item.data.timer = timer;
  sc = app_rta_queue_push(ctx, (uint8_t *) &item, sizeof(queue_item_t));
  SLI_DULT_ASSERT_ST(sc);
}

sl_status_t sli_dult_adaptation_schedule(sli_dult_adaptation_task_t task,
                                         void *data,
                                         uint16_t data_size)
{
  if (task == NULL || data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (data_size > SLI_DULT_ADAPTATION_DATA_BUF_SIZE) {
    return SL_STATUS_WOULD_OVERFLOW;
  }

  queue_item_t item = {
    .type = QUEUE_ITEM_TYPE_DISPATCH,
    .data.dispatch = {
      .task = task,
      .data_size = data_size
    }
  };

  memcpy(item.data.dispatch.data, data, data_size);
  return app_rta_queue_push(ctx, (uint8_t *) &item, sizeof(queue_item_t));
}

static void on_runtime_error(app_rta_error_t error, sl_status_t result)
{
  DULT_LOG_ERROR("Scheduler: Runtime error: %d, result: %lu", error, result);
}

sl_status_t sli_dult_adaptation_timer_create(sli_dult_timer_t *timer,
                                             bool shot,
                                             sli_dult_timer_callback_t callback)
{
  if (timer == NULL || callback == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  timer->shot     = shot;
  timer->callback = callback;
  return SL_STATUS_OK;
}

sl_status_t sli_dult_adaptation_timer_start(sli_dult_timer_t *timer,
                                            uint32_t timeout_ms,
                                            void *data)
{
  sl_status_t sc;

  if (timer == NULL || data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  timer->data = data;
  if (timer->shot) {
    sc = sl_sleeptimer_start_timer_ms(&timer->handle,
                                      timeout_ms,
                                      internal_timer_cb,
                                      timer,
                                      0,
                                      0);
  } else {
    sc = sl_sleeptimer_start_periodic_timer_ms(&timer->handle,
                                               timeout_ms,
                                               internal_timer_cb,
                                               timer,
                                               0,
                                               0);
  }
  return sc;
}

bool sli_dult_adaptation_is_timer_running(sli_dult_timer_t *timer)
{
  bool running;
  sl_status_t sc;

  if (timer == NULL) {
    return false;
  }

  sc = sl_sleeptimer_is_timer_running(&timer->handle, &running);
  SLI_DULT_ASSERT_ST(sc);
  return running;
}

sl_status_t sli_dult_adaptation_timer_stop(sli_dult_timer_t *timer)
{
  if (timer == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  return sl_sleeptimer_stop_timer(&timer->handle);
}

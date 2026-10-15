/***************************************************************************//**
 * @file
 * @brief DULT Platform Adaptation header file.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_DULT_ADAPTATION_H
#define SLI_DULT_ADAPTATION_H

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "sl_sleeptimer.h"

/// Timer callback function type
typedef void (*sli_dult_timer_callback_t)(void *data);

/// Timer structure type
/// Timer structure type
typedef struct {
  sl_sleeptimer_timer_handle_t handle;
  sli_dult_timer_callback_t callback;
  uint8_t *data;
  bool shot;
} sli_dult_timer_t;

#define SLI_DULT_ADAPTATION_DATA_BUF_SIZE 8

/// Dispatch callback function type
typedef void (*sli_dult_adaptation_task_t)(void *data, uint16_t data_size);

sl_status_t sli_dult_adaptation_init(void);

sl_status_t sli_dult_adaptation_schedule(sli_dult_adaptation_task_t task,
                                         void *data,
                                         uint16_t data_size);

sl_status_t
    sli_dult_adaptation_timer_create(sli_dult_timer_t *timer,
                                     bool shot,
                                     sli_dult_timer_callback_t callback);
sl_status_t sli_dult_adaptation_timer_start(sli_dult_timer_t *timer,
                                            uint32_t timeout_ms,
                                            void *data);
bool sli_dult_adaptation_is_timer_running(sli_dult_timer_t *timer);
sl_status_t sli_dult_adaptation_timer_stop(sli_dult_timer_t *timer);

#endif // SLI_DULT_ADAPTATION_H

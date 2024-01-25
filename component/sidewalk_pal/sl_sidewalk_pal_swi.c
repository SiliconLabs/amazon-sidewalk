/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_pal_swi.c
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 * Your use of this software is governed by the terms of
 * Silicon Labs Master Software License Agreement (MSLA)available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 * This software contains Third Party Software licensed by Silicon Labs from
 * Amazon.com Services LLC and its affiliates and is governed by the sections
 * of the MSLA applicable to Third Party Software and the additional terms set
 * forth in amazon_sidewalk_license.txt.
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
 *  claim that you wrote the original software. If you use this software
 *  in a product, an acknowledgment in the product documentation would be
 *  appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *  misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <em_device.h>
#include <em_core.h>
#include <sid_pal_swi_ifc.h>
#include <sid_pal_log_ifc.h>
#include "sl_sidewalk_pal_config.h"
#if (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD)
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#else // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT
#include <stddef.h>
#endif // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#if (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD)
#define SWI_TASK_STACK_SIZE    (2048 / sizeof(configSTACK_DEPTH_TYPE))
#else // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT
#define SWI3_PRIORITY 5
#endif // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
#if defined(SL_SIDEWALK_UNIT_TEST)
extern bool is_init;
extern sid_pal_swi_cb_t swi_callback;
#if (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD)
#pragma message "Unit test enabled"
extern SemaphoreHandle_t trigger;
extern TaskHandle_t task_handle;
#endif // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD
#else
static bool is_init = false;
static sid_pal_swi_cb_t swi_callback = NULL;
#if (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD)
static SemaphoreHandle_t trigger = NULL;
static TaskHandle_t task_handle = NULL;
#endif // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD
#endif

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
#if (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD)
static void swi_thread(void *context)
{
  (void)context;

  while (1) {
    if (xSemaphoreTake(trigger, portMAX_DELAY) == pdTRUE) {
      if (swi_callback != NULL) {
        swi_callback();
      }
    }
  }

  SID_PAL_LOG_ERROR("pal: swi thread fatal err");
  task_handle = NULL;
  vTaskDelete(NULL);
}
#else // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT
void SW3_IRQHandler(void)
{
  if (swi_callback != NULL) {
    swi_callback();
  }
}
#endif // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
sid_error_t sid_pal_swi_init(void)
{
  if (is_init) {
    return SID_ERROR_NONE;
  }

#if (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD)
  trigger = xSemaphoreCreateBinary();
  if (trigger == NULL) {
    return SID_ERROR_OOM;
  }

  BaseType_t status = xTaskCreate(swi_thread, "SWI", SWI_TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &task_handle);
  if (status != pdPASS) {
    return SID_ERROR_OOM;
  }

  SID_PAL_LOG_INFO("pal: swi task init ok");
#else
  NVIC_ClearPendingIRQ(SW3_IRQn);
  NVIC_SetPriority(SW3_IRQn, SWI3_PRIORITY);
  NVIC_EnableIRQ(SW3_IRQn);

  SID_PAL_LOG_INFO("pal: swi interrupt init ok");
#endif // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD

  is_init = true;

  return SID_ERROR_NONE;
}

sid_error_t sid_pal_swi_start(sid_pal_swi_cb_t event_callback)
{
  if (event_callback == NULL) {
    return SID_ERROR_NULL_POINTER;
  }

  sid_error_t err = SID_ERROR_NONE;
  if (is_init == false) {
    // Initialize to maintain api backward compatibility
    err = sid_pal_swi_init();
    if (err != SID_ERROR_NONE) {
      return err;
    }
  }
  swi_callback = event_callback;

#if SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT
  NVIC_ClearPendingIRQ(SW3_IRQn);
  NVIC_EnableIRQ(SW3_IRQn);
#endif // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT

  return SID_ERROR_NONE;
}

sid_error_t sid_pal_swi_stop(void)
{
#if SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT
  NVIC_ClearPendingIRQ(SW3_IRQn);
  NVIC_DisableIRQ(SW3_IRQn);
#endif // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT
  swi_callback = NULL;
  return SID_ERROR_NONE;
}

inline sid_error_t sid_pal_swi_trigger(void)
{
  if (!(is_init && swi_callback)) {
    return SID_ERROR_INVALID_STATE;
  }

#if (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD)
  // note: to avoid giving a semaphore that is already given
  if (uxSemaphoreGetCount(trigger) > 0) {
    return SID_ERROR_NONE;
  }

  BaseType_t semaphore_give_status;

  if (CORE_InIrqContext()) {
    BaseType_t higher_priority_task_woken;
    semaphore_give_status = xSemaphoreGiveFromISR(trigger, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
  } else {
    semaphore_give_status = xSemaphoreGive(trigger);
  }

  if (semaphore_give_status != pdTRUE) {
    SID_PAL_LOG_ERROR("pal: swi semaphore cannot be given: %d", semaphore_give_status);
    return SID_ERROR_NOT_FOUND;
  }
#else // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT
  NVIC_SetPendingIRQ(SW3_IRQn);
#endif // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD

  return SID_ERROR_NONE;
}

sid_error_t sid_pal_swi_deinit(void)
{
  if (!is_init) {
    return SID_ERROR_NONE;
  }

  sid_pal_swi_stop();

#if (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD)
  if (task_handle != NULL) {
    vTaskDelete(task_handle);
  }
  if (trigger != NULL) {
    vSemaphoreDelete(trigger);
  }
#endif // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD

  is_init = false;
  return SID_ERROR_NONE;
}

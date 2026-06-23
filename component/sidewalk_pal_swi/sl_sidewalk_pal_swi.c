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
#include <sl_core.h>
#include <sid_pal_swi_ifc.h>
#include "sl_sidewalk_pal_swi_config.h"

#if defined(SL_SIDEWALK_UNIT_TEST)
  #include "sl_sidewalk_log_pal_mock.h"
#else
  #include "sl_sidewalk_log_pal.h"
#endif

#if defined(SL_SIDEWALK_PAL_SWI_IMPL_METHOD) \
  && (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD)
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#else // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT
  #include <stddef.h>
  #if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    #include "sl_power_manager.h"
  #endif
#endif // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/*
 * Workaround for an issue related to HFXO_RESTORE_LL_ACK_MISS_WORKAROUND.
 * This workaround is only applicable when using an external radio and not using RADIO_DRIVER_RAIL.
 * By default, this workaround is enabled.
 * To disable this workaround, set HFXO_RESTORE_LL_ACK_MISS_WORKAROUND to 0 in the project configuration.
 *
 * Issue: In certain scenarios, after waking up the radio from EM2 and transmitting data, the MCU does not receive the ACK.
 * This issue occurs because the MCU clock is latched on the HFRCO clock frequency (20 MHz) instead of the HFXO frequency.
 *
 * Workaround: To address this issue, the MCU clock frequency is restored to HFXO before processing the SWI ISR.
 * This is achieved by adding an EM1 requirement before processing the SWI ISR and removing the EM1 requirement after processing the SWI ISR.
 */
#ifndef HFXO_RESTORE_LL_ACK_MISS_WORKAROUND
  #if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    #if defined(SL_RADIO_EXTERNAL) || !(defined(RADIO_DRIVER_RAIL) && RADIO_DRIVER_RAIL)
      #define HFXO_RESTORE_LL_ACK_MISS_WORKAROUND 1
    #endif
  #else // POWER_MANAGER
    #error "HFXO_RESTORE_LL_ACK_MISS_WORKAROUND not possible - dependency missing to power manager!"
  #endif
#endif // HFXO_RESTORE_LL_ACK_MISS_WORKAROUND

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
#if defined(SL_SIDEWALK_UNIT_TEST)
extern bool is_init;
extern sid_pal_swi_cb_t swi_callback;
#if defined(SL_SIDEWALK_PAL_SWI_IMPL_METHOD) \
  && (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD)
#pragma message "Unit test enabled"
extern SemaphoreHandle_t trigger;
extern TaskHandle_t task_handle;
#endif // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD
#else
static bool is_init = false;
static sid_pal_swi_cb_t swi_callback = NULL;
#if defined(SL_SIDEWALK_PAL_SWI_IMPL_METHOD) \
  && (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD)
static SemaphoreHandle_t trigger = NULL;
static TaskHandle_t task_handle = NULL;
#endif // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD
#endif // SL_SIDEWALK_UNIT_TEST

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
#if defined(SL_SIDEWALK_PAL_SWI_IMPL_METHOD) \
  && (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD)
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

  SL_SID_LOG_PAL_ERROR("pal swi: thread fatal err");
  task_handle = NULL;
  vTaskDelete(NULL);
}
#else // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT
void SW3_IRQHandler(void)
{
  if (swi_callback != NULL) {
    swi_callback();
  }
#if defined(HFXO_RESTORE_LL_ACK_MISS_WORKAROUND) && (HFXO_RESTORE_LL_ACK_MISS_WORKAROUND == 1)
  sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif
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

#if defined(SL_SIDEWALK_PAL_SWI_IMPL_METHOD) \
  && (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD)
  trigger = xSemaphoreCreateBinary();
  if (trigger == NULL) {
    return SID_ERROR_OOM;
  }

  BaseType_t status = xTaskCreate(swi_thread,
                                  "SWI",
                                  SWI_TASK_STACK_SIZE,
                                  NULL,
                                  configMAX_PRIORITIES - 1,
                                  &task_handle);
  if (status != pdPASS) {
    return SID_ERROR_OOM;
  }

  SL_SID_LOG_PAL_INFO("pal swi: task init ok");
#else
  NVIC_ClearPendingIRQ(SW3_IRQn);
  NVIC_SetPriority(SW3_IRQn, SWI3_PRIORITY);
  NVIC_EnableIRQ(SW3_IRQn);

  SL_SID_LOG_PAL_INFO("pal swi: interrupt init ok");
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

#if defined(SL_SIDEWALK_PAL_SWI_IMPL_METHOD) \
  && (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT)
  NVIC_ClearPendingIRQ(SW3_IRQn);
  NVIC_EnableIRQ(SW3_IRQn);
#endif // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT

  return SID_ERROR_NONE;
}

sid_error_t sid_pal_swi_stop(void)
{
#if defined(SL_SIDEWALK_PAL_SWI_IMPL_METHOD) \
  && (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT)
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

#if defined(SL_SIDEWALK_PAL_SWI_IMPL_METHOD) \
  && (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD)
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
    SL_SID_LOG_PAL_ERROR("pal swi: semaphore cannot be given: %d", semaphore_give_status);
    return SID_ERROR_NOT_FOUND;
  }
#else // SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT

#if defined(HFXO_RESTORE_LL_ACK_MISS_WORKAROUND) && (HFXO_RESTORE_LL_ACK_MISS_WORKAROUND == 1)
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif

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

#if defined(SL_SIDEWALK_PAL_SWI_IMPL_METHOD) \
  && (SL_SIDEWALK_PAL_SWI_IMPL_METHOD == SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD)
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

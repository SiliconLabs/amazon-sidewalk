/***************************************************************************//**
 * @file
 * @brief swi.c
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
#include <sid_pal_swi_ifc.h>
#include <em_device.h>
#include <em_core.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <sid_pal_log_ifc.h>
#include <sid_pal_assert_ifc.h>

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define SWI_TASK_STACK_SIZE    (2048 / sizeof(configSTACK_DEPTH_TYPE))

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
static SemaphoreHandle_t trigger = NULL;

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static void swi_thread(void *context)
{
  SID_PAL_ASSERT(context != NULL);

  sid_pal_swi_cb_t swi_callback = (sid_pal_swi_cb_t)context;

  while (1) {
    if (xSemaphoreTake(trigger, portMAX_DELAY) == pdTRUE) {
      swi_callback();
    }
  }

  SID_PAL_LOG_ERROR("swi thread fatal error");
  vTaskDelete(NULL);
}
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
sid_error_t sid_pal_swi_init(sid_pal_swi_cb_t event_callback)
{
  if (event_callback == NULL) {
    return SID_ERROR_NULL_POINTER;
  }

  trigger = xSemaphoreCreateBinary();
  if (trigger == NULL) {
    return SID_ERROR_OOM;
  }

  BaseType_t status = xTaskCreate(swi_thread, "SWI", SWI_TASK_STACK_SIZE, (void *)event_callback, configMAX_PRIORITIES - 1, NULL);
  if (status != pdPASS) {
    return SID_ERROR_OOM;
  }

  SID_PAL_LOG_INFO("swi thread created");

  return SID_ERROR_NONE;
}

inline sid_error_t sid_pal_swi_trigger(void)
{
  SID_PAL_ASSERT(trigger != NULL);

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
    SID_PAL_LOG_ERROR("swi semaphore cannot be given: %d", semaphore_give_status);
    return SID_ERROR_NOT_FOUND;
  }

  return SID_ERROR_NONE;
}

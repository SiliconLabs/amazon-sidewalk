/***************************************************************************//**
 * @file
 * @brief delay.c
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
#include "FreeRTOS.h"
#include "task.h"
#include "delay.h"
#include <sid_pal_delay_ifc.h>
#include <em_cmu.h>

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
static volatile uint32_t core_freq_coef = 0;
static volatile uint32_t core_freq_offset = 0;
static volatile uint32_t total_cycles = 0;
static volatile uint32_t cycles_start_cnt = 0;

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static uint32_t get_core_freq_offset(uint32_t core_freq_coef)
{
  return (uint32_t)((0.81 * core_freq_coef) + 7.24);
}

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void silabs_delay_init(void)
{
  core_freq_coef = CMU_ClockFreqGet(cmuClock_CORE) / 1000000;
  core_freq_offset = get_core_freq_offset(core_freq_coef);
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  ITM->LAR          = 0xc5acce55;
  DWT->CTRL        |= DWT_CTRL_CYCCNTENA_Msk;
}

void sid_pal_delay_us(uint32_t delay)
{
  total_cycles = (delay * core_freq_coef) - core_freq_offset;
  cycles_start_cnt = DWT->CYCCNT;
  while ((DWT->CYCCNT - cycles_start_cnt) < total_cycles) ;
}

/*
 * @details Assuming the implementation is based on FreeRTOS
 *  this will delay the current task through vTaskDelay(),
 *  given the scheduler the ability to run other tasks while
 *  waiting on the delay to expire.
 */
void sid_pal_scheduler_delay_ms(uint32_t delay)
{
  TickType_t ticks = pdMS_TO_TICKS(delay);
  // Minimum of 1 ticks delay
  vTaskDelay(ticks ? ticks : 1);
}

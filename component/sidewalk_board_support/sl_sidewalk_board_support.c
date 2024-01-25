/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_board_support.c
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
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#if (defined(SL_TEMPERATURE_SENSOR_EXTERNAL) || defined(SL_TEMPERATURE_SENSOR_INTERNAL))
#include "FreeRTOS.h"
#include "timers.h"
#endif

#include "em_gpio.h"

#if defined(SL_TEMPERATURE_SENSOR_EXTERNAL)
#include "sl_si70xx.h"
#include "sl_i2cspm_instances.h"
#include "sl_board_control_config.h"
#endif

#if defined(SL_TEMPERATURE_SENSOR_INTERNAL)
#include "tempdrv.h"
#endif

#if defined(SL_SEGMENT_LCD)
#include "sl_segmentlcd.h"
#include "em_cmu.h"
#endif

#include "sl_sidewalk_board_support.h"
#include "app_assert.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
/*******************************************************************************
 * Timer callback to periodically measuring and reporting the temperature and relative humidity.
 * It measures the ambient temperature if sensor is available on the current target.
 *
 * @param pxTimer
 ******************************************************************************/
static void HN_temperature_timer_cb(TimerHandle_t pxTimer);
#endif

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
static TimerHandle_t temperature_measure_timer_hnd = NULL;
#endif

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sl_sidewalk_board_support_init(void)
{
#if defined(SL_TEMPERATURE_SENSOR_EXTERNAL)
#if defined(SL_BOARD_ENABLE_SENSOR_RHT)
  GPIO_PinModeSet(SL_BOARD_ENABLE_SENSOR_RHT_PORT, SL_BOARD_ENABLE_SENSOR_RHT_PIN, gpioModePushPull, 0);
  GPIO_PinOutSet(SL_BOARD_ENABLE_SENSOR_RHT_PORT, SL_BOARD_ENABLE_SENSOR_RHT_PIN);
#endif
  sl_si70xx_init(sl_i2cspm_sensor, SI7021_ADDR);
#endif

#if defined(SL_SEGMENT_LCD)
  // Configure LCD to use step down mode and disable unused segments
  // Default display value 0
  sl_segment_lcd_init(false);
  LCD->BIASCTRL_SET = LCD_BIASCTRL_VDDXSEL_AVDD;
#if defined(SL_SEGMENT_LCD_MODULE_CE322_1002)
  // Display all 0's upon initialization
  sl_segment_lcd_number(0);
#elif defined(SL_SEGMENT_LCD_MODULE_CL010_1087)
  // Example only used upper numeric segments; disable unused segments
  SL_LCD_SEGMENTS_NUM_DIS();
  // Display 0 degC upon initialization
  sl_segment_lcd_lower_number(0);
  sl_segment_lcd_symbol(SL_LCD_SYMBOL_DEGC, 1);  // Display Degree C symbol
  sl_segment_lcd_symbol(SL_LCD_SYMBOL_DP5, 1);   // Display decimal symbol
#endif
#endif
}

void sl_sidewalk_start_temperature_timer(void)
{
#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
  temperature_measure_timer_hnd = xTimerCreate("temperature_timer",
                                               pdMS_TO_TICKS(1000),
                                               pdTRUE,
                                               (void*)0,
                                               HN_temperature_timer_cb);

  app_assert(temperature_measure_timer_hnd != NULL, "Timer creation failed");

  /* Please note that timer won't start until scheduler itself is not started,
     so this function shall be called from main_thread */
  xTimerStart(temperature_measure_timer_hnd, 0);
#endif
}

// -----------------------------------------------------------------------------
// Static Function Definitions
// -----------------------------------------------------------------------------

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
static void HN_temperature_timer_cb(TimerHandle_t pxTimer)
{
  (void) pxTimer;
  int32_t temp_data;
  static bool isPrevTempData = false;
  char number_buffer[8];
  memset(number_buffer, 0, sizeof(number_buffer));

#if defined(SL_TEMPERATURE_SENSOR_EXTERNAL)
  uint32_t rh_data;
  // Measure the values for relative humidity and temperature
  sl_si70xx_measure_rh_and_temp(sl_i2cspm_sensor,
                                SI7021_ADDR,
                                &rh_data,
                                &temp_data);
  sprintf(number_buffer, "%.2f", (float)((float)temp_data / 1000.0));
#elif defined(SL_TEMPERATURE_SENSOR_INTERNAL)
  temp_data = (int32_t)TEMPDRV_GetTemp();
  sprintf(number_buffer, "%d", temp_data);
#endif

#if defined(SL_SIDEWALK_SENSOR)
  sl_sidewalk_sensor_report(SL_SIDEWALK_SENSOR_TYPE_TEMPERATURE, number_buffer, SL_SIDEWALK_SENDER_TYPE_PRIORITY_LOW);
#endif

#if defined(SL_SEGMENT_LCD)
  if (!isPrevTempData) {
    sl_segment_lcd_temp_display(temp_data);
    isPrevTempData = true;
  } else {
    sl_segment_lcd_number(rh_data);
    sl_segment_lcd_symbol(SL_LCD_SYMBOL_P2, 1);
    isPrevTempData = false;
  }
#endif
}
#endif

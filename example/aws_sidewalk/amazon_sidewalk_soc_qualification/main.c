/*
 * Copyright 2020-2023 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.  This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <sl_system_init.h>
#include <sl_mbedtls.h>
#include "sl_system_kernel.h"

#include <app_assert.h>
#include <app_log.h>

#if defined(SL_BLE_SUPPORTED)
#include "ble_adapter.h"
#include "app_ble_config.h"
#endif

#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
#include "app_subghz_config.h"
#endif

#include <sid_api.h>
#include <sid_pal_common_ifc.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <portable.h>

#include <sid_asd_cli.h>
#include <sid_config_cli.h>
#include <sid_qa.h>
#include <mfg_store_app_values.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#if BOOTLOADER_SUPPORT
#include "app_properties.c"
#endif

#define MAIN_TASK_STACK_SIZE    (2048 / sizeof(configSTACK_DEPTH_TYPE))

typedef struct app_context{
  TaskHandle_t main_task;
} app_context_t;

static app_context_t app_context = {
  .main_task = NULL,
};

static void pwr_meas_mode_enter()
{
  // Opportunity to disable peripherals such as flashing LEDS and disable logging to accurately measure power
}

static void pwr_meas_mode_exit()
{
  // Opportunity to re-enable peripherals, LEDs and re-enable logging at the end of the measure power mode
}

static void pwr_meas_is_blocked(void)
{
  // sid_qa processing is blocked and requires the owning RTOS task to yield
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

static void pwr_meas_is_unblocked(void)
{
  // sid_qa processing is unblocked therefore the owning RTOS task can be resumed
  if (xPortIsInsideInterrupt()) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(app_context.main_task, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  } else {
    xTaskNotifyGive(app_context.main_task);
  }
}

static void reboot_func(void)
{
  NVIC_SystemReset();
}

static const sid_pal_mfg_store_region_t mfg_config = {
  .app_value_to_offset = 0,
};

static sid_pal_mfg_store_region_t sid_mfg_config_get(void)
{
  return mfg_config;
}

static void set_sub_ghz_cfg(const struct sid_sub_ghz_links_config * const sub_ghz_cfg)
{
  if (!sub_ghz_cfg) {
    app_log_error("Null pointer passed while setting sub ghz cfg");
  }
  else {
    struct sid_sub_ghz_links_config *cfg = app_get_sub_ghz_config();
    memcpy(cfg, sub_ghz_cfg, sizeof(*cfg));
  }
}

static void main_thread(void * context)
{
  (void)context;

  struct sid_config config = {
    .dev_ch = {
      .type = SID_END_DEVICE_TYPE_STATIC,
      .power_type = SID_END_DEVICE_POWERED_BY_LINE_POWER_ONLY,
      .qualification_id = 0x0002,
    },
    .callbacks = NULL,
#if defined(SL_BLE_SUPPORTED)
    .link_config = app_get_ble_config(),
#else
    .link_config = NULL,
#endif
    .sub_ghz_link_config = app_get_sub_ghz_config(),
  };


  platform_parameters_t platform_parameters = {
    .mfg_store_region =  sid_mfg_config_get(),
#if SL_CSS_SUPPORTED || SL_FSK_SUPPORTED
#if !(defined(RADIO_DRIVER_RAIL) && RADIO_DRIVER_RAIL)
    .platform_init_parameters.radio_cfg = get_radio_cfg(),
#else
    .platform_init_parameters.radio_cfg = get_radio_cfg(),
#endif
#endif
  };

    sid_error_t ret_code = sid_platform_init(&platform_parameters);
    if (ret_code != SID_ERROR_NONE) {
        app_log_error("Sidewalk Platform Init err: %d", ret_code);
    }
    app_assert(ret_code == SID_ERROR_NONE, "Sidewalk platform init failed");

  sid_cli_init();
  sid_config_cli_init();
  struct sid_qa_callbacks qa_callbacks = {
    .reboot_cmd = &reboot_func,
    .set_sub_ghz_cfg = &set_sub_ghz_cfg,
  };
  sid_qa_init(&qa_callbacks);

  struct sid_qa_pwr_meas_if pwr_meas_if = {
    .enter_func = &pwr_meas_mode_enter,
    .exit_func = &pwr_meas_mode_exit,
    .blocked_func = &pwr_meas_is_blocked,
    .unblocked_func = &pwr_meas_is_unblocked,
  };

  sid_qa_set_config(&config, &pwr_meas_if);

  app_log_info("SID QA cli application started...");
  while (1) {
    vTaskDelay(10);
    sid_cli_process();
    sid_qa_process(QA_PROC_NO_WAIT);
  }
}

int main(void)
{
  sl_system_init();

  app_log_info("Sidewalk DUT");

  if (pdPASS != xTaskCreate(main_thread, "MAIN", MAIN_TASK_STACK_SIZE, &app_context, 1, &app_context.main_task)) {
    app_log_error("failed to create main task");
  }

  sl_system_kernel_start();

  for (;;) {
    // Protective endless loop, this point should never be reached
  }
}

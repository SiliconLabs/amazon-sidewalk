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

#include <sl_mbedtls.h>
#include <sl_main_init.h>

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
#include <sid_device_information.h>
#include <mfg_store_app_values.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#if BOOTLOADER_SUPPORT
#include "app_properties.c"
#endif

#define MAIN_TASK_STACK_SIZE    (5120 / sizeof(configSTACK_DEPTH_TYPE))
#define DEVICE_INFO_ERROR 1
#define DEVICE_INFO_SUCCESS 0

#define SERIAL_NUM_SIZE 16
#define MAC_ADDRESS_SIZE 6
#define PRODUCT_FW_VERSION_SIZE 8
#define DEVICE_KIND_SIZE 22

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
  .app_value_to_offset = 0xffffffff,
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
#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
    struct sid_sub_ghz_links_config *cfg = app_get_sub_ghz_config();
    memcpy(cfg, sub_ghz_cfg, sizeof(*cfg));
#endif
  }
// dummy values for reference only, encoded in ascii hex
// dsn = GP13S4005095V3W2
uint8_t dsn[SERIAL_NUM_SIZE] = {0x47, 0x50, 0x31, 0x33, 0x53, 0x34, 0x30, 0x30,
                                0x35, 0x30, 0x39, 0x35, 0x56, 0x33, 0x57, 0x32};
uint8_t mac[MAC_ADDRESS_SIZE] = {0x9C, 0xC8, 0xE9, 0x95, 0xCC, 0x10};
// fw_ver = 1.85.0-4
uint8_t fw_ver[PRODUCT_FW_VERSION_SIZE] = {0x31, 0x2e, 0x38, 0x35, 0x2e, 0x30, 0x2d, 0x34};
// dev kind = reference_board_silabs
uint8_t dev_kind[DEVICE_KIND_SIZE] = {0x72, 0x65, 0x66, 0x65, 0x72, 0x65, 0x6E, 0x63, 0x65, 0x5F, 0x62,
                                      0x6F, 0x61, 0x72, 0x64, 0x5F, 0x73, 0x69, 0x6C, 0x61, 0x62, 0x73};

static struct sid_device_info dev_info = {
    .serial_number_size = SERIAL_NUM_SIZE,
    .mac_address_size = MAC_ADDRESS_SIZE,
    .product_fw_version_size = PRODUCT_FW_VERSION_SIZE,
    .device_kind_size = DEVICE_KIND_SIZE,
    .serial_number = dsn,
    .mac_address = mac,
    .product_fw_version = fw_ver,
    .device_kind = dev_kind,
};

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
#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
    .sub_ghz_link_config = app_get_sub_ghz_config(),
#endif
  };


  platform_parameters_t platform_parameters = {
    .mfg_store_region =  sid_mfg_config_get(),
#if (SL_CSS_SUPPORTED || SL_FSK_SUPPORTED)
    .platform_init_parameters.radio_cfg = get_radio_cfg(),
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
    .device_info_cfg = &dev_info,
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
  sid_platform_deinit();
  vTaskDelete(NULL);
}

void app_init(void)
{
  app_log_info("Sidewalk Qualification");

  if (pdPASS != xTaskCreate(main_thread, "MAIN", MAIN_TASK_STACK_SIZE, &app_context, 1, &app_context.main_task)) {
    app_log_error("failed to create main task");
  }
}

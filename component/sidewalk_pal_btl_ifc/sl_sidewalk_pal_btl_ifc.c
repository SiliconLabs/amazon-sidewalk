/***************************************************************************//**
 * @file sl_sidewalk_pal_btl_ifc.c
 * @brief sidewalk bootloader interface pal component
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <string.h>
#include "sid_pal_uptime_ifc.h"
#include "sid_pal_timer_ifc.h"
#include "sid_time_ops.h"
#include "sl_sidewalk_log_pal.h"
#include "sid_pal_assert_ifc.h"
#include "sl_sidewalk_pal_btl_ifc.h"
#include "btl_interface.h"
#include "btl_interface_storage.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define REBOOT_RESET_TIMER_VALUE (60) // secs

#define READ_BLOCK_SIZE (256) // not related to flash block/page size, etc.

struct btl_ifc_ctx {
  bool init;
  BootloaderStorageSlot_t slot_info;
  sid_pal_timer_t reboot_timer;
};

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

static void reboot_timer_cb(void *arg, sid_pal_timer_t *src);
static void trigger_reset_timeout(void);
#if defined(SL_SIDEWALK_ERASE_FLASH_ON_INIT) && SL_SIDEWALK_ERASE_FLASH_ON_INIT
static void erase_storage_slot_if_needed(void);
#endif
#ifndef SL_SIDEWALK_ERASE_FLASH_ON_INIT
static bool erase_page_if_needed(uint32_t flash_addr, uint32_t offset);
#endif

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

extern char linker_nvm_begin;
static const uint32_t nvm3_start_addr = (uint32_t)&linker_nvm_begin;

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static struct btl_ifc_ctx ctx = { 0 };

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static void reboot_timer_cb(void *arg, sid_pal_timer_t *src)
{
  (void)arg;
  (void)src;

  SL_SID_LOG_PAL_INFO("pal bootloader: rebooting");
  bootloader_rebootAndInstall();
}

static void trigger_reset_timeout(void)
{
  struct sid_timespec then = {};
  struct sid_timespec sid_tsec = {
    .tv_sec = REBOOT_RESET_TIMER_VALUE,
    .tv_nsec = 0,
  };

  if (sid_pal_timer_is_armed(&ctx.reboot_timer)) {
    sid_pal_timer_cancel(&ctx.reboot_timer);
  }
  sid_pal_uptime_now(&then);
  sid_time_add(&then, &sid_tsec);
  sid_pal_timer_arm(&ctx.reboot_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &then, NULL);
}

#if defined(SL_SIDEWALK_ERASE_FLASH_ON_INIT) && SL_SIDEWALK_ERASE_FLASH_ON_INIT
static void erase_storage_slot_if_needed(void)
{
  uint32_t offset = 0;
  uint8_t buffer[READ_BLOCK_SIZE];
  uint8_t buffer_withFF[READ_BLOCK_SIZE];
  bool not_empty = false;
  int32_t ret = BOOTLOADER_OK;
  uint32_t num_blocks = ctx.slot_info.length / READ_BLOCK_SIZE;

  memset(buffer, 0, sizeof(buffer));
  memset(buffer_withFF, 0xff, sizeof(buffer_withFF));

  while (!not_empty && (offset < READ_BLOCK_SIZE * num_blocks) && (ret == BOOTLOADER_OK)) {
    ret = bootloader_readStorage(0, offset, buffer, READ_BLOCK_SIZE);
    if (ret == BOOTLOADER_OK) {
      if (memcmp(buffer, buffer_withFF, sizeof(buffer))) {
        not_empty = true;
      }
      offset += READ_BLOCK_SIZE;
    }
  }

  if (ret != BOOTLOADER_OK) {
    SL_SID_LOG_PAL_ERROR("pal bootloader: bootloader_readStorage failed, addr: 0x%x, err: 0x%x",
                         (ctx.slot_info.address + offset), ret);
    return;
  } else if (not_empty) {
    SL_SID_LOG_PAL_WARNING("pal bootloader: bootloader slot 0 not empty");
    ret = bootloader_eraseRawStorage(ctx.slot_info.address, ctx.slot_info.length);
    if (ret != BOOTLOADER_OK) {
      SL_SID_LOG_PAL_ERROR("pal bootloader: bootloader_eraseRawStorage failed, start_addr: 0x%x, size: 0x%x, err: 0x%x",
                           ctx.slot_info.address,
                           ctx.slot_info.length,
                           ret);
      return;
    }
    SL_SID_LOG_PAL_INFO("pal bootloader: bootloader slot 0 erased, start_addr: 0x%x, size: 0x%x",
                        ctx.slot_info.address,
                        ctx.slot_info.length);
  } else {
    SL_SID_LOG_PAL_INFO("pal bootloader: bootloader slot 0 ready");
  }
}
#endif

#ifndef SL_SIDEWALK_ERASE_FLASH_ON_INIT
// TODO: test this function as physical page size is much greater than READ_BLOCK_SIZE
static bool erase_page_if_needed(uint32_t flash_addr, uint32_t offset)
{
  uint8_t buffer[READ_BLOCK_SIZE];
  uint8_t buffer_withFF[READ_BLOCK_SIZE];
  bool not_empty = false;
  int32_t ret = BOOTLOADER_OK;
  uint32_t num_blocks = FLASH_PAGE_SIZE / READ_BLOCK_SIZE;

  memset(buffer, 0, sizeof(buffer));
  memset(buffer_withFF, 0xff, sizeof(buffer_withFF));

  while (!not_empty && (num_blocks-- != 0) && (ret == BOOTLOADER_OK)) {
    ret = bootloader_readStorage(0, offset, buffer, READ_BLOCK_SIZE);
    if (ret == BOOTLOADER_OK) {
      if (memcmp(buffer, buffer_withFF, sizeof(buffer))) {
        not_empty = true;
      }
      flash_addr += READ_BLOCK_SIZE;
    }
  }

  if (ret != BOOTLOADER_OK) {
    SL_SID_LOG_PAL_ERROR("pal bootloader: bootloader_readStorage failed, addr: 0x%x, err: 0x%x", (ctx.slot_info.address + offset), ret);
    return false;
  } else if (not_empty) {
    SL_SID_LOG_PAL_WARNING("pal bootloader: bootloader slot 0 page not empty, addr: 0x%x", (ctx.slot_info.address + offset));
    ret = bootloader_eraseRawStorage(ctx.slot_info.address + offset, FLASH_PAGE_SIZE);
    if (ret != BOOTLOADER_OK) {
      SL_SID_LOG_PAL_ERROR("pal bootloader: bootloader_eraseRawStorage failed, start_addr: 0x%x, size: 0x%x, err: 0x%x",
                           (ctx.slot_info.address + offset),
                           FLASH_PAGE_SIZE,
                           ret);
      return false;
    }
    SL_SID_LOG_PAL_INFO("pal bootloader: bootloader slot 0 page erased, start_addr: 0x%x, size: 0x%x",
                        (ctx.slot_info.address + offset),
                        FLASH_PAGE_SIZE);
  } else {
    SL_SID_LOG_PAL_INFO("pal bootloader: bootloader slot 0 page ready, addr: 0x%x", (ctx.slot_info.address + offset));
  }
  return true;
}
#endif

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sid_pal_btl_ifc_init(void)
{
  if (ctx.init) {
    SL_SID_LOG_PAL_WARNING("pal bootloader: already initialized");
    return;
  }

  int32_t ret = bootloader_init();
  if (ret != BOOTLOADER_OK) {
    SL_SID_LOG_PAL_ERROR("pal bootloader: bootloader_init failed, err: %x", ret);
    return;
  }

  BootloaderInformation_t bootloader_info;
  bootloader_getInfo(&bootloader_info);
  SL_SID_LOG_PAL_INFO("pal bootloader: gecko bootloader v%lu.%lu",
                      (bootloader_info.version & 0xFF000000) >> 24,
                      (bootloader_info.version & 0x00FF0000) >> 16);

  ret = bootloader_getStorageSlotInfo(0, &ctx.slot_info);
  if (ret != BOOTLOADER_OK) {
    SL_SID_LOG_PAL_ERROR("pal bootloader: bootloader_getStorageSlotInfo failed, err: %x", ret);
    return;
  }
  if (ctx.slot_info.address + ctx.slot_info.length >= nvm3_start_addr) {
    SL_SID_LOG_PAL_ERROR("pal bootloader: bootloader slot 0 overlaps with the nvm3 instance, nvm3_start_addr: 0x%x",
                         nvm3_start_addr);
    return;
  }
  SL_SID_LOG_PAL_INFO("pal bootloader: bootloader slot 0 start_addr: 0x%x, end_addr: 0x%x, size: 0x%x",
                      ctx.slot_info.address,
                      (ctx.slot_info.address + ctx.slot_info.length),
                      ctx.slot_info.length);

#if defined(SL_SIDEWALK_ERASE_FLASH_ON_INIT) && SL_SIDEWALK_ERASE_FLASH_ON_INIT
  erase_storage_slot_if_needed();
#endif

  ctx.init = true;
  sid_pal_timer_init(&ctx.reboot_timer, reboot_timer_cb, NULL);
  SL_SID_LOG_PAL_INFO("pal bootloader: bootloader interface initialized");
}

uint32_t sid_pal_btl_ifc_get_free_space(void)
{
  return ctx.slot_info.length;
}

bool sid_pal_btl_ifc_write(uint32_t offset, void *data, uint32_t data_len)
{
  SID_PAL_ASSERT(data != NULL);

  uint32_t flash_addr = ctx.slot_info.address + offset;

  if ((flash_addr + data_len) >= nvm3_start_addr) {
    SL_SID_LOG_PAL_ERROR("pal bootloader: write addr overlaps with the nvm3 instance, nvm3_start_addr: 0x%x, flash_addr: 0x%x, data_len: 0x%x",
                         nvm3_start_addr,
                         flash_addr,
                         data_len);
    return false;
  }

#ifndef SL_SIDEWALK_ERASE_FLASH_ON_INIT
  if (!erase_page_if_needed(flash_addr, offset)) {
    return false;
  }
#endif

  int32_t ret = bootloader_writeStorage(0, offset, data, data_len);
  if (ret != BOOTLOADER_OK) {
    SL_SID_LOG_PAL_ERROR("pal bootloader: bootloader_writeStorage failed, err: 0x%x, flash_addr: 0x%x, data_len: 0x%x",
                         ret, offset, data_len);
    return false;
  }

  SL_SID_LOG_PAL_INFO("pal bootloader: %d bytes written to flash_addr 0x%x", data_len, flash_addr);
  return true;
}

bool sid_pal_btl_ifc_finalize(uint32_t data_len, uint32_t expected_crc)
{
  int32_t ret = 0;

  uint32_t calculated_crc = sid_pal_btl_ifc_crc32_compute((uint8_t *)ctx.slot_info.address, data_len, NULL);
  if (calculated_crc != expected_crc) {
    SL_SID_LOG_PAL_ERROR("pal bootloader: crc error, calculated_crc: 0x%x, expected_crc: 0x%x", calculated_crc, expected_crc);
    return false;
  }
  SL_SID_LOG_PAL_INFO("pal bootloader: crc check ok, crc: 0x%x, data_len: 0x%x", calculated_crc, data_len);

  ret = bootloader_setImageToBootload(0);
  if (ret != BOOTLOADER_OK) {
    SL_SID_LOG_PAL_ERROR("pal bootloader: bootloader_setImageToBootload failed, err: 0x%x", ret);
    return false;
  }

  SL_SID_LOG_PAL_INFO("pal bootloader: file transfer complete, rebooting in %ld secs", REBOOT_RESET_TIMER_VALUE);
  trigger_reset_timeout();
  return true;
}

void sid_pal_btl_ifc_reboot(void)
{
  SL_SID_LOG_PAL_INFO("pal bootloader: rebooting in %ld secs", REBOOT_RESET_TIMER_VALUE);
  trigger_reset_timeout();
}

uint32_t sid_pal_btl_ifc_crc32_compute(uint8_t const *p_data, uint32_t size, uint32_t const *p_crc)
{
  SID_PAL_ASSERT(p_data != NULL);

  uint32_t crc;
  crc = (p_crc == NULL) ? 0xFFFFFFFF : ~(*p_crc);
  for (uint32_t i = 0; i < size; i++) {
    crc = crc ^ p_data[i];
    for (uint32_t j = 8; j > 0; j--) {
      crc = (crc >> 1) ^ (0xEDB88320U & ((crc & 1) ? 0xFFFFFFFF : 0));
    }
  }
  return ~crc;
}

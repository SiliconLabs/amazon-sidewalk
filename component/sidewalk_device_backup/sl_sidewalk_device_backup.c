/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_device_backup.c
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

#include <stdint.h>
#include <string.h>

#include "app_assert.h"
#include "app_log.h"
#include "sid_pal_mfg_store_ifc.h"
#include "sl_sidewalk_device_backup.h"
#include "sli_sidewalk_device_backup_certificate_common.h"

#if defined(EFR32MG24)
#include "em_msc.h"
#include "em_cmu.h"
#else
#include "sl_se_manager_util.h"
#endif // EFR32MG24

#if defined(SV_ENABLED)
#include "nvm3.h"
#endif // SV_ENABLED

#if defined(SV_ENABLED)
extern nvm3_Handle_t *nvm3_defaultHandle;
#endif // SV_ENABLED

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define TOTAL_MFG_OBJ_CNT (35)

#define OFFSET_MAGIC_NUMBER (0)
#define OFFSET_MFG_OBJ_START (OFFSET_MAGIC_NUMBER + 1)

#define READ_BUF_MAX_SIZE (256)

#define MAGIC_NUMBER (0xCAFEBABE)

#if defined(SV_ENABLED)
#define ITS_OBJ_RANGE_START (0x83100)
#define ITS_OBJ_RANGE_END (0x83105)

#define WRAPPED_KEY_NVM3_KEY_START (ITS_OBJ_RANGE_START)

#define WRAPPED_KEY_CNT (2)
#define WRAPPED_KEY_LEN (128)

#define ITS_REC_CACHE_SIZE (50)
#define ITS_REC_NVM3_REPACK_HEADROOM (0)
#endif // SV_ENABLED

typedef struct {
  uint8_t key;
  uint8_t len;
  const uint8_t *val; // NULL if device specific common otherwise
} mfg_obj_tbl_t;

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

static void erase_user_data(void);
static void write_user_data(uint32_t offset, void *data, uint32_t data_len);
static void read_user_data(uint32_t offset, uint8_t *data, uint32_t data_len);
static bool is_magic_number_present(void);
static bool is_apid_valid(void);
#if defined(SV_ENABLED)
static bool are_wrapped_keys_present(void);
#endif /* SV_ENABLED */
static bool is_backup_needed(void);
static bool is_restore_needed(void);
static bool is_backup_possible(void);
static bool is_restore_possible(void);
static void perform_backup(void);
static void perform_restore(void);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static const mfg_obj_tbl_t MFG_OBJECT_TABLE[] = {
  { .key = SID_PAL_MFG_STORE_SMSN,                          .len = SID_PAL_MFG_STORE_SMSN_SIZE,                           .val = NULL },
  { .key = SID_PAL_MFG_STORE_APP_PUB_ED25519,               .len = SID_PAL_MFG_STORE_APP_PUB_ED25519_SIZE,                .val = APP_PUB_ED25519_VAL },
#if defined(SV_ENABLED)
  { .key = SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519,           .len = SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_SIZE,            .val = DEVICE_PRIV_ED25519 },
#else
  { .key = SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519,           .len = SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_SIZE,            .val = NULL },
#endif /* SV_ENABLED */
  { .key = SID_PAL_MFG_STORE_DEVICE_PUB_ED25519,            .len = SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIZE,             .val = NULL },
  { .key = SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE,  .len = SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE_SIZE,   .val = NULL },
#if defined(SV_ENABLED)
  { .key = SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1,            .len = SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_SIZE,             .val = DEVICE_PRIV_P256R1 },
#else
  { .key = SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1,            .len = SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_SIZE,             .val = NULL },
#endif /* SV_ENABLED */
  { .key = SID_PAL_MFG_STORE_DEVICE_PUB_P256R1,             .len = SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIZE,              .val = NULL },
  { .key = SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE,   .len = SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE_SIZE,    .val = NULL },
  { .key = SID_PAL_MFG_STORE_DAK_PUB_ED25519,               .len = SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIZE,                .val = NULL },
  { .key = SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE,     .len = SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE_SIZE,      .val = NULL },
  { .key = SID_PAL_MFG_STORE_DAK_ED25519_SERIAL,            .len = SID_PAL_MFG_STORE_DAK_ED25519_SERIAL_SIZE,             .val = NULL },
  { .key = SID_PAL_MFG_STORE_DAK_PUB_P256R1,                .len = SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIZE,                 .val = NULL },
  { .key = SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE,      .len = SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE_SIZE,       .val = NULL },
  { .key = SID_PAL_MFG_STORE_DAK_P256R1_SERIAL,             .len = SID_PAL_MFG_STORE_DAK_P256R1_SERIAL_SIZE,              .val = NULL },
  { .key = SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519,           .len = SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIZE,            .val = PRODUCT_PUB_ED25519_VAL },
  { .key = SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE, .len = SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE_SIZE,  .val = PRODUCT_PUB_ED25519_SIGNATURE_VAL },
  { .key = SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL,        .len = SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL_SIZE,         .val = PRODUCT_ED25519_SERIAL_VAL },
  { .key = SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1,            .len = SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIZE,             .val = PRODUCT_PUB_P256R1_VAL },
  { .key = SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE,  .len = SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE_SIZE,   .val = PRODUCT_PUB_P256R1_SIGNATURE_VAL },
  { .key = SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL,         .len = SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL_SIZE,          .val = PRODUCT_P256R1_SERIAL_VAL },
  { .key = SID_PAL_MFG_STORE_MAN_PUB_ED25519,               .len = SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIZE,                .val = MAN_PUB_ED25519_VAL },
  { .key = SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE,     .len = SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE_SIZE,      .val = MAN_PUB_ED25519_SIGNATURE_VAL },
  { .key = SID_PAL_MFG_STORE_MAN_ED25519_SERIAL,            .len = SID_PAL_MFG_STORE_MAN_ED25519_SERIAL_SIZE,             .val = MAN_ED25519_SERIAL_VAL },
  { .key = SID_PAL_MFG_STORE_MAN_PUB_P256R1,                .len = SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIZE,                 .val = MAN_PUB_P256R1_VAL },
  { .key = SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE,      .len = SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE_SIZE,       .val = MAN_PUB_P256R1_SIGNATURE_VAL },
  { .key = SID_PAL_MFG_STORE_MAN_P256R1_SERIAL,             .len = SID_PAL_MFG_STORE_MAN_P256R1_SERIAL_SIZE,              .val = MAN_P256R1_SERIAL_VAL },
  { .key = SID_PAL_MFG_STORE_SW_PUB_ED25519,                .len = SID_PAL_MFG_STORE_SW_PUB_ED25519_SIZE,                 .val = SW_PUB_ED25519_VAL },
  { .key = SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE,      .len = SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE_SIZE,       .val = SW_PUB_ED25519_SIGNATURE_VAL },
  { .key = SID_PAL_MFG_STORE_SW_ED25519_SERIAL,             .len = SID_PAL_MFG_STORE_SW_ED25519_SERIAL_SIZE,              .val = SW_ED25519_SERIAL_VAL },
  { .key = SID_PAL_MFG_STORE_SW_PUB_P256R1,                 .len = SID_PAL_MFG_STORE_SW_PUB_P256R1_SIZE,                  .val = SW_PUB_P256R1_VAL },
  { .key = SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE,       .len = SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE_SIZE,        .val = SW_PUB_P256R1_SIGNATURE_VAL },
  { .key = SID_PAL_MFG_STORE_SW_P256R1_SERIAL,              .len = SID_PAL_MFG_STORE_SW_P256R1_SERIAL_SIZE,               .val = SW_P256R1_SERIAL_VAL },
  { .key = SID_PAL_MFG_STORE_AMZN_PUB_ED25519,              .len = SID_PAL_MFG_STORE_AMZN_PUB_ED25519_SIZE,               .val = AMZN_PUB_ED25519_VAL },
  { .key = SID_PAL_MFG_STORE_AMZN_PUB_P256R1,               .len = SID_PAL_MFG_STORE_AMZN_PUB_P256R1_SIZE,                .val = AMZN_PUB_P256R1_VAL },
  { .key = SID_PAL_MFG_STORE_APID,                          .len = SID_PAL_MFG_STORE_APID_SIZE,                           .val = APID_VAL }
};

#if !defined(EFR32MG24)
static sl_se_command_context_t cmd_ctx;
#endif // !EFR32MG24

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sl_sidewalk_device_backup_handle_backup_restore(void)
{
  if (is_backup_needed()) {
    if (is_backup_possible()) {
      perform_backup();
      app_log_info("device data backed up");
    } else {
      app_assert(false, "backup is not possible, check manufacturing data");
    }
  }
  if (is_restore_needed()) {
    if (is_restore_possible()) {
      perform_restore();
      app_log_info("device data restored");
    } else {
      app_assert(false, "restore is not possible, please contact customer support!");
    }
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief Erases @userdata region of the memory
 *****************************************************************************/
static void erase_user_data(void)
{
#if defined(EFR32MG24)
  CMU_ClockEnable(cmuClock_MSC, true);

  MSC_Status_TypeDef ret = MSC_ErasePage((uint32_t *)USERDATA_BASE);
  app_assert(ret == mscReturnOk, "@userdata erase failed");
#else
  sl_status_t ret = sl_se_erase_user_data(&cmd_ctx);
  app_assert(ret == SL_STATUS_OK, "@userdata erase failed");
#endif
}

/***************************************************************************//**
 * @brief Writes data to the @userdata memory region
 *
 * @param offset Offset from `USERDATA_BASE` address (increments by four)
 * @param data Data to be written to the @userdata region (it shall contain
 * a number of bytes that is divisable by four)
 * @param data_len Number of actual bytes to be written (it must be divisable
 * by four)
 *****************************************************************************/
static void write_user_data(uint32_t offset, void *data, uint32_t data_len)
{
  app_assert(data_len % sizeof(uint32_t) == 0, "@userdata data length is not word aligned");

#if defined(EFR32MG24)
  MSC_Init();

  MSC_Status_TypeDef ret = MSC_WriteWord(((uint32_t *)USERDATA_BASE + offset), data, data_len);
  app_assert(ret == mscReturnOk, "@userdata write failed");

  MSC_Deinit();
#else
  sl_status_t ret = sl_se_write_user_data(&cmd_ctx, (sizeof(uint32_t) * offset), data, data_len);
  app_assert(ret == SL_STATUS_OK, "@userdata write failed");
#endif
}

/***************************************************************************//**
 * @brief Reads data from the @userdata memory region
 *
 * @param offset Offset from `USERDATA_BASE` address (increments by four)
 * @param data Read data buffer
 * @param data_len Number of actual bytes to be read (it must be divisable
 * by four)
 *****************************************************************************/
static void read_user_data(uint32_t offset, uint8_t *data, uint32_t data_len)
{
  app_assert(data_len % sizeof(uint32_t) == 0, "@userdata data length is not word aligned");

  for (uint8_t i = 0; i < data_len; i++) {
    data[i] = *((uint8_t *)USERDATA_BASE + (offset * sizeof(uint32_t)) + i);
  }
}

/***************************************************************************//**
 * @brief Checks if magic number is present or not
 *
 * @note Checks the value in @userdata's magic number location. It has to be
 * written at the end of the backup process. If it's not there, it means that
 * backup did not take place or the backup region (@userdata) is erased. This
 * function can also be used to check if restore is possible or not. If the
 * magic number is not there, it means that there is no backup to be restored.
 * In this case, the device gets useless and the user should contact customer
 * support.
 *
 * @return True if magic number is present false otherwise
 *****************************************************************************/
static bool is_magic_number_present(void)
{
  uint32_t value;

  read_user_data(OFFSET_MAGIC_NUMBER, (uint8_t *)&value, sizeof(value));
  if (value != MAGIC_NUMBER) {
    return false;
  } else {
    return true;
  }
}

/***************************************************************************//**
 * @brief Checks if APID is valid or not
 *
 * @note Checks if the APID value (which is common to all sidewalk device
 * instances) matches with the value that is supposed to be on the manufacturing
 * image. If not, there are three possibilities:
 *    1. there is no manufacturing image flashed
 *    2. wrong manufacturing image flashed
 *    3. wrong common sidewalk device certificate added to the project
 *
 * @note Objects other than APID which are also common to all sidewalk device
 * instances could also have been used for checking. APID is arbitrarily chosen
 * among all common objects.
 *
 * @return True if APID is valid false otherwise
 *****************************************************************************/
static bool is_apid_valid(void)
{
  uint8_t apid_dev[SID_PAL_MFG_STORE_APID_SIZE];

  sid_pal_mfg_store_read(SID_PAL_MFG_STORE_APID, apid_dev, SID_PAL_MFG_STORE_APID_SIZE);
  if (memcmp(apid_dev, APID_VAL, SID_PAL_MFG_STORE_APID_SIZE) == 0) {
    return true;
  } else {
    return false;
  }
}

#if defined(SV_ENABLED)
/***************************************************************************//**
 * @brief Checks if wrapped keys are present or not
 *
 * @note There must be WRAPPED_KEY_CNT wrapped keys in the default NVM3
 * instance.
 *
 * @return True if wrapped keys are present false otherwise
 *****************************************************************************/
static bool are_wrapped_keys_present(void)
{
  Ecode_t st;
  uint32_t obj_type;
  size_t obj_len;
  uint8_t rec_cnt = 0;

  for (uint32_t key = ITS_OBJ_RANGE_START; key < ITS_OBJ_RANGE_END; key++) {
    st = nvm3_getObjectInfo(nvm3_defaultHandle, key, &obj_type, &obj_len);
    if (st == ECODE_NVM3_OK && obj_type == NVM3_OBJECTTYPE_DATA && obj_len == WRAPPED_KEY_LEN) {
      rec_cnt++;
    }
  }

  if (rec_cnt == WRAPPED_KEY_CNT) {
    return true;
  } else {
    return false;
  }
}
#endif /* SV_ENABLED */

/***************************************************************************//**
 * @brief Checks if backup is needed or not
 *
 * @return True if backup is needed false otherwise
 *****************************************************************************/
static bool is_backup_needed(void)
{
  return !is_magic_number_present();
}

/***************************************************************************//**
 * @brief Checks if restore is needed or not
 *
 * @return True if restore is needed false otherwise
 *****************************************************************************/
static bool is_restore_needed(void)
{
#if defined(SV_ENABLED)
  return (!is_apid_valid() || !are_wrapped_keys_present());
#else
  return !is_apid_valid();
#endif /* SV_ENABLED */
}

/***************************************************************************//**
 * @brief Checks if backup is possible or not
 *
 * @return True if backup is possible false otherwise
 *****************************************************************************/
static bool is_backup_possible(void)
{
#if defined(SV_ENABLED)
  return (are_wrapped_keys_present() && is_apid_valid());
#else
  return is_apid_valid();
#endif /* SV_ENABLED */
}

/***************************************************************************//**
 * @brief Checks if restore is possible or not
 *
 * @return True if restore is possible false otherwise
 *****************************************************************************/
static bool is_restore_possible(void)
{
  return is_magic_number_present();
}

/***************************************************************************//**
 * @brief Backups device data to @userdata region of the flash memory
 *
 * @note Magic number is written at the end of the backup process as an indicator
 * that can be checked at boot time to know if the device data has been backed up
 * before or not.
 *
 * @note If device data is backed up once, there is no need to run this function
 * each boot. This can be checked by `is_backup_needed`.
 *****************************************************************************/
static void perform_backup(void)
{
  uint8_t read_buffer[READ_BUF_MAX_SIZE];
  uint32_t offset = OFFSET_MFG_OBJ_START;
  uint32_t magic_number = MAGIC_NUMBER;
#if defined(SV_ENABLED)
  Ecode_t st;
  uint32_t obj_type;
  size_t obj_len;
#endif /* SV_ENABLED */

  erase_user_data();

  for (uint8_t i = 0; i < TOTAL_MFG_OBJ_CNT; i++) {
    if (MFG_OBJECT_TABLE[i].val == NULL) {
      // Backup device specific objects as the static data is hardcoded
      memset(read_buffer, 0, sizeof(read_buffer));
      sid_pal_mfg_store_read((int)MFG_OBJECT_TABLE[i].key, read_buffer, MFG_OBJECT_TABLE[i].len);
      write_user_data(offset, read_buffer, (uint32_t)MFG_OBJECT_TABLE[i].len);
      offset += MFG_OBJECT_TABLE[i].len / sizeof(uint32_t);
    }
  }

#if defined(SV_ENABLED)
  for (uint32_t key = ITS_OBJ_RANGE_START; key < ITS_OBJ_RANGE_END; key++) {
    st = nvm3_getObjectInfo(nvm3_defaultHandle, key, &obj_type, &obj_len);
    if (st == ECODE_NVM3_OK && obj_type == NVM3_OBJECTTYPE_DATA && obj_len == WRAPPED_KEY_LEN) {
      // Backup wrapped keys
      memset(read_buffer, 0, sizeof(read_buffer));
      st = nvm3_readData(nvm3_defaultHandle, key, read_buffer, obj_len);
      app_assert(st == ECODE_NVM3_OK, "default nvm3 object cannot be read");
      write_user_data(offset, read_buffer, obj_len);
      offset += obj_len / sizeof(uint32_t);
    }
  }
#endif /* SV_ENABLED */

  offset = OFFSET_MAGIC_NUMBER;
  write_user_data(offset, (void *)&magic_number, sizeof(magic_number));
}

/***************************************************************************//**
 * @brief Restores device data from @userdata region of the flash memory
 *
 * @note If device data is restored once, there is no need to run this function
 * each boot. This can be checked by `is_restore_needed`.
 *****************************************************************************/
static void perform_restore(void)
{
  uint8_t read_buffer[READ_BUF_MAX_SIZE];
  int32_t ret;
  uint32_t offset = OFFSET_MFG_OBJ_START;
#if defined(SV_ENABLED)
  Ecode_t st;
  uint32_t key;
  uint32_t obj_len;
#endif /* SV_ENABLED */

  for (uint8_t i = 0; i < TOTAL_MFG_OBJ_CNT; i++) {
    if (MFG_OBJECT_TABLE[i].val == NULL) {
      // device specific data
      memset(read_buffer, 0, sizeof(read_buffer));
      read_user_data(offset, read_buffer, MFG_OBJECT_TABLE[i].len);
      ret = sid_pal_mfg_store_write((int)MFG_OBJECT_TABLE[i].key, read_buffer, MFG_OBJECT_TABLE[i].len);
      offset += MFG_OBJECT_TABLE[i].len / sizeof(uint32_t);
    } else {
      // common data
      ret = sid_pal_mfg_store_write((int)MFG_OBJECT_TABLE[i].key, MFG_OBJECT_TABLE[i].val, MFG_OBJECT_TABLE[i].len);
    }
    app_assert(ret == 0, "mfg object cannot be written");
  }

#if defined(SV_ENABLED)
  st = nvm3_eraseAll(nvm3_defaultHandle);
  app_assert(st == ECODE_NVM3_OK, "default nvm3 instance cannot be erased");
  for (uint8_t i = 0; i < WRAPPED_KEY_CNT; i++) {
    // wrapped keys
    memset(read_buffer, 0, sizeof(read_buffer));
    key = WRAPPED_KEY_NVM3_KEY_START + i;
    obj_len = WRAPPED_KEY_LEN;
    read_user_data(offset, read_buffer, obj_len);
    st = nvm3_writeData(nvm3_defaultHandle, key, read_buffer, obj_len);
    app_assert(st == ECODE_NVM3_OK, "default object cannot be written");
    if (nvm3_repackNeeded(nvm3_defaultHandle)) {
      st = nvm3_repack(nvm3_defaultHandle);
      app_assert(st == ECODE_NVM3_OK, "default nvm3 instance repack failed");
    }
    offset += obj_len / sizeof(uint32_t);
  }
#endif /* SV_ENABLED */
}

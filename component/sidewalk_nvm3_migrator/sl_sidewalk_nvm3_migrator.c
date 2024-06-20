/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_nvm3_migrator.c
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "app_assert.h"
#include "app_log.h"
#include "nvm3.h"
#include "nvm3_hal_flash.h"
#include "nvm3_manager.h"
#include "sid_pal_mfg_store_ifc.h"
#include "sl_memory_manager.h"
#include "sl_sidewalk_nvm3_migrator.h"
#include "sl_sidewalk_nvm3_migrator_config.h"

// -----------------------------------------------------------------------------
//                              External Variables
// -----------------------------------------------------------------------------
#if defined(__GNUC__)
// Obtain linker symbol for the end of the nvm3 area
extern char linker_nvm_end;
#else
#error "Unsupported toolchain"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// Convenience macro for the end of the NVM3 area
#define NVM3_END                   (&linker_nvm_end)

// Expected size of the original MFG and KV areas respectievly. No other size is expected based on the design.
#define ORIGNAL_MFG_KV_SIZE         0x6000

// A typical NVM3 cache size, will be used for the temporary NVM3 instances.
#define TEMP_INSTANCE_CACHE_SIZE    30

// Base addresses and sizes of the temporary instances.
// There are A, B and C temporary NVM3 instances to be used during migration:
//  - A: Covers the original KV storage (source of KV objects to be migrated).
//       Also this is the destination of all migrated objects.
//  - B: Covers the original MFG area (source of MFG objects to be migrated).
//  - C: Covers the original DI area (source of DI objects to be migrated).
#define INSTANCE_A_SIZE             ORIGNAL_MFG_KV_SIZE
#define INSTANCE_A_BASE             (NVM3_END - INSTANCE_A_SIZE)
#define INSTANCE_B_SIZE             ORIGNAL_MFG_KV_SIZE
#define INSTANCE_B_BASE             (INSTANCE_A_BASE - INSTANCE_B_SIZE)
#define INSTANCE_C_SIZE             SL_SIDEWALK_NVM3_MIGRATOR_ORIGINAL_DI_SIZE
#define INSTANCE_C_BASE             (INSTANCE_B_BASE - INSTANCE_C_SIZE)

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static nvm3_Handle_t * instance_a = NULL;
static nvm3_Handle_t * instance_b = NULL;
static nvm3_Handle_t * instance_c = NULL;

static nvm3_Init_t instance_a_initdata =
{
  (nvm3_HalPtr_t) INSTANCE_A_BASE,
  INSTANCE_A_SIZE,
  NULL,
  TEMP_INSTANCE_CACHE_SIZE,
  NVM3_MAX_OBJECT_SIZE_DEFAULT,
  0,
  &nvm3_halFlashHandle,
};

static nvm3_Init_t instance_b_initdata =
{
  (nvm3_HalPtr_t) INSTANCE_B_BASE,
  INSTANCE_B_SIZE,
  NULL,
  TEMP_INSTANCE_CACHE_SIZE,
  NVM3_MAX_OBJECT_SIZE_DEFAULT,
  0,
  &nvm3_halFlashHandle,
};

static nvm3_Init_t instance_c_initdata =
{
  (nvm3_HalPtr_t) INSTANCE_C_BASE,
  INSTANCE_C_SIZE,
  NULL,
  TEMP_INSTANCE_CACHE_SIZE,
  NVM3_MAX_OBJECT_SIZE_DEFAULT,
  0,
  &nvm3_halFlashHandle,
};

// Default size for the migration copy buffer which is sufficient for all
// currently known objects. A larger buffer is allocated during runtime is this
// is not large enough.
static uint32_t copy_buffer_size = 256;
// The copy buffer used during migration. Original object data is read to here
// and written to new objects from here.
static uint8_t * copy_buffer = NULL;

// NVM3 version is added with the introduction of the new NVM3 structure.
// For this first migration the 0.x.x.x and 1.0.0.0 values are reserved.
static uint8_t version[SID_PAL_MFG_STORE_SL_NVM3_VERSION_SIZE] = { 0, 0, 0, 0 };

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static bool check_version(uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3);
static void set_version(uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3);
static void ensure_copy_buffer_size(uint32_t obj_len);

static uint32_t prepare_nvm3_instances(void);
static uint32_t open_temp_instance(nvm3_Handle_t ** instance, nvm3_Init_t * initdata);

static void migrate_nvm3_data(void);
static void migrate_objects_per_instance(nvm3_Handle_t * src_instance);
static void get_key_list_from_instance(nvm3_Handle_t * instance, uint32_t * obj_cnt, nvm3_ObjectKey_t ** key_list);

static void cleanup_nvm3_instances(void);
static void close_temp_instance(nvm3_Handle_t ** instance, nvm3_CacheEntry_t ** cache);

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * Utility function to check if the (already read) NVM3 structure version equals
 * a given version number.
 *
 * @param v0 Version byte 0 to be checked (major version)
 * @param v1 Version byte 1 to be checked
 * @param v2 Version byte 2 to be checked
 * @param v3 Version byte 3 to be checked
 * @returns True is the NVM3 version equals the version given in the arguments.
 *****************************************************************************/
static bool check_version(uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3)
{
  return ((version[0] == v0) && (version[1] == v1) && (version[2] == v2) && (version[3] == v3));
}

/**************************************************************************//**
 * Utility function to set the NVM3 structure version to a given version number.
 * Also writes the new version to the DI.
 *
 * @param v0 Byte 0 of the new version (major version)
 * @param v1 Byte 1 of the new version
 * @param v2 Byte 2 of the new version
 * @param v3 Byte 3 of the new version
 *****************************************************************************/
static void set_version(uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3)
{
  version[0] = v0;
  version[1] = v1;
  version[2] = v2;
  version[3] = v3;

  Ecode_t nvm3_res = nvm3_writeData(instance_a,
                                    SLI_SID_NVM3_MAP_KEY(MFG, SID_PAL_MFG_STORE_SL_NVM3_VERSION),
                                    version,
                                    SID_PAL_MFG_STORE_SL_NVM3_VERSION_SIZE);
  app_assert(nvm3_res == ECODE_NVM3_OK, "nvm3_migrator: writing version failed");
}

/**************************************************************************//**
 * Utility function checking that the migration copy buffer is large enough and
 * allocates a larger buffer if needed.
 *
 * @param obj_len Length of the currently copied object, i.e., the currently
 *                required buffer size
 *****************************************************************************/
static void ensure_copy_buffer_size(uint32_t obj_len)
{
  if (obj_len > copy_buffer_size) {
    sl_free(copy_buffer);
    copy_buffer = (uint8_t *) sl_calloc(obj_len, sizeof(uint8_t));
    app_assert(copy_buffer != NULL, "nvm3_migrator: extending copy buffer failed");
    copy_buffer_size = obj_len;
  }
}

/**************************************************************************//**
 * Prepares the NVM3 instances before migration is done. I.e., the DI is closed
 * and the temporary instances are opened.
 *
 * @returns The total number of NVM objects found in the complete area of the
 *          temporary instances. This is normally the number of original NVM3
 *          objects. After a power off during migration this is the total number
 *          of the already moved and the not yet deleted objects.
 *****************************************************************************/
static uint32_t prepare_nvm3_instances(void)
{
  uint32_t total_obj_count = 0;
  Ecode_t nvm3_res;

  // Close DI
  nvm3_res = nvm3_deinitDefault();
  app_assert(nvm3_res == ECODE_NVM3_OK, "nvm3_migrator: the default instance cannot be closed");

  // Open temporary NVM3 instances
  app_log_info("nvm3_migrator: opening instance A (orignal KV)");
  total_obj_count += open_temp_instance(&instance_a, &instance_a_initdata);
  app_log_info("nvm3_migrator: opening instance B (orignal MFG)");
  total_obj_count += open_temp_instance(&instance_b, &instance_b_initdata);
  app_log_info("nvm3_migrator: opening instance C (orignal DI)");
  total_obj_count += open_temp_instance(&instance_c, &instance_c_initdata);

  return total_obj_count;
}

/**************************************************************************//**
 * Opens a temporary NVM3 instance used during migration.
 *
 * @param instance Temporary instance handle.
 * @param initdata Initializer sturcture for the instance.
 * @returns The NVM3 object found in the area of the now opened instance.
 *
 * @note The instance handle and cache are dynamically allocated.
 *****************************************************************************/
static uint32_t open_temp_instance(nvm3_Handle_t ** instance, nvm3_Init_t * initdata)
{
  // Allocate space for the temporary instance
  *instance = (nvm3_Handle_t *) sl_calloc(1, sizeof(nvm3_Handle_t));
  app_assert(*instance != NULL, "nvm3_migrator: temp instance allocation failed");

  // Allocate space for the temporary instance cache
  initdata->cachePtr = (nvm3_CacheEntry_t *) sl_calloc(TEMP_INSTANCE_CACHE_SIZE, sizeof(nvm3_CacheEntry_t));
  app_assert(initdata->cachePtr != NULL, "nvm3_migrator: temp instance cache allocation failed");

  // Open temporary instances
  Ecode_t nvm3_res = nvm3_open(*instance, initdata);
  app_assert(nvm3_res == ECODE_NVM3_OK, "nvm3_migrator: opening temp instance failed");

  uint32_t obj_cnt = (uint32_t)nvm3_enumObjects(*instance, NULL, 0, NVM3_KEY_MIN, NVM3_KEY_MAX);
  app_log_info("nvm3_migrator:     start addr: %x, size: %d, object num: %d",
               (int) initdata->nvmAdr,
               (int) initdata->nvmSize,
               (int) obj_cnt);

  return obj_cnt;
}

/**************************************************************************//**
 * Mid-level function of the migration process. Allocates the default copy
 * buffer, calls and logs the migration parts per temporary instance.
 *****************************************************************************/
static void migrate_nvm3_data(void)
{
  // Allocate copy buffer with default size
  copy_buffer = (uint8_t *) sl_calloc(copy_buffer_size, sizeof(uint8_t));
  app_assert(copy_buffer != NULL, "nvm3_migrator: copy buffer allocation failed");

  // Do the migration for each default instance
  // Do not change order of instances as an underlying function relies on it.
  if (check_version(0, 0, 0, 1)) {
    app_log_info("nvm3_migrator: migrating KV ...");
    migrate_objects_per_instance(instance_a);
    set_version(0, 0, 0, 2);
    app_log_info("nvm3_migrator: ... DONE, version set to 0.0.0.2");
  }

  if (check_version(0, 0, 0, 2)) {
    app_log_info("nvm3_migrator: migrating MFG ...");
    migrate_objects_per_instance(instance_b);
    set_version(0, 0, 0, 3);
    app_log_info("nvm3_migrator: ... DONE, version set to 0.0.0.3");
  }

  if (check_version(0, 0, 0, 3)) {
    app_log_info("nvm3_migrator: migrating DI ...");
    migrate_objects_per_instance(instance_c);
    set_version(1, 0, 0, 0);
    app_log_info("nvm3_migrator: ... DONE, version set to 1.0.0.0");
  }

  // Free the copy buffer
  sl_free(copy_buffer);
  copy_buffer = NULL;
}

/**************************************************************************//**
 * Low-level funciton performing the migration for a given temporary instance.
 *
 * @param src_instance Handle of the source instance from which object are
 *                     migrated (it is a temporary instance for migration).
 *
 * This funciton:
 *  - Iterates over all objects of the source instance. And copies them to the
 *    destination instance, which is always instance_A.
 *  - Checks if the copy buffer is sufficient.
 *  - Performs the necessary key transformations.
 *  - Deletes original objects from NVM3 only after they are successfully
 *    migrated.
 *  - Is resistant to power off. (This algorithm can be simply continued after a
 *    power off at any point.)
 *
 *****************************************************************************/
static void migrate_objects_per_instance(nvm3_Handle_t * src_instance)
{
  uint32_t obj_cnt = 0;
  nvm3_ObjectKey_t *key_list = NULL;

  // Obtain the number of objects in the source instance and the list of keys
  get_key_list_from_instance(src_instance, &obj_cnt, &key_list);

  // Iterate over the objects
  for (uint32_t i = 0; i < obj_cnt; i++) {
    Ecode_t nvm3_res;
    nvm3_ObjectKey_t orig_key = key_list[i];
    nvm3_ObjectKey_t new_key;
    uint32_t obj_type;
    uint32_t obj_len;

    // Get object size
    nvm3_res = nvm3_getObjectInfo(src_instance, orig_key, &obj_type, (size_t*) &obj_len);
    app_assert(nvm3_res == ECODE_NVM3_OK, "nvm3_migrator: getting object info failed");

    // Check if copy buffer is sufficient, extend if needed
    ensure_copy_buffer_size(obj_len);

    // Read object
    nvm3_res = nvm3_readData(src_instance, orig_key, copy_buffer, obj_len);
    app_assert(nvm3_res == ECODE_NVM3_OK, "nvm3_migrator: reading original object failed");

    if (src_instance == instance_a) {
      new_key = SLI_SID_NVM3_MAP_KEY(KV, orig_key);
    } else if (src_instance == instance_b) {
      new_key = SLI_SID_NVM3_MAP_KEY(MFG, orig_key);
    } else {
      new_key = orig_key;
    }

    // Write the object to instance A with its new key
    nvm3_res = nvm3_writeData(instance_a, new_key, copy_buffer, obj_len);
    app_assert(nvm3_res == ECODE_NVM3_OK, "nvm3_migrator: writing new object failed");

    // Delete the original object
    nvm3_res = nvm3_deleteObject(src_instance, orig_key);
    app_assert(nvm3_res == ECODE_NVM3_OK, "nvm3_migrator: deleting original object failed");
  }

  sl_free(key_list);
  key_list = NULL;
}

/**************************************************************************//**
 * Gets the list of NVM3 keys from the given temporary instance, belonging to
 * objects to be migrated.
 *
 * @param instance The temporary instance whose migration is in progress.
 * @param obj_cnt Output argument returning the number of keys.
 * @param key_list Output argument returning the keys.
 *
 * @note This function dynamically allocates the key list.
 * @note After a power off during KV migration instance_A may contain already
 *       migrated and original KV objects mixed. (But no other object as KV
 *       (instance_A) is migrated first). So for this instance a key sub-range
 *       is monitored wihch covers only the original KV keys and no migrated
 *       object key. This is based on the fact that original KV keys are
 *       restricted to 16 bit. For the other instances the whole possible NVM3
 *       key range is checked.
 *****************************************************************************/
static void get_key_list_from_instance(nvm3_Handle_t * instance, uint32_t * obj_cnt, nvm3_ObjectKey_t ** key_list)
{
  nvm3_ObjectKey_t key_min;
  nvm3_ObjectKey_t key_max;

  if (instance == instance_a) {
    // Checking keys using the lower 16 bits only in case of KV (instance_A).
    key_min = 0x0;
    key_max = 0xFFFF;
  } else {
    key_min = NVM3_KEY_MIN;
    key_max = NVM3_KEY_MAX;
  }

  *obj_cnt = nvm3_enumObjects(instance, NULL, 0, key_min, key_max);
  if (*obj_cnt == 0) {
    return;
  }

  *key_list = (nvm3_ObjectKey_t *) sl_calloc(*obj_cnt, sizeof(nvm3_ObjectKey_t));
  app_assert(*key_list != NULL, "nvm3_migrator: key list allocation failed");

  nvm3_enumObjects(instance, *key_list, *obj_cnt, NVM3_KEY_MIN, NVM3_KEY_MAX);
}

/**************************************************************************//**
 * Closes temporary instances used during migration and reopens the actual DI.
 *****************************************************************************/
static void cleanup_nvm3_instances(void)
{
  close_temp_instance(&instance_a, &(instance_a_initdata.cachePtr));
  close_temp_instance(&instance_b, &(instance_b_initdata.cachePtr));
  close_temp_instance(&instance_c, &(instance_c_initdata.cachePtr));
  app_log_info("nvm3_migrator: temp instances closed");

  Ecode_t nvm3_res = nvm3_initDefault();
  app_assert(nvm3_res == ECODE_NVM3_OK, "nvm3_migrator: the default instance cannot be reopened");
}

/**************************************************************************//**
 * Closes a given temporary instance and frees allocated memory.
 *
 * @param instance Handle of the temporary instance to be closed.
 * @param cache Cache of the instance to be freed.
 *****************************************************************************/
static void close_temp_instance(nvm3_Handle_t ** instance, nvm3_CacheEntry_t ** cache)
{
  Ecode_t nvm3_res = nvm3_close(*instance);
  app_assert(nvm3_res == ECODE_NVM3_OK, "nvm3_migrator: closing temp instance failed");

  sl_free(*cache);
  *cache = NULL;

  sl_free(*instance);
  *instance = NULL;
}

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void sl_sidewalk_nvm3_migrator_run(void)
{
  // Ensure Default Instance is opened before checking if migration is needed
  if (nvm3_defaultHandle->hasBeenOpened == false) {
    Ecode_t nvm3_res = nvm3_initDefault();
    app_assert(nvm3_res == ECODE_NVM3_OK, "nvm3_migrator: the default instance cannot be opened");
  }

  // Check NVM3 structure version
  Ecode_t nvm3_res_version = nvm3_readData(nvm3_defaultHandle,
                                           SLI_SID_NVM3_MAP_KEY(MFG, SID_PAL_MFG_STORE_SL_NVM3_VERSION),
                                           version,
                                           SID_PAL_MFG_STORE_SL_NVM3_VERSION_SIZE);
  app_log_info("nvm3_migrator: initial version read (err code: %x, version: %d.%d.%d.%d)",
               (int) nvm3_res_version,
               (int) version[0],
               (int) version[1],
               (int) version[2],
               (int) version[3]);

  // Return as early as possible if the new NVM3 structure version is already in use
  if ((nvm3_res_version == ECODE_NVM3_OK) && (check_version(1, 0, 0, 0))) {
    app_log_info("nvm3_migrator: NVM3 structure is up to date, no migration is needed");
    // Note: NVM3 DI can be left open at this point
    return;
  }

  uint32_t total_obj_count = prepare_nvm3_instances();

  if (total_obj_count != 0) {
    // Perform main functionality
    if (nvm3_res_version == ECODE_NVM3_OK) {
      migrate_nvm3_data();
    } else if (nvm3_res_version == ECODE_NVM3_ERR_KEY_NOT_FOUND) {
      set_version(0, 0, 0, 1);
      migrate_nvm3_data();
    } else {
      app_assert(false, "nvm3_migrator: reading the version failed");
    }
  } else {
#if defined(SL_CATALOG_SIDEWALK_DEVICE_BACKUP_PRESENT)
    cleanup_nvm3_instances();
    app_log_info("nvm3_migrator: no NVM3 object found at all, return and rely on device backup");
    return;
#else
    app_assert(false, "nvm3_migrator: no NVM3 object found at all");
#endif // defined(SL_CATALOG_SIDEWALK_DEVICE_BACKUP_PRESENT)
  }

  cleanup_nvm3_instances();
  app_log_info("nvm3_migrator: migration DONE");
}

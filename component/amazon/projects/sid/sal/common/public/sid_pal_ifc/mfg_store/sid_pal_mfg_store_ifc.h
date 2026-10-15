/*
 * Copyright 2020-2025 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */


#ifndef SID_PAL_MFG_STORE_IFC_H
#define SID_PAL_MFG_STORE_IFC_H

/**
 * \addtogroup sid_ifc
 * @{
 */
/**
 * \addtogroup sid_pal_ifc
 * @{
 */
/**
 * \addtogroup sid_pal_storage_ifc
 * @{
 */
/**
 * \addtogroup sid_pal_mfg_ifc
 * @{
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * @addtogroup sid_pal_mfg_ifc_types Type definitions
 * @ingroup sid_pal_mfg_ifc
 * @{
 *****************************************************************************/

/**
 * @brief The current version of the MFG storage. The version is stored during generating MFG.
 * It can be used to migrate between different versions of MFG when changing its data structure.
 */
#define SID_PAL_MFG_STORE_EMPTY_VERSION_NUMBER 0xFFFFFFFF
#define SID_PAL_MFG_STORE_FIXED_OFFSETS_VERSION 7 /*!< Last version with fixed offsets */
#define SID_PAL_MFG_STORE_TLV_VERSION 8           /*!< This macro defines the version number for the TLV (Type-Length-Value) */

enum { SID_PAL_MFG_STORE_INVALID_OFFSET = UINT32_MAX };

/**
 * @brief Values available to all users of the manufacturing store.
 * 
 */
typedef enum {
    SID_PAL_MFG_STORE_DEVID = 1,                            /*!< use sid_pal_mfg_store_dev_id_get */
    /**
     * @note  Version is stored in network order
     */
    SID_PAL_MFG_STORE_VERSION = 2,                          /*!< use sid_pal_mfg_store_get_version */
    SID_PAL_MFG_STORE_SERIAL_NUM = 3,                       /*!< use sid_pal_mfg_store_dev_id_get */
    SID_PAL_MFG_STORE_SMSN = 4,
    SID_PAL_MFG_STORE_APP_PUB_ED25519 = 5,
    SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519 = 6,
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519 = 7,
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE = 8,
    SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1 = 9,
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1 = 10,
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE = 11,
    SID_PAL_MFG_STORE_DAK_PUB_ED25519 = 12,
    SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE = 13,
    SID_PAL_MFG_STORE_DAK_ED25519_SERIAL = 14,
    SID_PAL_MFG_STORE_DAK_PUB_P256R1 = 15,
    SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE = 16,
    SID_PAL_MFG_STORE_DAK_P256R1_SERIAL = 17,
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519 = 18,
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE = 19,
    SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL = 20,
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1 = 21,
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE = 22,
    SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL = 23,
    SID_PAL_MFG_STORE_MAN_PUB_ED25519 = 24,
    SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE = 25,
    SID_PAL_MFG_STORE_MAN_ED25519_SERIAL = 26,
    SID_PAL_MFG_STORE_MAN_PUB_P256R1 = 27,
    SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE = 28,
    SID_PAL_MFG_STORE_MAN_P256R1_SERIAL = 29,
    SID_PAL_MFG_STORE_SW_PUB_ED25519 = 30,
    SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE = 31,
    SID_PAL_MFG_STORE_SW_ED25519_SERIAL = 32,
    SID_PAL_MFG_STORE_SW_PUB_P256R1 = 33,
    SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE = 34,
    SID_PAL_MFG_STORE_SW_P256R1_SERIAL = 35,
    SID_PAL_MFG_STORE_AMZN_PUB_ED25519 = 36,
    SID_PAL_MFG_STORE_AMZN_PUB_P256R1 = 37,
    SID_PAL_MFG_STORE_APID = 38,
    SID_PAL_MFG_STORE_DTID = 39,

    /**
     * @note This arbitrary value is the number of value identifiers
     * reserved by Sidewalk. The range of these value identifiers is:
     * [0, SID_PAL_MFG_STORE_CORE_VALUE_MAX].
     * Applications may use identifiers outside of that range.
     */
    SID_PAL_MFG_STORE_CORE_VALUE_MAX = 4000,
    /** @note The value 0x6FFF is reserved for internal use */
    SID_PAL_MFG_STORE_VALUE_MAX = 0x6FFE,
} sid_pal_mfg_store_value_t;


/**
 * @brief Value sizes in bytes.
 *
 * This enum defines the sizes of various values stored in the manufacturing store.
 */
typedef enum {
    SID_PAL_MFG_STORE_DEVID_SIZE                        = 5,
    SID_PAL_MFG_STORE_VERSION_SIZE                      = 4,
    SID_PAL_MFG_STORE_SERIAL_NUM_SIZE                   = 17,
    SID_PAL_MFG_STORE_SMSN_SIZE                         = 32,
    SID_PAL_MFG_STORE_APP_PUB_ED25519_SIZE              = 32,
    SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_SIZE          = 32,
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIZE           = 32,
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE_SIZE = 64,
    SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_SIZE           = 32,
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIZE            = 64,
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE_SIZE  = 64,
    SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIZE              = 32,
    SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE_SIZE    = 64,
    SID_PAL_MFG_STORE_DAK_ED25519_SERIAL_SIZE           = 4,
    SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIZE               = 64,
    SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE_SIZE     = 64,
    SID_PAL_MFG_STORE_DAK_P256R1_SERIAL_SIZE            = 4,
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIZE          = 32,
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE_SIZE= 64,
    SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL_SIZE       = 4,
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIZE           = 64,
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE_SIZE = 64,
    SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL_SIZE        = 4,
    SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIZE              = 32,
    SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE_SIZE    = 64,
    SID_PAL_MFG_STORE_MAN_ED25519_SERIAL_SIZE           = 4,
    SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIZE               = 64,
    SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE_SIZE     = 64,
    SID_PAL_MFG_STORE_MAN_P256R1_SERIAL_SIZE            = 4,
    SID_PAL_MFG_STORE_SW_PUB_ED25519_SIZE               = 32,
    SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE_SIZE     = 64,
    SID_PAL_MFG_STORE_SW_ED25519_SERIAL_SIZE            = 4,
    SID_PAL_MFG_STORE_SW_PUB_P256R1_SIZE                = 64,
    SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE_SIZE      = 64,
    SID_PAL_MFG_STORE_SW_P256R1_SERIAL_SIZE             = 4,
    SID_PAL_MFG_STORE_AMZN_PUB_ED25519_SIZE             = 32,
    SID_PAL_MFG_STORE_AMZN_PUB_P256R1_SIZE              = 64,
    SID_PAL_MFG_STORE_APID_SIZE                         = 4,
    SID_PAL_MFG_STORE_DTID_SIZE = 14,
} sid_pal_mfg_store_value_size_t;

/**
 * @def SID_PAL_MFG_STORE_MAX_FLASH_WRITE_LEN
 * @brief Maximum length for flash write operations in the manufacturing store.
 *
 * This macro defines the maximum number of bytes that can be written to the 
 * flash memory in a single write operation within the manufacturing store.
 * 
 * @note This value is set to 64 bytes.
 */
#define SID_PAL_MFG_STORE_MAX_FLASH_WRITE_LEN 64

/* Basic I/O and setup */

/**
 * @typedef sid_pal_mfg_store_app_value_to_offset_t
 * @brief Function pointer type for converting an application-specific value to an offset.
 *
 * @param value The application-specific value to be converted to an offset.
 * @return A 32-bit unsigned integer representing the offset corresponding to the input value.
 */
typedef uint32_t (*sid_pal_mfg_store_app_value_to_offset_t)(int value);

/**
 * @typedef sid_pal_mfg_store_overwrite_read_t
 * @brief Function pointer type for overwriting the default read calls
 *
 * @param value  Enum constant for the desired value. Use values from
 *               sid_pal_mfg_store_value_t or application defined values
 *               here.
 * @param buffer Buffer to which the value will be copied.
 * @param length Length of the value in bytes. Use values from
 *               sid_pal_mfg_store_value_size_t here.
 * @return True if the value is overwritten, otherwise false
 */
typedef bool (*sid_pal_mfg_store_overwrite_read_t)(uint16_t value, uint8_t *buffer, uint16_t length);

/**
 * @brief Type which holds the start and end addresses of the manufacturing store
 */
typedef struct {
    uintptr_t addr_start;   /*!< The start address of the manufacturing store region. */
    uintptr_t addr_end;     /*!< The end address of the manufacturing store region. */
    /**
     * @brief This function allows applications to extend the manufacturing store to be
     * be used for non-Sidewalk values. Applications should provide an
     * implementation of this function if they wish to extend the manufacturing
     * store for their own use. Its responsibility is to convert a value identifier
     * to an offset (in bytes) from the beginning of the manufacturing store.
     * Sidewalk owns the first SID_PAL_MFG_STORE_CORE_VALUE_MAX identifiers, So this
     * function's input should be greater than SID_PAL_MFG_STORE_CORE_VALUE_MAX and
     * its output should a valid offset below the mfg_store end address.
     *  If no mapping from the value identifier to an offset can be found, this
     *  function should return SID_PAL_MFG_STORE_INVALID_OFFSET value, which will
     *  cause the manufacturing store implementation to reject any operation on the
     *  provided value.
     */
    sid_pal_mfg_store_app_value_to_offset_t app_value_to_offset;

    /**
     * @brief This function allows to overwrite read calls and call the callback function
     *  defined by overwrite_read, if the value is overwritten then it returns true
     *  else false
     */
    sid_pal_mfg_store_overwrite_read_t overwrite_read;
} sid_pal_mfg_store_region_t;

/** @} (end sid_pal_mfg_ifc_types) */

/** 
 *  @brief Prepare the manufacturing store for use. Must be called before
 *  any of the other sid_pal_mfg_store functions.
 *
 *  @param[in]  mfg_store_region Structure containing start and end addresses
 *                              of the manufacturing store.
 */
void sid_pal_mfg_store_init(sid_pal_mfg_store_region_t mfg_store_region);

/** 
 *  @brief Deinitialize previously initialized mfg region.
 */
void sid_pal_mfg_store_deinit(void);

/** 
 *  @brief Erase the manufacturing store.
 *  Because the manufacturing store is backed by flash memory, and flash memory
 *  can only be erased in large chunks (pages), this interface only supports
 *  erasing the entire manufacturing store.
 *
 *  @note This function is only supported for diagnostic builds.
 *
 *  @return  0 on success, negative value on failure.
 */
int32_t sid_pal_mfg_store_erase(void);

/** 
 *  @brief Check if the manufacturing store is empty.
 *
 *  @note This function is only supported for diagnostic builds.
 *
 * @retval  true if the entire manufacturing store is empty,
 *          such as just after an erase.
 */
bool sid_pal_mfg_store_is_empty(void);

/** 
 *  @brief Write to mfg store.
 *
 *  @param[in]  value  Enum constant for the desired value. Use values from
 *                     sid_pal_mfg_store_value_t or application defined values
 *                     here.
 *  @param[in]  buffer Buffer containing the value to be stored.
 *  @param[in]  length Length of the value in bytes. Use values from
 *                     sid_pal_mfg_store_value_size_t here.
 *  @retval  0 on success, negative value on failure.
 */
int32_t sid_pal_mfg_store_write(uint16_t value, const uint8_t *buffer, uint16_t length);


/** 
 *  @brief Read from mfg store.
 *
 *  @param[in]  value  Enum constant for the desired value. Use values from
 *                     sid_pal_mfg_store_value_t or application defined values
 *                     here.
 *  @param[out] buffer Buffer to which the value will be copied.
 *  @param[in]  length Length of the value in bytes. Use values from
 *                     sid_pal_mfg_store_value_size_t here.
 */
void sid_pal_mfg_store_read(uint16_t value, uint8_t *buffer, uint16_t length);


/** 
 *  @brief Get length of a tag ID.
 *
 *  @param[in]  value  Enum constant for the desired value. Use values from
 *                     sid_pal_mfg_store_value_t or application defined values
 *                     here.
 *  @retval  Length of the value in bytes for the tag that is requested on success,
 *           0 on failure (not found)
 */
uint16_t sid_pal_mfg_store_get_length_for_value(uint16_t value);


/** 
 *  @brief Check if the manufacturing store supports TLV based storage.
 *
 *  @note This function only indicates that the platform supports TLV,
 *        but the device may have storage with fixed offsets that was
 *        flashed during production.
 *  @retval  true if the manufacturing store supports TLV based storage
 */
bool sid_pal_mfg_store_is_tlv_support(void);


/** @note Functions specific to Sidewalk with special handling */

/** 
 *  @brief Get version of values stored in mfg store.
 *  The version of the mfg values is stored along with all the values
 *  in mfg store. This API retrieves the value by reading the
 *  address at which the version is stored.
 *
 *  @retval   version of mfg store.
 */
uint32_t sid_pal_mfg_store_get_version(void);


/** 
 *  @brief Get the device ID from the mfg store.
 *
 *  @param[out] dev_id The device ID
 *  @retval true if the device ID could be found
 */
bool sid_pal_mfg_store_dev_id_get(uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE]);


/** 
 *  @brief Get the device serial number from the mfg store.
 *
 *  @param[out] serial_num The device serial number
 *  @retval true if the device serial number could be found
 */
bool sid_pal_mfg_store_serial_num_get(uint8_t serial_num[SID_PAL_MFG_STORE_SERIAL_NUM_SIZE]);

/** 
 *  @brief Get the APID.
 *  Applicable only for products with short form certificate chain.
 *
 *  @param[out] apid The apid
 */
void sid_pal_mfg_store_apid_get(uint8_t apid[SID_PAL_MFG_STORE_APID_SIZE]);


/** 
 *  @brief Get the Application public key.
 *  Applicable only for products with short form certificate chain.
 *
 *  @param[out] app_pub The Application public key
 */
void sid_pal_mfg_store_app_pub_key_get(uint8_t app_pub[SID_PAL_MFG_STORE_APP_PUB_ED25519_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* SID_PAL_MFG_STORE_IFC_H */

/** @} */ // end of sid_ifc group
/** @} */ // end of sid_pal_ifc group
/** @} */ // end of sid_pal_storage_ifc group
/** @} */ // end of sid_pal_mfg_ifc group
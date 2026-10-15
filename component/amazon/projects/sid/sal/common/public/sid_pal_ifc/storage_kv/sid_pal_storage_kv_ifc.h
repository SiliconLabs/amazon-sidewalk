/*
 * Copyright 2020-2023 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_PAL_STORAGE_KV_IFC_H
#define SID_PAL_STORAGE_KV_IFC_H

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
 * \addtogroup sid_pal_kv_ifc
 * @{
 */

#include <sid_error.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SID_SDK_CONFIG_ENABLE_METRICS_PERSISTENCE) && SID_SDK_CONFIG_ENABLE_METRICS_PERSISTENCE
/**
 * @def SID_PAL_KV_STORE_MAX_LENGTH_BYTES
 * @brief Maximum length in bytes for the Key-Value store.
 */
#define SID_PAL_KV_STORE_MAX_LENGTH_BYTES 200
#else
/**
 * @def SID_PAL_KV_STORE_MAX_LENGTH_BYTES
 * @brief Maximum length in bytes for the Key-Value store.
 */
#define SID_PAL_KV_STORE_MAX_LENGTH_BYTES 60
#endif

/**
 * @brief Initialize the key value storage subsystem
 *
 * @retval SID_ERROR_NONE in case of success
 */
sid_error_t sid_pal_storage_kv_init(void);

/**
 * @brief Deinitialize the key value storage subsystem
 *
 * @retval SID_ERROR_NONE in any case
 */
sid_error_t sid_pal_storage_kv_deinit(void);


/**
 * @brief Get a value using its group and key IDs
 *
 * @param[in]   group    Group
 * @param[in]   key      Key
 * @param[out]  p_data   Pointer to output buffer to contain the value
 * @param[in]   len      Maximum length of buffer pointed to by p_data in bytes
 *
 * @retval SID_ERROR_NONE in case of success
 */
sid_error_t sid_pal_storage_kv_record_get(uint16_t group, uint16_t key, void * p_data, uint32_t len);


/**
 * @brief Get the size of a value using its group and key IDs
 *
 * @param[in]   group   Group
 * @param[in]   key     Key
 * @param[out]  p_len   Pointer to integer to contain the size of the value in bytes
 *
 * @retval SID_ERROR_NONE in case of success
 */
sid_error_t sid_pal_storage_kv_record_get_len(uint16_t group, uint16_t key, uint32_t * p_len);


/**
 * @brief Set a value using its group and key IDs
 *
 * @param[in]  group    Group
 * @param[in]  key      Key
 * @param[in]  p_data   Pointer to input buffer which contains the value
 * @param[in]  len      The size of the input value in bytes
 *
 * @retval SID_ERROR_NONE in case of success
 */
sid_error_t sid_pal_storage_kv_record_set(uint16_t group, uint16_t key, void const * p_data, uint32_t len);


/**
 * @brief Delete a value using its group and key IDs
 *
 * @param[in]  group    Group
 * @param[in]  key      Key
 *
 * @retval SID_ERROR_NONE in case of success
 */
sid_error_t sid_pal_storage_kv_record_delete(uint16_t group, uint16_t key);


/**
 * @brief Delete all values in a group
 *
 * @param[in]  group    Group
 *
 * @retval SID_ERROR_NONE in case of success
 */
sid_error_t sid_pal_storage_kv_group_delete(uint16_t group);


#ifdef __cplusplus
}
#endif

#endif

/** @} */ // end of sid_ifc group
/** @} */ // end of sid_pal_ifc group
/** @} */ // end of sid_pal_storage_ifc group
/** @} */ // end of sid_pal_kv_ifc group
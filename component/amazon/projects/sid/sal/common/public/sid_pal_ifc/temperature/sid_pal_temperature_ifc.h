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

#ifndef SID_PAL_TEMPERATURE_IFC_H
#define SID_PAL_TEMPERATURE_IFC_H

/**
 * \addtogroup sid_pal_ifc
 * @{
 */
/**
 * \addtogroup sid_pal_peripheral_ifc
 * @{
 */
/**
 * \addtogroup sid_pal_temp_ifc
 * @{
 */
#include <sid_error.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Init temperature detection
 *
 * @retval true if support, else false
 */
sid_error_t sid_pal_temperature_init(void);

/**
 * @brief Get temperature
 *
 * @retval temperature in celsius degree
 */
int16_t sid_pal_temperature_get(void);

#ifdef __cplusplus
}
#endif

#endif /* SID_PAL_TEMPERATURE_IFC_H */

/** @} */ // end of sid_pal_ifc group
/** @} */ // end of sid_pal_peripheral_ifc group
/** @} */ // end of sid_pal_temp_ifc group
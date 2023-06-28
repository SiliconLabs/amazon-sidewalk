/***************************************************************************//**
 * @file
 * @brief sid_pal_serial_bus_efr32_spi_config.h
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

#ifndef SID_PAL_SERIAL_BUS_EFR32_SPI_CONFIG_H
#define SID_PAL_SERIAL_BUS_EFR32_SPI_CONFIG_H

#include <sid_pal_serial_bus_ifc.h>

#include <sid_error.h>

#if defined(EFR32XG21)
    #include "em_usart.h"
    #define USART_INSTANCE_TYPE USART_TypeDef
#elif defined(EFR32XG24) || defined(EFR32XG28)
    #include "em_eusart.h"
    #define USART_INSTANCE_TYPE EUSART_TypeDef
#else
    #error UNSUPPORTED PLATFORM!
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sid_pal_serial_bus_efr32_spi_config {
  USART_INSTANCE_TYPE *peripheral_id;
};

sid_error_t sid_pal_serial_bus_efr32_spi_create(const struct sid_pal_serial_bus_iface **iface, const void *cfg);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_PAL_SERIAL_BUS_EFR32_SPI_CONFIG_H */

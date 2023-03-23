/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef HALO_PLATFORM_SILABS_EFR32_INCLUDE_EXPORT_HALO_PLATFORM_BUS_SERIAL_SPI_CONFIG_H
#define HALO_PLATFORM_SILABS_EFR32_INCLUDE_EXPORT_HALO_PLATFORM_BUS_SERIAL_SPI_CONFIG_H

#include <halo/error.h>

#include <dev/bus/serial/serial.h>

#include <stdint.h>

#if defined(EFR32MG21) || defined(EFR32BG21)
    #include "em_usart.h"
    #define USART_INSTANCE_TYPE USART_TypeDef
#elif defined(EFR32MG24)
    #include "em_eusart.h"
    #define USART_INSTANCE_TYPE EUSART_TypeDef
#else
    #error UNSUPPORTED PLATFORM!
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t instance_id;
    USART_INSTANCE_TYPE *peripheral_id;
} halo_serial_bus_ef32_spi_config_t;

halo_error_t bus_serial_efr32_spi_create(const halo_serial_bus_iface_t **iface, const void *cfg);

#ifdef __cplusplus
}
#endif

#endif /*! HALO_PLATFORM_SILABS_EFR32_INCLUDE_EXPORT_HALO_PLATFORM_BUS_SERIAL_SPI_CONFIG_H */

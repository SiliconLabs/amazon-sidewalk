/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.  This file is a Modifiable
 * File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_SERIAL_BUS_EFR32_SPI_CONFIG_H
#define SID_PAL_SERIAL_BUS_EFR32_SPI_CONFIG_H

#include <sid_pal_serial_bus_ifc.h>

#include <sid_error.h>

#if defined(EFR32MG21) || defined(EFR32BG21)
    #include "em_usart.h"
    #define USART_INSTANCE_TYPE USART_TypeDef
#elif defined(EFR32MG24)
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

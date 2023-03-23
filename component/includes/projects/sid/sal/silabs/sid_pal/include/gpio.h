/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <sid_pal_gpio_ifc.h>
#include "em_gpio.h"

enum SL_PINout {
    SL_PIN_BUSY = 0,
    SL_PIN_ANTSW,
    SL_PIN_DIO,
    SL_PIN_NRESET,
    SL_PIN_NSS,
#ifdef EFR32BG21
    SL_PIN_MAUI_BAND_SEL,
#endif
    SL_PIN_MAX
};

struct GPIO_LookupItem
{
    uint32_t GPIO_Port;
    uint8_t Pin;
    GPIO_Mode_TypeDef mode;
    sid_pal_gpio_irq_handler_t callback;
    struct
    {
        bool falling;
        bool rising;
    }
    irq;
    void * callbackarg;
};
/***************************************************************************//**
 * @file
 * @brief app_gpio_config.c
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
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

/******************************************************************************
   This config only supports Adaptor board Rev A02!
 *******************************************************************************/

#include "include/gpio.h"

#if defined(EFR32MG21) || defined(EFR32BG21)
    #include "sl_spidrv_exp_config.h"
#elif defined(EFR32MG24)
    #include "sl_spidrv_eusart_exp_config.h"
#endif

#if defined(EFR32MG21) || defined(EFR32MG24)
#include "sl_sx126x_exp_config.h"
// These pin configs are overridden with custom made sl_spidrv_exp_config or sl_spidrv_eusart_exp_config header file for semtech board
#elif defined(EFR32BG21)
#if (MODULE_MAUI == 2)
    #define SL_BUSY_PIN    (3)
    #define SL_BUSY_PORT   GPIO_PORTD
#else
    #define SL_BUSY_PIN    (0)
    #define SL_BUSY_PORT   GPIO_PORTA
#endif

    #define SL_ANTSW_PIN   (3)
    #define SL_ANTSW_PORT  GPIO_PORTA

#if (MODULE_MAUI == 2)
    #define SL_DIO_PIN     (0)
    #define SL_DIO_PORT    GPIO_PORTA
#else
    #define SL_DIO_PIN     (3)
    #define SL_DIO_PORT    GPIO_PORTD
#endif

    #define SL_NRESET_PIN  (2)
    #define SL_NRESET_PORT GPIO_PORTD

#if (MODULE_MAUI == 2)
    #define SL_PIN_BAND_SEL_PIN  (4)
    #define SL_PIN_BAND_SEL_PORT GPIO_PORTD
#else
    #define SL_PIN_BAND_SEL_PIN  (4)
    #define SL_PIN_BAND_SEL_PORT GPIO_PORTA
#endif
#else
    #error UNSUPPORTED PLATFORM!
#endif

struct GPIO_LookupItem gpio_lookup_table[] =
{
  [SL_PIN_BUSY] =      { .GPIO_Port    = SL_BUSY_PORT,
                         .Pin          = SL_BUSY_PIN,   // EXP_13
                         .callback     = NULL,
                         .callbackarg  = NULL,
                         .mode         = gpioModeDisabled,
                         .irq          = { .falling = false, .rising = false } },

  [SL_PIN_ANTSW] =     { .GPIO_Port    = SL_ANTSW_PORT,
                         .Pin          = SL_ANTSW_PIN,    // EXP_11
                         .callback     = NULL,
                         .callbackarg  = NULL,
                         .mode         = gpioModeDisabled,
                         .irq          = { .falling = false, .rising = false } },

  [SL_PIN_DIO] =       { .GPIO_Port    = SL_DIO_PORT,
                         .Pin          = SL_DIO_PIN,    // EXP_12
                         .callback     = NULL,
                         .callbackarg  = NULL,
                         .mode         = gpioModeDisabled,
                         .irq          = { .falling = false, .rising = false } },

  [SL_PIN_NRESET] =    { .GPIO_Port    = SL_NRESET_PORT,
                         .Pin          = SL_NRESET_PIN,   // EXP_14
                         .callback     = NULL,
                         .callbackarg  = NULL,
                         .mode         = gpioModeDisabled,
                         .irq          = { .falling = false, .rising = false } },
#if defined(EFR32MG21)
  [SL_PIN_NSS] =       { .GPIO_Port    = SL_SPIDRV_EXP_CS_PORT,
                         .Pin          = SL_SPIDRV_EXP_CS_PIN,    // EXP_10
                         .callback     = NULL,
                         .callbackarg  = NULL,
                         .mode         = gpioModeDisabled,
                         .irq          = { .falling = false, .rising = false } },
#elif defined(EFR32MG24)
  [SL_PIN_NSS] =       { .GPIO_Port    = SL_SPIDRV_EUSART_EXP_CS_PORT,
                         .Pin          = SL_SPIDRV_EUSART_EXP_CS_PIN,   // EXP_10
                         .callback     = NULL,
                         .callbackarg  = NULL,
                         .mode         = gpioModeDisabled,
                         .irq          = { .falling = false, .rising = false } },
#elif defined(EFR32BG21)
  [SL_PIN_NSS] =       { .GPIO_Port    = SL_SPIDRV_EXP_CS_PORT,
                         .Pin          = SL_SPIDRV_EXP_CS_PIN,    // EXP_10
                         .callback     = NULL,
                         .callbackarg  = NULL,
                         .mode         = gpioModeDisabled,
                         .irq          = { .falling = false, .rising = false } },
  [SL_PIN_MAUI_BAND_SEL] = {  .GPIO_Port     = SL_PIN_BAND_SEL_PORT,
                              .Pin           = SL_PIN_BAND_SEL_PIN,   // Maui (V2) sub-GHz Rx filter band select
                              .callback      = NULL,
                              .callbackarg   = NULL,
                              .mode          = gpioModeDisabled,
                              .irq           = { .falling = false, .rising = false } },
#endif
};

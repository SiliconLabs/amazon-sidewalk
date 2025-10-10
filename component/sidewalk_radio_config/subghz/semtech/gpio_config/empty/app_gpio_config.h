/***************************************************************************//**
 * @file
 * @brief app_gpio_config.h
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

#ifndef APP_GPIO_CONFIG_H
#define APP_GPIO_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "sl_spidrv_exp_config.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define SEMTECH_GPIO_NOT_CONFIGURED -1

// Used to indicate the status of internal state machine
#define SL_BUSY_PIN                              SEMTECH_GPIO_NOT_CONFIGURED
#define SL_BUSY_PORT                             SEMTECH_GPIO_NOT_CONFIGURED

// External antenna switch to control antenna switch to RECEIVE or
// TRANSMIT.
#define SL_ANTSW_PIN                             SEMTECH_GPIO_NOT_CONFIGURED
#define SL_ANTSW_PORT                            SEMTECH_GPIO_NOT_CONFIGURED

// IRQ line from sx126x chip
// See sx126x datasheet for IRQs list.
#define SL_DIO_PIN                               SEMTECH_GPIO_NOT_CONFIGURED
#define SL_DIO_PORT                              SEMTECH_GPIO_NOT_CONFIGURED

// Factory reset pin. Will be followed by standard calibration procedure
// and previous context will be lost.
#define SL_NRESET_PIN                            SEMTECH_GPIO_NOT_CONFIGURED
#define SL_NRESET_PORT                           SEMTECH_GPIO_NOT_CONFIGURED

#define SL_SX_CS_PIN                             SL_SPIDRV_EXP_CS_PIN
#define SL_SX_CS_PORT                            SL_SPIDRV_EXP_CS_PORT

// -----------------------------------------------------------------------------
//                              Compile-time Checks
// -----------------------------------------------------------------------------
// Ensure all GPIO macros are properly defined and configured

// Check all GPIO macros in a single statement
#if !defined(SL_BUSY_PIN) || !defined(SL_BUSY_PORT) || \
    !defined(SL_ANTSW_PIN) || !defined(SL_ANTSW_PORT) || \
    !defined(SL_DIO_PIN) || !defined(SL_DIO_PORT) || \
    !defined(SL_NRESET_PIN) || !defined(SL_NRESET_PORT) || \
    !defined(SL_SX_CS_PIN) || !defined(SL_SX_CS_PORT)
#error "All GPIO macros must be defined"
#endif

// Check if any GPIO is not configured (equals SEMTECH_GPIO_NOT_CONFIGURED)
#if (SL_BUSY_PIN == SEMTECH_GPIO_NOT_CONFIGURED) || \
    (SL_BUSY_PORT == SEMTECH_GPIO_NOT_CONFIGURED) || \
    (SL_ANTSW_PIN == SEMTECH_GPIO_NOT_CONFIGURED) || \
    (SL_ANTSW_PORT == SEMTECH_GPIO_NOT_CONFIGURED) || \
    (SL_DIO_PIN == SEMTECH_GPIO_NOT_CONFIGURED) || \
    (SL_DIO_PORT == SEMTECH_GPIO_NOT_CONFIGURED) || \
    (SL_NRESET_PIN == SEMTECH_GPIO_NOT_CONFIGURED) || \
    (SL_NRESET_PORT == SEMTECH_GPIO_NOT_CONFIGURED) || \
    (SL_SX_CS_PIN == SEMTECH_GPIO_NOT_CONFIGURED) || \
    (SL_SX_CS_PORT == SEMTECH_GPIO_NOT_CONFIGURED)
#error "All GPIO macros must be properly configured"
#endif

#ifdef __cplusplus
}
#endif

#endif /* APP_GPIO_CONFIG_H */

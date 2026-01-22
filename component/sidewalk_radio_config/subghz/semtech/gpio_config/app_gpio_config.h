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

#if !defined(SL_RADIO_NATIVE)
#include "sl_spidrv_exp_config.h"
#endif

#include "sl_emlib_gpio_init_exp_11_config.h"
#include "sl_emlib_gpio_init_exp_12_config.h"
#include "sl_emlib_gpio_init_exp_13_config.h"
#include "sl_emlib_gpio_init_exp_14_config.h"

#ifdef SL_LOCATION_FULL
#include "sl_emlib_gpio_init_exp_3_config.h"
#include "sl_emlib_gpio_init_exp_5_config.h"
#include "sl_emlib_gpio_init_exp_7_config.h"
#include "sl_emlib_gpio_init_exp_9_config.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
// BUSY on Expander Header Pin 13
// Used to indicate the status of internal state machine
#define SL_BUSY_PIN                              SL_EMLIB_GPIO_INIT_EXP_13_PIN
#define SL_BUSY_PORT                             SL_EMLIB_GPIO_INIT_EXP_13_PORT

#if defined(SID_RADIO_PLATFORM_SX126X)
// ANT_SW on Expander Header Pin 11
// External antenna switch to control antenna switch to RECEIVE or
// TRANSMIT.
#define SL_ANTSW_PIN                             SL_EMLIB_GPIO_INIT_EXP_11_PIN
#define SL_ANTSW_PORT                            SL_EMLIB_GPIO_INIT_EXP_11_PORT
#endif

// DIO1 on Expander Header Pin 12
// IRQ line from sx126x chip
// See sx126x datasheet for IRQs list.
#define SL_DIO_PIN                               SL_EMLIB_GPIO_INIT_EXP_12_PIN
#define SL_DIO_PORT                              SL_EMLIB_GPIO_INIT_EXP_12_PORT

// SX NRESET on Expander Header Pin 14
// Factory reset pin. Will be followed by standard calibration procedure
// and previous context will be lost.
#define SL_NRESET_PIN                            SL_EMLIB_GPIO_INIT_EXP_14_PIN
#define SL_NRESET_PORT                           SL_EMLIB_GPIO_INIT_EXP_14_PORT

#define SL_SX_CS_PIN                             SL_SPIDRV_EXP_CS_PIN
#define SL_SX_CS_PORT                            SL_SPIDRV_EXP_CS_PORT

#ifdef SL_LOCATION_FULL

#define SL_GNSS_LNA_PIN                      SL_EMLIB_GPIO_INIT_EXP_3_PIN
#define SL_GNSS_LNA_PORT                     SL_EMLIB_GPIO_INIT_EXP_3_PORT

#define SL_LED_RX_PIN                        SL_EMLIB_GPIO_INIT_EXP_9_PIN
#define SL_LED_RX_PORT                       SL_EMLIB_GPIO_INIT_EXP_9_PORT

#define SL_LED_TX_PIN                        SL_EMLIB_GPIO_INIT_EXP_7_PIN
#define SL_LED_TX_PORT                       SL_EMLIB_GPIO_INIT_EXP_7_PORT

#define SL_LED_SNIFFING_PIN                  SL_EMLIB_GPIO_INIT_EXP_5_PIN
#define SL_LED_SNIFFING_PORT                 SL_EMLIB_GPIO_INIT_EXP_5_PORT

#endif

#ifdef __cplusplus
}
#endif

#endif /* APP_GPIO_CONFIG_H */

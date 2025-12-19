/***************************************************************************//**
 * @file
 * @brief gpio.h
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

#ifndef GPIO_H
#define GPIO_H

/**
 * \addtogroup sid_pal
 * @{
 */
/**
 * \addtogroup sid_pal_gpio
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <sid_pal_gpio_ifc.h>
#include "sl_gpio.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @addtogroup sid_pal_gpio_types Type definitions
 * @ingroup sid_pal_gpio
 * @{
 *****************************************************************************/

/**
 * @brief Enumeration for GPIO pin assignments.
 *
 * This enumeration defines the various GPIO pin assignments.
 */
enum SL_PINout {
  SL_PIN_BUSY = 0,    /*!< GPIO pin for BUSY signal. */
  SL_PIN_ANTSW,       /*!< GPIO pin for antenna switch. */
  SL_PIN_DIO,         /*!< GPIO pin for DIO signal. */
  SL_PIN_NRESET,      /*!< GPIO pin for reset signal. */
  SL_PIN_NSS,         /*!< GPIO pin for NSS signal. */
#ifdef MODULE_KG100S
  SL_PIN_KG100S_BAND_SEL, /*!< GPIO pin for KG100S band selection. */
#endif
#ifdef SL_LOCATION_FULL
  SL_PIN_GNSS_LNA,      /*!< GPIO pin for GNSS LNA signal. */
  SL_PIN_LED_RX,        /*!< GPIO pin for LED RX signal. */
  SL_PIN_LED_TX,        /*!< GPIO pin for LED TX signal. */
  SL_PIN_LED_SNIFFING,  /*!< GPIO pin for LED Sniffing signal. */
#endif
  SL_PIN_MAX              /*!< Maximum number of GPIO pins. */
};

/**
 * @brief Configuration structure for a GPIO.
 *
 * This structure defines the configuration parameters for a GPIO.
 */
struct GPIO_PinConfig{
  sid_pal_gpio_direction_t dir;       /*!< Direction of the GPIO pin (input/output). */
  sid_pal_gpio_input_t input_mode;    /*!< Input mode configuration for the GPIO pin. */
  sid_pal_gpio_output_t output_mode;  /*!< Output mode configuration for the GPIO pin. */
  sid_pal_gpio_pull_t pull_mode;      /*!< Pull mode configuration for the GPIO pin. */
};

/**
 * @brief Lookup item structure for GPIO configuration.
 *
 * This structure defines the lookup item for GPIO configuration, including port, pin,
 * pin configuration, mode, interrupt settings, and callback information.
 */
struct GPIO_LookupItem{
  sl_gpio_t gpio;                       /*!< gpio port and pin number. */
  int32_t IntNO;                        /*!< Interrupt number assigned by sl_gpio_configure_external_interrupt */
  struct GPIO_PinConfig PinConfig;      /*!< Configuration parameters for the GPIO pin. */
  sl_gpio_mode_t mode;                  /*!< Mode of the GPIO pin. */
  sid_pal_gpio_irq_handler_t callback;  /*!< Callback function to be called on GPIO interrupt. */
  struct {
    bool falling;                       /*!< Indicates if the interrupt is triggered on falling edge. */
    bool rising;                        /*!< Indicates if the interrupt is triggered on rising edge. */
  }
  irq;                                  /*!< Interrupt configuration for the GPIO pin. */
  void * callbackarg;                   /*!< Argument to be passed to the callback function. */
};

/** @} (end sid_pal_gpio_types) */

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H */

/** @} */ // end of sid_pal group
/** @} */ // end of sid_pal_gpio group

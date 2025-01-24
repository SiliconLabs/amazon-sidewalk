/***************************************************************************//**
 * @file
 * @brief gpio.c
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <sid_pal_assert_ifc.h>

#include "gpiointerrupt.h"
#include <gpio.h>

#ifdef SL_SIDEWALK_UNIT_TEST
#include "em_gpio_test.h"
#endif

// -----------------------------------------------------------------------------
//                            Defines for Unittesting
// -----------------------------------------------------------------------------
#ifdef SL_SIDEWALK_UNIT_TEST
#define GPIO_PinInGet GPIO_PinInGet_stub
#define GPIO_PinOutSet GPIO_PinOutSet_stub
#define GPIO_PinOutClear GPIO_PinOutClear_stub
#define GPIO_PinOutToggle GPIO_PinOutToggle_stub
#endif

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
// gpio application specific config
extern struct GPIO_LookupItem gpio_lookup_table[];

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static void gpio_irq_handler(uint8_t pin)
{
  for (uint8_t ix = 0; ix < SL_PIN_MAX; ix++) {
    if (pin == gpio_lookup_table[ix].Pin) {
      if (gpio_lookup_table[ix].callback) {
        gpio_lookup_table[ix].callback(ix, gpio_lookup_table[ix].callbackarg);
      }
    }
  }
}

static void sid_pal_pin_mode_input(struct GPIO_LookupItem *lookupptr,
                                   enum GPIO_Mode_TypeDef_enum *mode,
                                   bool *out)
{
  if (lookupptr->PinConfig.input_mode == SID_PAL_GPIO_INPUT_DISCONNECT) {
    if (lookupptr->PinConfig.pull_mode == SID_PAL_GPIO_PULL_UP) {
      *mode = gpioModeDisabled;
      *out = 1;
    } else {
      *mode = gpioModeDisabled;
      *out = 0;
    }
  } else if (lookupptr->PinConfig.input_mode == SID_PAL_GPIO_INPUT_CONNECT) {
    if (lookupptr->PinConfig.pull_mode == SID_PAL_GPIO_PULL_UP) {
      *mode = gpioModeInputPull;
      *out = 1;
    } else if (lookupptr->PinConfig.pull_mode == SID_PAL_GPIO_PULL_DOWN) {
      *mode = gpioModeInputPull;
      *out = 0;
    } else if (lookupptr->PinConfig.pull_mode == SID_PAL_GPIO_PULL_NONE) {
      *mode = gpioModeInput;
      *out = 0;
    }
  }
}

static void sid_pal_pin_mode_output(struct GPIO_LookupItem *lookupptr,
                                    enum GPIO_Mode_TypeDef_enum *mode,
                                    bool *out)
{
  if (lookupptr->PinConfig.output_mode == SID_PAL_GPIO_OUTPUT_PUSH_PULL) {
    *mode = gpioModePushPull;
    *out = 0;
  } else if (lookupptr->PinConfig.output_mode == SID_PAL_GPIO_OUTPUT_OPEN_DRAIN) {
    if (lookupptr->PinConfig.pull_mode == SID_PAL_GPIO_PULL_UP) {
      *mode = gpioModeWiredAndPullUp;
      *out = 0;
    } else {
      *mode = gpioModeWiredAnd;
      *out = 0;
    }
  }
}

static void sid_pal_pin_mode_set(struct GPIO_LookupItem *lookupptr)
{
  enum GPIO_Mode_TypeDef_enum mode = gpioModeDisabled;
  bool out = 0;

  if (lookupptr->PinConfig.dir == SID_PAL_GPIO_DIRECTION_INPUT) {
    sid_pal_pin_mode_input(lookupptr, &mode, &out);
  } else if (lookupptr->PinConfig.dir == SID_PAL_GPIO_DIRECTION_OUTPUT) {
    sid_pal_pin_mode_output(lookupptr, &mode, &out);
  }

  GPIO_PinModeSet(lookupptr->GPIO_Port, lookupptr->Pin, mode, out);
  lookupptr->mode = mode;
}

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
sid_error_t sid_pal_gpio_set_direction(uint32_t gpio_number,
                                       sid_pal_gpio_direction_t direction)
{
  struct GPIO_LookupItem * lookupptr;
  sid_error_t retval = SID_ERROR_NONE;

  if (direction > SID_PAL_GPIO_DIRECTION_OUTPUT) {
    return SID_ERROR_INVALID_ARGS;
  }

  if (gpio_number < SL_PIN_MAX) {
    lookupptr = &gpio_lookup_table[gpio_number];

    lookupptr->PinConfig.dir = direction;

    sid_pal_pin_mode_set(lookupptr);
    retval = SID_ERROR_NONE;
  } else {
    retval = SID_ERROR_INVALID_ARGS;
  }
  return retval;
}

sid_error_t sid_pal_gpio_input_mode(uint32_t gpio_number,
                                    sid_pal_gpio_input_t mode)
{
  struct GPIO_LookupItem * lookupptr;
  sid_error_t retval = SID_ERROR_NONE;

  if (mode > SID_PAL_GPIO_INPUT_DISCONNECT) {
    return SID_ERROR_INVALID_ARGS;
  }

  if (gpio_number < SL_PIN_MAX) {
    lookupptr = &gpio_lookup_table[gpio_number];

    lookupptr->PinConfig.input_mode = mode;

    sid_pal_pin_mode_set(lookupptr);
    retval = SID_ERROR_NONE;
  } else {
    retval = SID_ERROR_INVALID_ARGS;
  }
  return retval;
}

sid_error_t sid_pal_gpio_output_mode(uint32_t gpio_number,
                                     sid_pal_gpio_output_t mode)
{
  struct GPIO_LookupItem * lookupptr;
  sid_error_t retval = SID_ERROR_NONE;

  if (mode > SID_PAL_GPIO_OUTPUT_OPEN_DRAIN) {
    return SID_ERROR_INVALID_ARGS;
  }

  if (gpio_number < SL_PIN_MAX) {
    lookupptr = &gpio_lookup_table[gpio_number];

    lookupptr->PinConfig.output_mode = mode;

    sid_pal_pin_mode_set(lookupptr);

    retval = SID_ERROR_NONE;
  } else {
    retval = SID_ERROR_INVALID_ARGS;
  }
  return retval;
}

sid_error_t sid_pal_gpio_pull_mode(uint32_t gpio_number,
                                   sid_pal_gpio_pull_t pull)
{
  struct GPIO_LookupItem * lookupptr;
  sid_error_t retval = SID_ERROR_NONE;

  if (pull > SID_PAL_GPIO_PULL_DOWN) {
    return SID_ERROR_INVALID_ARGS;
  }

  if (gpio_number < SL_PIN_MAX) {
    lookupptr = &gpio_lookup_table[gpio_number];

    lookupptr->PinConfig.pull_mode = pull;

    sid_pal_pin_mode_set(lookupptr);
    retval = SID_ERROR_NONE;
  } else {
    retval = SID_ERROR_INVALID_ARGS;
  }
  return retval;
}

sid_error_t sid_pal_gpio_read(uint32_t gpio_number,
                              uint8_t * value)
{
  struct GPIO_LookupItem * lookupptr;
  sid_error_t retval = SID_ERROR_NONE;

  if (gpio_number < SL_PIN_MAX) {
    lookupptr = &gpio_lookup_table[gpio_number];
    *value = GPIO_PinInGet(lookupptr->GPIO_Port, lookupptr->Pin);
  } else {
    retval = SID_ERROR_INVALID_ARGS;
  }
  return retval;
}

sid_error_t sid_pal_gpio_write(uint32_t gpio_number,
                               uint8_t value)
{
  struct GPIO_LookupItem * lookupptr;
  sid_error_t retval = SID_ERROR_NONE;

  if (gpio_number < SL_PIN_MAX) {
    lookupptr = &gpio_lookup_table[gpio_number];

    if (value) {
      GPIO_PinOutSet(lookupptr->GPIO_Port, lookupptr->Pin);
    } else {
      GPIO_PinOutClear(lookupptr->GPIO_Port, lookupptr->Pin);
    }
  } else {
    retval = SID_ERROR_INVALID_ARGS;
  }
  return retval;
}

sid_error_t sid_pal_gpio_toggle(uint32_t gpio_number)
{
  struct GPIO_LookupItem * lookupptr;
  sid_error_t retval = SID_ERROR_NONE;

  if (gpio_number < SL_PIN_MAX) {
    lookupptr = &gpio_lookup_table[gpio_number];
    GPIO_PinOutToggle(lookupptr->GPIO_Port, lookupptr->Pin);
  } else {
    retval = SID_ERROR_INVALID_ARGS;
  }
  return retval;
}

sid_error_t sid_pal_gpio_set_irq(uint32_t gpio_number,
                                 sid_pal_gpio_irq_trigger_t irq_trigger,
                                 sid_pal_gpio_irq_handler_t gpio_callback,
                                 void * callback_arg)
{
  bool IsRisingEdge = false;
  bool IsFallingEdge = false;
  bool enableIrq = true;

  struct GPIO_LookupItem * lookupptr;
  sid_error_t retval = SID_ERROR_NONE;

  switch (irq_trigger) {
    case SID_PAL_GPIO_IRQ_TRIGGER_NONE:
      enableIrq = false;
      break;

    case SID_PAL_GPIO_IRQ_TRIGGER_RISING:
      IsRisingEdge = true;
      IsFallingEdge = false;
      break;

    case SID_PAL_GPIO_IRQ_TRIGGER_FALLING:
      IsRisingEdge = false;
      IsFallingEdge = true;
      break;

    case SID_PAL_GPIO_IRQ_TRIGGER_EDGE:
      IsRisingEdge = true;
      IsFallingEdge = true;
      break;

    default:
      return SID_ERROR_NOSUPPORT;
  }

  if (gpio_number < SL_PIN_MAX) {
    lookupptr = &gpio_lookup_table[gpio_number];
    lookupptr->irq.falling = IsFallingEdge;
    lookupptr->irq.rising = IsRisingEdge;
    lookupptr->callback = gpio_callback;
    lookupptr->callbackarg = callback_arg;
    GPIOINT_CallbackRegister(lookupptr->Pin, gpio_irq_handler);
    GPIO_ExtIntConfig(lookupptr->GPIO_Port, lookupptr->Pin, lookupptr->Pin, IsRisingEdge, IsFallingEdge, enableIrq);
  } else {
    retval = SID_ERROR_INVALID_ARGS;
  }
  return retval;
}

sid_error_t sid_pal_gpio_irq_enable(uint32_t gpio_number)
{
  sid_error_t retval = SID_ERROR_NONE;
  struct GPIO_LookupItem * lookupptr;

  lookupptr = &gpio_lookup_table[gpio_number];

  if (gpio_number < SL_PIN_MAX) {
    GPIO_ExtIntConfig(lookupptr->GPIO_Port,
                      lookupptr->Pin,
                      lookupptr->Pin,
                      lookupptr->irq.rising,
                      lookupptr->irq.falling,
                      true);
  } else {
    retval = SID_ERROR_INVALID_ARGS;
  }
  return retval;
}

sid_error_t sid_pal_gpio_irq_disable(uint32_t gpio_number)
{
  sid_error_t retval = SID_ERROR_NONE;
  struct GPIO_LookupItem * lookupptr;

  if (gpio_number < SL_PIN_MAX) {
    lookupptr = &gpio_lookup_table[gpio_number];
    GPIO_ExtIntConfig(lookupptr->GPIO_Port,
                      lookupptr->Pin,
                      lookupptr->Pin,
                      lookupptr->irq.rising,
                      lookupptr->irq.falling,
                      false);
  } else {
    retval = SID_ERROR_INVALID_ARGS;
  }
  return retval;
}

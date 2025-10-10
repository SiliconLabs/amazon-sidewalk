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

#include <gpio.h>

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
// gpio application specific config
extern struct GPIO_LookupItem gpio_lookup_table[];

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static void gpio_irq_handler(uint8_t pin, void *context)
{
  (void)context;
  for (uint8_t ix = 0; ix < SL_PIN_MAX; ix++) {
    if ((pin == gpio_lookup_table[ix].IntNO) && (gpio_lookup_table[ix].callback)) {
      gpio_lookup_table[ix].callback(ix, gpio_lookup_table[ix].callbackarg);
      break;
    }
  }
}

static void sid_pal_pin_mode_input(struct GPIO_LookupItem *lookupptr,
                                   sl_gpio_mode_t *mode,
                                   bool *out)
{
  if (lookupptr->PinConfig.input_mode == SID_PAL_GPIO_INPUT_DISCONNECT) {
    if (lookupptr->PinConfig.pull_mode == SID_PAL_GPIO_PULL_UP) {
      *mode = SL_GPIO_MODE_DISABLED;
      *out = 1;
    } else {
      *mode = SL_GPIO_MODE_DISABLED;
      *out = 0;
    }
  } else if (lookupptr->PinConfig.input_mode == SID_PAL_GPIO_INPUT_CONNECT) {
    if (lookupptr->PinConfig.pull_mode == SID_PAL_GPIO_PULL_UP) {
      *mode = SL_GPIO_MODE_INPUT_PULL;
      *out = 1;
    } else if (lookupptr->PinConfig.pull_mode == SID_PAL_GPIO_PULL_DOWN) {
      *mode = SL_GPIO_MODE_INPUT_PULL;
      *out = 0;
    } else if (lookupptr->PinConfig.pull_mode == SID_PAL_GPIO_PULL_NONE) {
      *mode = SL_GPIO_MODE_INPUT;
      *out = 0;
    }
  }
}

static void sid_pal_pin_mode_output(struct GPIO_LookupItem *lookupptr,
                                    sl_gpio_mode_t *mode,
                                    bool *out)
{
  if (lookupptr->PinConfig.output_mode == SID_PAL_GPIO_OUTPUT_PUSH_PULL) {
    *mode = SL_GPIO_MODE_PUSH_PULL;
    *out = 0;
  } else if (lookupptr->PinConfig.output_mode == SID_PAL_GPIO_OUTPUT_OPEN_DRAIN) {
    if (lookupptr->PinConfig.pull_mode == SID_PAL_GPIO_PULL_UP) {
      *mode = SL_GPIO_MODE_WIRED_AND_PULLUP;
      *out = 0;
    } else {
      *mode = SL_GPIO_MODE_WIRED_AND;
      *out = 0;
    }
  }
}

static sid_error_t sid_pal_pin_mode_set(struct GPIO_LookupItem *lookupptr)
{
  sl_gpio_mode_t mode = SL_GPIO_MODE_DISABLED;
  bool out = 0;

  if (lookupptr->PinConfig.dir == SID_PAL_GPIO_DIRECTION_INPUT) {
    sid_pal_pin_mode_input(lookupptr, &mode, &out);
  } else if (lookupptr->PinConfig.dir == SID_PAL_GPIO_DIRECTION_OUTPUT) {
    sid_pal_pin_mode_output(lookupptr, &mode, &out);
  }

  if (sl_gpio_set_pin_mode(&lookupptr->gpio, mode, out) != SL_STATUS_OK) {
    return SID_ERROR_GENERIC;
  }

  lookupptr->mode = mode;

  return SID_ERROR_NONE;
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

    retval = sid_pal_pin_mode_set(lookupptr);
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

    retval = sid_pal_pin_mode_set(lookupptr);
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

    retval = sid_pal_pin_mode_set(lookupptr);
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

    retval = sid_pal_pin_mode_set(lookupptr);
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
  bool pin_value = false;

  if (gpio_number < SL_PIN_MAX) {
    lookupptr = &gpio_lookup_table[gpio_number];
    if (sl_gpio_get_pin_input(&lookupptr->gpio, &pin_value) != SL_STATUS_OK) {
      retval = SID_ERROR_GENERIC;
    }
  } else {
    retval = SID_ERROR_INVALID_ARGS;
  }

  if (pin_value) {
    *value = 1;
  } else {
    *value = 0;
  }

  return retval;
}

sid_error_t sid_pal_gpio_write(uint32_t gpio_number,
                               uint8_t value)
{
  struct GPIO_LookupItem * lookupptr;
  sl_status_t status;
  sid_error_t retval = SID_ERROR_NONE;

  if (gpio_number < SL_PIN_MAX) {
    lookupptr = &gpio_lookup_table[gpio_number];

    if (value) {
      status = sl_gpio_set_pin(&lookupptr->gpio);
    } else {
      status = sl_gpio_clear_pin(&lookupptr->gpio);
    }

    if (status != SL_STATUS_OK) {
      retval = SID_ERROR_GENERIC;
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
    if (sl_gpio_toggle_pin(&lookupptr->gpio) != SL_STATUS_OK) {
      retval = SID_ERROR_GENERIC;
    }
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
  sl_gpio_interrupt_flag_t flags;

  struct GPIO_LookupItem * lookupptr;
  sl_status_t status;

  switch (irq_trigger) {
    case SID_PAL_GPIO_IRQ_TRIGGER_NONE:
      flags = SL_GPIO_INTERRUPT_NO_EDGE;
      break;

    case SID_PAL_GPIO_IRQ_TRIGGER_RISING:
      IsRisingEdge = true;
      IsFallingEdge = false;
      flags = SL_GPIO_INTERRUPT_RISING_EDGE;
      break;

    case SID_PAL_GPIO_IRQ_TRIGGER_FALLING:
      IsRisingEdge = false;
      IsFallingEdge = true;
      flags = SL_GPIO_INTERRUPT_FALLING_EDGE;
      break;

    case SID_PAL_GPIO_IRQ_TRIGGER_EDGE:
      IsRisingEdge = true;
      IsFallingEdge = true;
      flags = SL_GPIO_INTERRUPT_RISING_FALLING_EDGE;
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
    if (lookupptr->IntNO != SL_GPIO_INTERRUPT_UNAVAILABLE) {
      status = sl_gpio_deconfigure_external_interrupt(lookupptr->IntNO);
      if (status != SL_STATUS_OK) {
        return SID_ERROR_GENERIC;
      }
      lookupptr->IntNO = SL_GPIO_INTERRUPT_UNAVAILABLE;
    }
    status = sl_gpio_configure_external_interrupt(&lookupptr->gpio, &(lookupptr->IntNO), flags, gpio_irq_handler, NULL);
    if (status != SL_STATUS_OK) {
      return SID_ERROR_GENERIC;
    } else if (lookupptr->IntNO == SL_GPIO_INTERRUPT_UNAVAILABLE) {
      return SID_ERROR_INVALID_ARGS;
    }
  } else {
    return SID_ERROR_INVALID_ARGS;
  }
  return SID_ERROR_NONE;
}

sid_error_t sid_pal_gpio_irq_enable(uint32_t gpio_number)
{
  sid_error_t retval = SID_ERROR_NONE;
  struct GPIO_LookupItem * lookupptr;

  if (gpio_number < SL_PIN_MAX) {
    lookupptr = &gpio_lookup_table[gpio_number];
    if ((lookupptr->IntNO != SL_GPIO_INTERRUPT_UNAVAILABLE) &&
        (sl_gpio_enable_interrupts(1 << lookupptr->IntNO) != SL_STATUS_OK)) {
        retval = SID_ERROR_GENERIC;
    }
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
    if ((lookupptr->IntNO != SL_GPIO_INTERRUPT_UNAVAILABLE) &&
        (sl_gpio_disable_interrupts(1 << lookupptr->IntNO) != SL_STATUS_OK)) {
        retval = SID_ERROR_GENERIC;
    }
  } else {
    retval = SID_ERROR_INVALID_ARGS;
  }
  return retval;
}

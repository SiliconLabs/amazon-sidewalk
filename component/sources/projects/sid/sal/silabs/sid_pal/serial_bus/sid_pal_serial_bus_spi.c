/***************************************************************************//**
 * @file
 * @brief sid_pal_serial_bus_spi.c
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
#include <sid_pal_serial_bus_ifc.h>
#include <sid_pal_serial_bus_efr32_spi_config.h>

#include <sid_pal_gpio_ifc.h>

#include <spidrv.h>
#include "sl_spidrv_instances.h"
#include <em_usart.h>
#include <gpio.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define SPI_TRANSFER_TIMEOUT_MS 200u

#define SLI_CONTAINEROF(ptr, type, member)                         \
  ({                                                               \
    const __typeof__(((type *)0)->member) * tmp_member_ = (ptr);   \
    ((type *)((uintptr_t)(tmp_member_) - offsetof(type, member))); \
  })

struct serial_bus_efr32_spi_ctx {
  struct sid_pal_serial_bus_iface iface;
  struct sid_pal_serial_bus_efr32_spi_config config;
};

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static sid_error_t bus_serial_spi_xfer(const struct sid_pal_serial_bus_iface *iface,
                                       const struct sid_pal_serial_bus_client *client,
                                       uint8_t *tx,
                                       uint8_t *rx,
                                       size_t xfer_size)
{
  if (!iface || !client || (!tx && !rx) || !(xfer_size)) {
    return SID_ERROR_INVALID_ARGS;
  }

  struct serial_bus_efr32_spi_ctx *bus = SLI_CONTAINEROF(iface, struct serial_bus_efr32_spi_ctx, iface);

  sid_pal_gpio_write(client->client_selector, 0);

  for (size_t i = 0; i < xfer_size; i++) {
    rx[i] = (uint8_t)USART_SpiTransfer(bus->config.peripheral_id, tx[i]);
  }

  sid_pal_gpio_write(client->client_selector, 1);

  return SID_ERROR_NONE;
}

static sid_error_t bus_serial_spi_destroy(const struct sid_pal_serial_bus_iface *iface)
{
  if (!iface) {
    return SID_ERROR_INVALID_ARGS;
  }

  return SID_ERROR_NONE;
}

static const struct sid_pal_serial_bus_iface bus_ops = {
  .xfer = bus_serial_spi_xfer,
  .destroy = bus_serial_spi_destroy,
  .xfer_hd = NULL,
};

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
sid_error_t sid_pal_serial_bus_efr32_spi_create(const struct sid_pal_serial_bus_iface **iface, const void *cfg)
{
  if (!iface || !cfg) {
    return SID_ERROR_INVALID_ARGS;
  }
  static struct serial_bus_efr32_spi_ctx bus = { 0 };

  sid_pal_gpio_pull_mode(SL_PIN_NSS, SID_PAL_GPIO_PULL_UP);
  sid_pal_gpio_set_direction(SL_PIN_NSS, SID_PAL_GPIO_DIRECTION_OUTPUT);

  // Set ANT_SW pin to logical 1, as DIO2 pin is used for antenna switch instead
  sid_pal_gpio_set_direction(SL_PIN_ANTSW, SID_PAL_GPIO_DIRECTION_OUTPUT);
  sid_pal_gpio_pull_mode(SL_PIN_ANTSW, SID_PAL_GPIO_PULL_UP);

  const struct sid_pal_serial_bus_efr32_spi_config *config = (struct sid_pal_serial_bus_efr32_spi_config *)cfg;
  bus.config = *config;

  bus.iface = bus_ops;
  *iface = &bus.iface;
  return SID_ERROR_NONE;
}

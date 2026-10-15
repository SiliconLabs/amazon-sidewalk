/*
 * Copyright 2022-2023 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_SERIAL_BUS_IFC_H
#define SID_PAL_SERIAL_BUS_IFC_H

/**
 * \addtogroup sid_ifc
 * @{
 */
/**
 * \addtogroup sid_pal_ifc
 * @{
 */
/**
 * \addtogroup sid_pal_peripheral_ifc
 * @{
 */
/**
 * \addtogroup sid_pal_sbus_ifc
 * @{
 */

#include <sid_error.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * @addtogroup sid_pal_sbus_ifc_types Type definitions
 * @ingroup sid_pal_sbus_ifc
 * @{
 *****************************************************************************/

/**
 * @brief Describes the bit order of messages exchanged on serial bus interface.
 */
enum sid_pal_serial_bus_bit_order {
    SID_PAL_SERIAL_BUS_BIT_ORDER_MSB_FIRST, /*!< Most significant bit first. */
    SID_PAL_SERIAL_BUS_BIT_ORDER_LSB_FIRST, /*!< Least significant bit first. */
};

/**
 * @brief Describes the client selection or deselection
 */
enum sid_pal_serial_bus_client_select {
    SID_PAL_SERIAL_BUS_CLIENT_DESELECT, /*!< Deselect the serial bus client */
    SID_PAL_SERIAL_BUS_CLIENT_SELECT,   /*!< Select the serial bus client */
};

/**
 * @brief Describes the configuration of the serial bus client.
 */
struct sid_pal_serial_bus_client {
    uint32_t client_selector;                       /*!< client id on the serial bus.*/
    uint32_t speed_hz;                              /*!< baud rate.*/
    enum sid_pal_serial_bus_bit_order bit_order;    /*!< bit order.*/
    uint8_t mode;                                   /*!< serial bus mode.*/
    /**
     * Callback to select client on serial bus
     *
     * If this callback if not set, then  #client_selector will be used.
     *
     * @param[in] client A pointer to #sid_pal_serial_bus_client, which is never NULL
     * @param[in] select Enum value that indicates if client should be selected or not
     * @param[in] context The context pointer given in #client_selector_context
     *
     * @retval true if client selection is successful, false otherwise
     * **/
    bool (*client_selector_cb)(const struct sid_pal_serial_bus_client *const client,
                               enum sid_pal_serial_bus_client_select select,
                               void *context);
    void *client_selector_context;                  /*!< Context returned back in #client_selector_cb */
};

struct sid_pal_serial_bus_iface;

/**
 * @brief The set of callbacks the implementation supports.
 */
struct sid_pal_serial_bus_iface {
    /**
     * @brief Callback to transfer messages in full duplex mode.
     *
     * @param[in] iface pointer to serial bus interface.
     * @param[in] client pointer to serial bus client.
     * @param[in] tx pointer to the message to be sent.
     * @param[out] rx pointer to the message to be received.
     * @param[in] xfer_size maximum length of the message sent or received.
     * @return sid_error_t error code indicating the result of the operation.
     */
    sid_error_t (*xfer)(const struct sid_pal_serial_bus_iface *iface,
                        const struct sid_pal_serial_bus_client *client,
                        uint8_t *tx,
                        uint8_t *rx,
                        size_t xfer_size);
    /**
     * @brief Callback to transfer messages in half duplex mode.
     *
     * @param[in] iface pointer to serial bus interface.
     * @param[in] client pointer to serial bus client.
     * @param[in] tx pointer to the message to be sent.
     * @param[out] rx pointer to the message to be received.
     * @param[in] tx_size maximum length of the message to be sent.
     * @param[in] rx_size maximum length of the message to be received.
     * @return sid_error_t error code indicating the result of the operation.
     */
    sid_error_t (*xfer_hd)(const struct sid_pal_serial_bus_iface *iface,
                           const struct sid_pal_serial_bus_client *client,
                           uint8_t *tx,
                           uint8_t *rx,
                           size_t tx_size,
                           size_t rx_size);
    /**
     * @brief Callback to delete the serial bus interface.
     *
     * @param[in] iface pointer to serial bus interface.
     * @return sid_error_t error code indicating the result of the operation.
     */
    sid_error_t (*destroy)(const struct sid_pal_serial_bus_iface *iface);
};

/**
 * @brief Factory for creating serial bus client interfaces.
 */
struct sid_pal_serial_bus_factory {
    /**
     * @brief Callback to create serial bus client interface
     *
     * @param[in] iface pointer to serial bus interface.
     * @param[in] config pointer to client config.
     * @return sid_error_t error code indicating the result of the operation.
     */
    sid_error_t (*create)(const struct sid_pal_serial_bus_iface **iface, const void *config);
    const void *config; /*!< pointer to client config*/
};

/** @} (end sid_pal_sbus_ifc_types) */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_PAL_SERIAL_BUS_IFC_H */

/** @} */ // end of sid_ifc group
/** @} */ // end of sid_pal_ifc group
/** @} */ // end of sid_pal_peripheral_ifc group
/** @} */ // end of sid_pal_sbus_ifc group

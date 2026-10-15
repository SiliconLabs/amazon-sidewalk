/***************************************************************************//**
 * @file sl_sidewalk_ota_dfu.h
 * @brief sidewalk over-the-air device firmware upgrade component
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SL_SIDEWALK_OTA_DFU_H
#define SL_SIDEWALK_OTA_DFU_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "sid_api.h"
#include "sid_bulk_data_transfer_api.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/*******************************************************************************
 * @brief
 *   Queues a message to release the OTA DFU buffer
 *
 * @returns None
 ******************************************************************************/
extern void sl_app_trigger_ota_dfu_release_buffer(void);

/***************************************************************************//**
 * @brief
 *   Initializes over-the-air device firmware upgrade service.
 *
 * @param[in] sidewalk_handle Current sidewalk handle
 ******************************************************************************/
void sl_sid_ota_dfu_init(struct sid_handle *sidewalk_handle);

/***************************************************************************//**
 * @brief
 *   Deinitializes over-the-air device firmware upgrade service.
 *
 * @param[in] sidewalk_handle Current sidewalk handle
 ******************************************************************************/
void sl_sid_ota_dfu_deinit(struct sid_handle *sidewalk_handle);

/***************************************************************************//**
 * @brief
 *   Cancels the ongoing over-the-air device firmware upgrade process.
 *
 * @param[in] sidewalk_handle Current sidewalk handle
 ******************************************************************************/
void sl_sid_ota_dfu_cancel(struct sid_handle *sidewalk_handle);

/***************************************************************************//**
 * @brief
 *   Stats for the ongoing over-the-air device firmware process.
 *
 * @param[in] sidewalk_handle Current sidewalk handle
 ******************************************************************************/
void sl_sid_ota_dfu_stat(struct sid_handle *sidewalk_handle);

/***************************************************************************//**
 * @brief
 *   Parameters of the ongoing over-the-air device firmware process.
 *
 * @param[in] sidewalk_handle Current sidewalk handle
 ******************************************************************************/
void sl_sid_ota_dfu_param(struct sid_handle *sidewalk_handle);

/***************************************************************************//**
 * @brief
 *   Minimum scratch buffer size based on a given fragment of over-the-air
 *   device firmware seervice.
 ******************************************************************************/
void sl_sid_ota_dfu_min_scratch_buf_size(void);

/***************************************************************************//**
 * @brief
 *   Releases OTA DFU buffer outside of on_data_received callback context.
 ******************************************************************************/
void sl_sid_ota_dfu_release_buffer(void);

#ifdef __cplusplus
}
#endif

#endif  // SL_SIDEWALK_OTA_DFU_H

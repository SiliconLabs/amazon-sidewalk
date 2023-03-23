/***************************************************************************//**
 * @file
 * @brief app_process.h
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
#ifndef APP_PROCESS_H
#define APP_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>

#include "sid_api.h"
#if defined(EFR32MG21) || defined(EFR32MG24)
#include "sl_simple_button_instances.h"
#endif
#include "app_init.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// Unused function parameter
#define UNUSED(x) (void)(x)

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

extern link_status_t link_status;
extern struct sid_msg_desc LAST_MESSG_RCVD_DESC;
extern QueueHandle_t g_event_queue;
extern uint32_t current_init_link;

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Main task
 ******************************************************************************/
void main_task(void *context);

#if defined(EFR32MG21) || defined(EFR32MG24)
/*******************************************************************************
 * Button handler callback
 ******************************************************************************/
void sl_button_on_change(const sl_button_t *handle);
#endif

/*******************************************************************************
 * Issue a queue event
 ******************************************************************************/
void queue_event(QueueHandle_t queue, enum event_type event, bool in_isr);

#ifdef __cplusplus
}
#endif

#endif // APP_PROCESS_H

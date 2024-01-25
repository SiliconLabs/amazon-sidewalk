/***************************************************************************//**
 * @file
 * @brief Sidewalk PAL configuration
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

#ifndef SL_SIDEWALK_PAL_CONFIG_H
#define SL_SIDEWALK_PAL_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

#define SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT   1
#define SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD     2

// <h> Sidewalk PAL configuration
// <o SL_SIDEWALK_PAL_SWI_IMPL_METHOD> SWI implementation method
// <SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT=> SWI interrupt
// <SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD=> RTOS thread
// <i> Default: SL_SIDEWALK_PAL_SWI_IMPL_METHOD_RTOS_THREAD
#ifndef SL_SIDEWALK_PAL_SWI_IMPL_METHOD
#define SL_SIDEWALK_PAL_SWI_IMPL_METHOD SL_SIDEWALK_PAL_SWI_IMPL_METHOD_SWI_INTERRUPT
#endif
// </h>

// <<< end of configuration section >>>

#endif // SL_SIDEWALK_PAL_CONFIG_H

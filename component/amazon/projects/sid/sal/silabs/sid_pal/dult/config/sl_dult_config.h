/***************************************************************************//**
 * @file
 * @brief DULT configuration file
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_DULT_CONFIG_H
#define SL_DULT_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> DULT

// <o SL_DULT_CONFIG_INSTANCE_MAX> DULT instance count
// <i> Default: 3
// <i> DULT instance count
#ifndef SL_DULT_CONFIG_INSTANCE_MAX
#define SL_DULT_CONFIG_INSTANCE_MAX 3
#endif

// <o SL_DULT_CONFIG_IDENTIFIER_READ_STATE_TIMEOUT_MS> DULT identifier read timeout [ms]
// <i> Default: 300000
// <i> DULT identifier read timeout in millisecond
#ifndef SL_DULT_CONFIG_IDENTIFIER_READ_STATE_TIMEOUT_MS
#define SL_DULT_CONFIG_IDENTIFIER_READ_STATE_TIMEOUT_MS 300000
#endif

// </h>

// <h> Advertising

// <o SL_DULT_CONFIG_ADV_INTERVAL_MIN> DULT advertising interval minimal [0.625ms]
// <i> Default: 3200
// <i> DULT advertising interval minimal (in units of 0.625 ms.)
#ifndef SL_DULT_CONFIG_ADV_INTERVAL_MIN
#define SL_DULT_CONFIG_ADV_INTERVAL_MIN 3200
#endif

// <o SL_DULT_CONFIG_ADV_INTERVAL_MAX> DULT advertising interval max [0.625ms]
// <i> Default: 3200
// <i> DULT advertising interval max (in units of 0.625 ms.)
#ifndef SL_DULT_CONFIG_ADV_INTERVAL_MAX
#define SL_DULT_CONFIG_ADV_INTERVAL_MAX 3200
#endif

// <o SL_DULT_CONFIG_ADV_DURATION> DULT advertising duration [10ms]
// <i> Default: 0
// <i> DULT advertising duration (in units of 10 ms.)
#ifndef SL_DULT_CONFIG_ADV_DURATION
#define SL_DULT_CONFIG_ADV_DURATION 0
#endif


// </h>

// <h> Motion detection

// <o SL_DULT_CONFIG_SEPARATED_BACKOFF_MINUTE> DULT Separated backoff [minute]
// <i> Default: 360
// <i> DULT Separated backoff (in units of minute.)
#ifndef SL_DULT_CONFIG_SEPARATED_BACKOFF_MINUTE
#define SL_DULT_CONFIG_SEPARATED_BACKOFF_MINUTE 360
#endif

// <o SL_DULT_CONFIG_SEPARATED_TIMEOUT_MIN_MINUTE> DULT Separated timeout min [minute]
// <i> Default: 480
// <i> DULT Separated timeout min (in units of minute.)
#ifndef SL_DULT_CONFIG_SEPARATED_TIMEOUT_MIN_MINUTE
#define SL_DULT_CONFIG_SEPARATED_TIMEOUT_MIN_MINUTE 480
#endif

// <o SL_DULT_CONFIG_SEPARATED_TIMEOUT_MAX_MINUTE> DULT Separated timeout max [minute]
// <i> Default: 1440
// <i> DULT Separated timeout max (in units of minute.)
#ifndef SL_DULT_CONFIG_SEPARATED_TIMEOUT_MAX_MINUTE
#define SL_DULT_CONFIG_SEPARATED_TIMEOUT_MAX_MINUTE 1440
#endif


// </h>

// <h> Advertising rotation interval

// <o SL_DULT_CONFIG_ADV_ROTATION_INTERVAL_NEAR_OWNER_MINUTE> DULT Advertising rotation interval near owner [minute]
// <i> Default: 15
// <i> DULT Advertising rotation interval near owner (in units of minute.)
#ifndef SL_DULT_CONFIG_ADV_ROTATION_INTERVAL_NEAR_OWNER_MINUTE
#define SL_DULT_CONFIG_ADV_ROTATION_INTERVAL_NEAR_OWNER_MINUTE 15
#endif

// <o SL_DULT_CONFIG_ADV_ROTATION_INTERVAL_SEPARATED_MINUTE> DULT Advertising rotation interval separated [minute]
// <i> Default: 1440
// <i> DULT Advertising rotation interval separated (in units of minute.)
#ifndef SL_DULT_CONFIG_ADV_ROTATION_INTERVAL_SEPARATED_MINUTE
#define SL_DULT_CONFIG_ADV_ROTATION_INTERVAL_SEPARATED_MINUTE 1440
#endif

// </h>

// <<< end of configuration section >>>

#endif // SL_DULT_CONFIG_H

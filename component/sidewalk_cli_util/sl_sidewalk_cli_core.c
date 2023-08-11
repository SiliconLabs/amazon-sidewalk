/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_cli_core.c
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "sl_cli.h"
#include "sl_sidewalk_cli_core.h"
#include "sl_sidewalk_cli_util.h"
#include "sl_cmsis_os2_common.h"
#include "sl_sidewalk_cli_settings.h"
#include "sl_sidewalk_cli_util_config.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief App CLI mutex
 *****************************************************************************/
static osMutexId_t cli_mtx;

/**************************************************************************//**
 * @brief App CLI control block
 *****************************************************************************/
__ALIGNED(8) static uint8_t cli_mtx_cb[osMutexCbSize] = { 0 };

/**************************************************************************//**
 * @brief App CLI Mutex attribute
 *****************************************************************************/
static const osMutexAttr_t CLI_MTX_ATTR = {
  .name      = "AppSidewalkCliMutex",
  .attr_bits = osMutexRecursive,
  .cb_mem    = cli_mtx_cb,
  .cb_size   = sizeof(cli_mtx_cb)
};

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief Public app core mutex lock
 *****************************************************************************/
static void cli_mutex_lock(void)
{
  assert(osMutexAcquire(cli_mtx, osWaitForever) == osOK);
}

/**************************************************************************//**
 * @brief Public app core mutex unlock
 *****************************************************************************/
static void cli_mutex_unlock(void)
{
  assert(osMutexRelease(cli_mtx) == osOK);
}

/**************************************************************************//**
 * @brief Init App Sidewalk CLI component
 *****************************************************************************/
void sl_sidewalk_cli_util_init(void)
{
  // Init cli network mutex
  cli_mtx = osMutexNew(&CLI_MTX_ATTR);
  assert(cli_mtx != NULL);

  sl_sidewalk_cli_util_settings_init();
}

/**************************************************************************//**
 * @brief CLI app set
 *****************************************************************************/
void sl_sidewalk_cli_util_set_settings(sl_cli_command_arg_t *arguments)
{
  char *domain_and_key = NULL;
  const char *value = NULL;

  cli_mutex_lock();

  uint8_t argument_count = (uint8_t)sl_cli_get_argument_count(arguments);

  if (argument_count == 0 || (!strcmp(sl_cli_get_argument_string(arguments, 0), "help") && argument_count == 1)) {
    if (sl_sidewalk_cli_util_help(NULL, 0) != SL_STATUS_OK) {
      printf("[Failed: unable to get the set help]\r\n");
    }
  } else if (argument_count == 0 || (!strcmp(sl_cli_get_argument_string(arguments, argument_count - 1), "help"))) {
    domain_and_key = (char *) sl_cli_get_argument_string(arguments, 0);
    value = NULL;
    if (sl_sidewalk_cli_util_help(domain_and_key, 0) != SL_STATUS_OK) {
      printf("[Failed: unable to get the get help of : %s]\r\n", domain_and_key);
    }
  } else {
    if (sl_cli_get_argument_count(arguments) != 2) {
      printf("[Failed: incorrect number of arguments]\r\n");
      cli_mutex_unlock();
      return;
    }

    domain_and_key = (char *) sl_cli_get_argument_string(arguments, 0);
    value = sl_cli_get_argument_string(arguments, 1);

    if (value == NULL || domain_and_key == NULL) {
      printf("[Failed: missing %s]\r\n", value == NULL ? "value" : "key");
      cli_mutex_unlock();
      return;
    }

    if (sl_sidewalk_cli_util_set(domain_and_key, value) != SL_STATUS_OK) {
      printf("[Failed: unable to set the key: %s]\r\n", domain_and_key);
    }

#if (SIDEWALK_CLI_AUTO_SAVE)
    sl_sidewalk_cli_util_save();
#endif
  }

  cli_mutex_unlock();
}

/**************************************************************************//**
 * @brief CLI app get
 *****************************************************************************/
void sl_sidewalk_cli_util_get_settings(sl_cli_command_arg_t *arguments)
{
  char *domain_and_key = NULL;

  cli_mutex_lock();

  uint8_t argument_count = (uint8_t)sl_cli_get_argument_count(arguments);

  if (argument_count == 0 || (!strcmp(sl_cli_get_argument_string(arguments, 0), "help") && argument_count == 1)) {
    if (sl_sidewalk_cli_util_help(NULL, 1) != SL_STATUS_OK) {
      printf("[Failed: unable to get the get help]\r\n");
    }
  } else if (argument_count == 0 || (!strcmp(sl_cli_get_argument_string(arguments, argument_count - 1), "help"))) {
    domain_and_key = (char *) sl_cli_get_argument_string(arguments, 0);
    if (sl_sidewalk_cli_util_help(domain_and_key, 1) != SL_STATUS_OK) {
      printf("[Failed: unable to get the set help of : %s]\r\n", domain_and_key);
    }
  } else {
    if (sl_cli_get_argument_count(arguments) != 1) {
      printf("[Failed: incorrect number of arguments]\r\n");
      cli_mutex_unlock();
      return;
    }

    domain_and_key = (char *) sl_cli_get_argument_string(arguments, 0);
    if (sl_sidewalk_cli_util_get(domain_and_key) != SL_STATUS_OK) {
      printf("[Failed: unable to get the key: %s]\r\n", domain_and_key);
    }
  }

  cli_mutex_unlock();
}

/**************************************************************************//**
 * @brief CLI Save settings
 *****************************************************************************/
void sl_sidewalk_cli_util_save_settings(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  (void)arguments;

  cli_mutex_lock();

  ret = sl_sidewalk_cli_util_save();
  if (ret == SL_STATUS_OK) {
    printf("[Settings saved]\r\n");
  } else {
    printf("[Failed to save settings: %lu]\r\n", ret);
  }

  cli_mutex_unlock();
}

/**************************************************************************//**
 * @brief CLI Reset settings
 *****************************************************************************/
void sl_sidewalk_cli_util_reset_settings(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  cli_mutex_lock();

  sl_sidewalk_cli_util_reset();
  printf("[Settings reset]\r\n");

  cli_mutex_unlock();
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

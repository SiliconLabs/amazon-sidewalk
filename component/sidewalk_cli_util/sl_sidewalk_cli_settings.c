/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_cli_settings.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sl_sidewalk_cli_settings.h"
#include "nvm3.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief  NVM key base address define
 *****************************************************************************/
#define SL_APP_SETTINGS_NVM_KEY_BASE    (0x00000000U)

/**************************************************************************//**
 * @brief App settings None value string
 *****************************************************************************/
#define SL_APP_SETTINGS_NONE_VALUE_STR  "None"

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief Load settings from NVM
 * @details Copy settings from the nvm3 memory to the destination
 * @param[in] settings_domain settings domain
 * @param[in,out] settings destination ptr of settings
 * @param[in] settings_size size of the settings
 * @return sl_status_t SL_STATUS_OK on succes, SL_STATUS_FAIL on error
 *****************************************************************************/
static sl_status_t settings_nvm_load(uint8_t settings_domain,
                                            void *const settings,
                                            size_t settings_size);

/**************************************************************************//**
* @brief Save settings to NVM
* @details Save the settings into nvm3 memory
* @param[in] settings_domain settings domain
* @param[in] settings source settings
* @param[in] settings_size size of the settings
* @return sl_status_t SL_STATUS_OK on succes, SL_STATUS_FAIL on error
******************************************************************************/
static sl_status_t settings_nvm_save(uint8_t settings_domain,
                                            const void *const settings,
                                            size_t settings_size);

/**************************************************************************//**
 * @brief Delete settings from NVM
 * @details Remove settings from nvm3 memory
 * @param[in] settings_domain settings domain
 *****************************************************************************/
static void settings_nvm_delete(uint8_t settings_domain);

/**************************************************************************//**
 * @brief App help print and pad
 * @details Print help informations on 2 columns
 * @param permission Command permission
 * @param entry App settings entry
 *****************************************************************************/
static void help_print_and_pad(const char *permission, const sl_sidewalk_cli_util_entry_t *entry);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief App settings domain string array (weak implementation)
 *****************************************************************************/
SL_WEAK const char *app_settings_domain_str[] = { "" };

/**************************************************************************//**
 * @brief App settings entries array (weak implementation)
 *****************************************************************************/
SL_WEAK const sl_sidewalk_cli_util_entry_t app_settings_entries[] = { 0 };

/**************************************************************************//**
 * @brief Saveing settings array (weak implementation)
 *****************************************************************************/
SL_WEAK const sl_sidewalk_cli_util_saving_item_t *saving_settings[] = { 0 };

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief App settings init
 * Initialize settings to default value
 *****************************************************************************/
void sl_sidewalk_cli_util_settings_init(void)
{
  uint8_t index = 0;
  sl_status_t ret = SL_STATUS_OK;

  // for each element of settings table, load from nvm3
  while (saving_settings[index]) {
    ret = settings_nvm_load(index,
                                   saving_settings[index]->data,
                                   saving_settings[index]->data_size);
    if (ret != SL_STATUS_OK) {
      // if load did not succeed, set to default value
      if (saving_settings[index]->default_val) {
        memcpy(saving_settings[index]->data, saving_settings[index]->default_val, saving_settings[index]->data_size);
      }
    }
    index++;
  }
}

/**************************************************************************//**
 * @brief App setinngs save
 * Save settings value into nvm
 *****************************************************************************/
sl_status_t sl_sidewalk_cli_util_save(void)
{
  uint8_t index = 0;
  sl_status_t ret = SL_STATUS_OK;

  while (saving_settings[index]) {
    ret = settings_nvm_save(index,
                                   saving_settings[index]->data,
                                   saving_settings[index]->data_size);
    if (ret != SL_STATUS_OK) {
      break;
    }
    index++;
  }

  return ret;
}

/**************************************************************************//**
 * @brief App settings reset
 * Reset setting to default value
 *****************************************************************************/
void sl_sidewalk_cli_util_reset(void)
{
  uint8_t index = 0;

  while (saving_settings[index]) {
    settings_nvm_delete(index);
    if (saving_settings[index]->default_val) {
      memcpy(saving_settings[index]->data, saving_settings[index]->default_val, saving_settings[index]->data_size);
    }
    index++;
  }
}

/**************************************************************************//**
 * @brief App settings set
 * Set a setting to a given value.
 *****************************************************************************/
sl_status_t sl_sidewalk_cli_util_set(char *const domain_and_key, const char *const value_str)
{
  uint8_t index = 0;
  const char *domain = NULL;
  char *key = NULL;
  const char *nested_key = NULL;

  if (!domain_and_key || !value_str) {
    return SL_STATUS_INVALID_KEY;
  }

  key = domain_and_key;
  domain = strtok(key, ".");
  key = strtok(NULL, ".");
  nested_key = strtok(NULL, ".");

  if (!domain || !key) {
    return SL_STATUS_INVALID_KEY;
  }

 while(app_settings_entries[index].key) {
    if (!strcmp(domain, app_settings_domain_str[app_settings_entries[index].domain])) {
      if (!strcmp(app_settings_entries[index].key, key)) {
        if (app_settings_entries[index].set_handler) {
          printf("%s.%s = %s\r\n", app_settings_domain_str[app_settings_entries[index].domain], app_settings_entries[index].key, value_str);
          return app_settings_entries[index].set_handler(value_str, nested_key, &app_settings_entries[index]);
        } else {
          return SL_STATUS_PERMISSION;
        }
      }
    }
    index++;
  }

  return SL_STATUS_INVALID_KEY;
}

/**************************************************************************//**
 * @brief App settings get
 * Get setting reference given a domain and key
 *****************************************************************************/
sl_status_t sl_sidewalk_cli_util_get(char *const domain_and_key)
{
  sl_status_t ret;
  uint8_t index = 0;
  const char *domain = NULL;
  char *key = NULL;
  const char *nested_key = NULL;
  char value_str[128];

  key = domain_and_key;
  domain = strtok(key, ".");
  key = strtok(NULL, ".");
  nested_key = strtok(NULL, ".");

  while (app_settings_entries[index].key){
    if (!domain || !strcmp(domain, app_settings_domain_str[app_settings_entries[index].domain])) {
      if (!key || !strcmp(app_settings_entries[index].key, key)) {
        if (app_settings_entries[index].get_handler) {
          ret = app_settings_entries[index].get_handler(value_str, nested_key, &app_settings_entries[index]);
          if (ret == SL_STATUS_OK) {
            printf("%s.%s = %s\r\n", app_settings_domain_str[app_settings_entries[index].domain], app_settings_entries[index].key, value_str);
          }
        }
      }
    }
    index++;
  }

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * @brief App settings help
 * Get help for given setting
 *****************************************************************************/
sl_status_t sl_sidewalk_cli_util_help(char *const domain_and_key, bool get)
{
  uint8_t index = 0;
  const char *domain = NULL;
  char *key = NULL;

  uint8_t permission_write = 0x01;
  uint8_t permission_read = 0x02;

  const char *permission_str[] =
  {
    "[??]",
    "[wo]",
    "[ro]",
    "[rw]"
  };

  key = domain_and_key;
  domain = strtok(key, ".");
  key = strtok(NULL, ".");

  if (!domain) {
    printf("Help of get and set methods\r\n\r\nAvailable domains :\r\n");
    uint8_t domains_nb = 0;
    while (app_settings_domain_str[domains_nb] != NULL) {
      printf(" %s\r\n", app_settings_domain_str[domains_nb]);
      domains_nb++;
    }
    printf("Type '[get or set] domain help'\r\n");
    printf("\r\neg. 'get %s help' to get the help of %s\r\n    'get %s' for all 'get' commands\r\n",
           app_settings_domain_str[0], app_settings_domain_str[0], app_settings_domain_str[0]);
    printf("\r\nCommands permissions :\r\n %s : Write Only\r\n %s : Read Only\r\n %s : Read and Write\r\n",
           permission_str[1], permission_str[2], permission_str[3]);
  } else if (domain) {
    while (app_settings_entries[index].key) {
      if (!domain || !strcmp(domain, app_settings_domain_str[app_settings_entries[index].domain])) {
        if (!key || !strcmp(app_settings_entries[index].key, key)) {
          uint8_t permission = 0;
          permission |= (app_settings_entries[index].get_handler ? permission_read : 0);
          permission |= (app_settings_entries[index].set_handler ? permission_write : 0);
          if ((app_settings_entries[index].get_handler && get) || (app_settings_entries[index].set_handler && !get)) {
            help_print_and_pad(permission_str[permission], &app_settings_entries[index]);
          }
        }
      }
      index++;
    }
  }
  return SL_STATUS_OK;
}

/**************************************************************************//**
 * @brief App settings set string
 * Set a string type setting to given str
 *****************************************************************************/
sl_status_t sl_sidewalk_cli_util_set_string(const char *value_str,
                                       const char *key_str,
                                       const sl_sidewalk_cli_util_entry_t *entry)
{
  char * entry_value_str;
  (void)key_str;

  entry_value_str = entry->value;
  strncpy(entry_value_str, value_str, entry->value_size - 1);
  entry_value_str[entry->value_size - 1] = '\0';

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * @brief App settings get string
 * Get the string value of a given entry string setting
 *****************************************************************************/
sl_status_t sl_sidewalk_cli_util_settings_get_string(char *value_str,
                                       const char *key_str,
                                       const sl_sidewalk_cli_util_entry_t *entry)
{
  (void)key_str;

  sprintf(value_str, "\"%s\"", (const char *)entry->value);

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * @brief App settings set integer
 * Set a integer type setting given a string value
 *****************************************************************************/
sl_status_t sl_sidewalk_cli_util_set_integer(const char *value_str,
                                        const char *key_str,
                                        const sl_sidewalk_cli_util_entry_t *entry)
{
  uint32_t value;
  sl_status_t ret;
  (void)key_str;

  ret = sl_sidewalk_cli_util_get_integer(&value,
                                value_str,
                                entry->input_enum_list,
                                entry->input & SL_APP_SETTINGS_INPUT_FLAG_SIGNED);
  if (ret != SL_STATUS_OK) {
    return SL_STATUS_INVALID_TYPE;
  }

  switch (entry->value_size) {
    case SL_APP_SETTINGS_VALUE_SIZE_UINT8:
      *((uint8_t *)entry->value) = (uint8_t)value;
      break;

    case SL_APP_SETTINGS_VALUE_SIZE_UINT16:
      *((uint16_t *)entry->value) = (uint16_t)value;
      break;

    case SL_APP_SETTINGS_VALUE_SIZE_UINT32:
      *((uint32_t *)entry->value) = value;
      break;

    default:
      // Unsupported integer size
      return SL_STATUS_INVALID_TYPE;
  }

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * @brief App settings get integer
 * Get string of integer setting entry
 *****************************************************************************/
sl_status_t sl_sidewalk_cli_util_settings_get_integer(char *value_str,
                                        const char *key_str,
                                        const sl_sidewalk_cli_util_entry_t *entry)
{
  uint32_t value;
  uint8_t value_length = 0;
  (void)key_str;

  switch (entry->value_size) {
    case SL_APP_SETTINGS_VALUE_SIZE_UINT8:
      value = *((uint8_t *)entry->value);
      if ((entry->output & SL_APP_SETTINGS_OUTPUT_FLAG_SIGNED)
          && (value & 0x80)) {
        value += 0xFFFFFF00;
      }
      break;

    case SL_APP_SETTINGS_VALUE_SIZE_UINT16:
      value = *((uint16_t *)entry->value);
      if ((entry->output & SL_APP_SETTINGS_OUTPUT_FLAG_SIGNED)
          && (value & 0x8000)) {
        value += 0xFFFF0000;
      }
      break;

    case SL_APP_SETTINGS_VALUE_SIZE_UINT32:
      value = *((uint32_t *)entry->value);
      break;

    default:
      // Unsupported integer size
      return SL_STATUS_INVALID_TYPE;
  }

  if (entry->output & SL_APP_SETTINGS_OUTPUT_FLAG_FIXEDSIZE) {
    if (entry->output & SL_APP_SETTINGS_OUTPUT_FLAG_HEX) {
      value_length = entry->value_size * 2;
    }
  }

  return sl_sidewalk_cli_util_get_string(value_str,
                                value,
                                entry->output_enum_list,
                                entry->output & SL_APP_SETTINGS_OUTPUT_FLAG_SIGNED,
                                entry->output & SL_APP_SETTINGS_OUTPUT_FLAG_HEX,
                                value_length);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief App help print and pad
 *****************************************************************************/
static void help_print_and_pad(const char *permission, const sl_sidewalk_cli_util_entry_t *entry)
{
  printf("%s %s.%s", permission, app_settings_domain_str[entry->domain], entry->key);
  size_t string_length = strlen(permission) + strlen(app_settings_domain_str[entry->domain]) + strlen(entry->key);
  for (; string_length < 60; string_length++) {
    printf(" ");
  }
  printf("%s\r\n", entry->description ? entry->description : " ");
}

/**************************************************************************//**
 * @brief App settings nvm load
 *****************************************************************************/
static sl_status_t settings_nvm_load(uint8_t settings_domain,
                                            void *const settings,
                                            size_t settings_size)
{
  nvm3_ObjectKey_t nvm_key;
  uint32_t nvm_type;
  size_t nvm_size;
  Ecode_t ret;

  nvm_key = SL_APP_SETTINGS_NVM_KEY_BASE + settings_domain;
  ret = nvm3_getObjectInfo(nvm3_defaultHandle,
                           nvm_key,
                           &nvm_type,
                           &nvm_size);
  if (ret != ECODE_NVM3_OK) {
    return SL_STATUS_FAIL;
  }

  if (nvm_size != settings_size) {
    return SL_STATUS_FAIL;
  }

  ret = nvm3_readData(nvm3_defaultHandle,
                      nvm_key,
                      settings,
                      nvm_size);
  if (ret != ECODE_NVM3_OK) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * @brief App settings NVM save
 *****************************************************************************/
static sl_status_t settings_nvm_save(uint8_t settings_domain,
                                            const void *const settings,
                                            size_t settings_size)
{
  nvm3_ObjectKey_t nvm_key;
  Ecode_t ret;

  nvm_key = SL_APP_SETTINGS_NVM_KEY_BASE + settings_domain;
  ret = nvm3_writeData(nvm3_defaultHandle,
                       nvm_key,
                       settings,
                       settings_size);
  if (ret != ECODE_NVM3_OK) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * @brief App settings NVM delete
 *****************************************************************************/
static void settings_nvm_delete(uint8_t settings_domain)
{
  nvm3_ObjectKey_t nvm_key;

  nvm_key = SL_APP_SETTINGS_NVM_KEY_BASE + settings_domain;
  (void)nvm3_deleteObject(nvm3_defaultHandle, nvm_key);
}


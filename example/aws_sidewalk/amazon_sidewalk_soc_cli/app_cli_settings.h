/***************************************************************************//**
 * @file app_cli_settings.h
 * @brief Application settings handler
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef APP_CLI_SETTINGS_H
#define APP_CLI_SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

#include "sid_api.h"
#include "sid_900_cfg.h"
#include "sl_sidewalk_cli_settings.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define LINK_STRING_SIZE          (20U)
#define REGION_STRING_SIZE        (20U)
#define STATE_STRING_SIZE         (20U)
#define TIME_STRING_SIZE          (30U)
#define MAC_STRING_SIZE           (30U)
#define AUTOCONNECT_STRING_SIZE   (20U)
#define FREQ_STRING_SIZE          (20U)
#define MTU_STRING_SIZE           (10U)
#define DEVICE_PROFILE_ID_SIZE    (5U)
#define POWER_PROFILE_SIZE        (1U)

typedef enum {
  app_settings_domain_sidewalk      = 0x00,
  app_settings_domain_radio         = 0x01,
// Below is the definition of enumeration value depending on all link availability
// P = 2 ^ N of link = 2 ^ 3 = 8 possibility
// Order is important (eg. ble -> fsk -> css).

// 0 0 0
#if !defined(SL_BLE_SUPPORTED) && !defined(SL_FSK_SUPPORTED) && !defined(SL_CSS_SUPPORTED)
  #error "At least one link must be available!"
// 0 0 1
#elif !defined(SL_BLE_SUPPORTED) && !defined(SL_FSK_SUPPORTED) && defined(SL_CSS_SUPPORTED)
  app_settings_domain_css           = 0x02,
// 0 1 0
#elif !defined(SL_BLE_SUPPORTED) && defined(SL_FSK_SUPPORTED) && !defined(SL_CSS_SUPPORTED)
  app_settings_domain_fsk           = 0x02,
// 0 1 1
#elif defined(SL_FSK_SUPPORTED) && defined(SL_CSS_SUPPORTED) && !defined(SL_BLE_SUPPORTED)
  app_settings_domain_fsk           = 0x02,
  app_settings_domain_css           = 0x03,
// 1 0 0
#elif defined(SL_BLE_SUPPORTED) && !defined(SL_FSK_SUPPORTED) && !defined(SL_CSS_SUPPORTED)
  app_settings_domain_ble           = 0x02,
// 1 0 1
#elif defined(SL_BLE_SUPPORTED) && !defined(SL_FSK_SUPPORTED) && defined(SL_CSS_SUPPORTED)
  app_settings_domain_ble           = 0x02,
  app_settings_domain_css           = 0x03,
// 1 1 0
#elif defined(SL_BLE_SUPPORTED) && defined(SL_FSK_SUPPORTED) && !defined(SL_CSS_SUPPORTED)
  app_settings_domain_ble           = 0x02,
  app_settings_domain_fsk           = 0x03,
// 1 1 1
#elif defined(SL_BLE_SUPPORTED) && defined(SL_FSK_SUPPORTED) && defined(SL_FSK_SUPPORTED)
  app_settings_domain_ble           = 0x02,
  app_settings_domain_fsk           = 0x03,
  app_settings_domain_css           = 0x04,
#endif
} app_settings_domain_t;

extern const char *app_settings_domain_str[];
extern const sl_sidewalk_cli_util_saving_item_t app_saving_item_sidewalk;
extern const sl_sidewalk_cli_util_saving_item_t app_saving_item_radio;
#if defined(SL_BLE_SUPPORTED)
extern const sl_sidewalk_cli_util_saving_item_t app_saving_item_ble;
#endif
#if defined(SL_FSK_SUPPORTED)
extern const sl_sidewalk_cli_util_saving_item_t app_saving_item_fsk;
#endif
#if defined(SL_CSS_SUPPORTED)
extern const sl_sidewalk_cli_util_saving_item_t app_saving_item_css;
#endif
extern const sl_sidewalk_cli_util_saving_item_t *saving_settings[];
extern const sl_sidewalk_cli_util_entry_t app_settings_entries[];

// Struct of data that update cli parameters on cli event requests
typedef struct {
  struct sid_timespec current_time;
  struct sid_status current_status;
  struct sid_device_profile device_profile;
  size_t mtu;
} app_setting_cli_queue_t;

typedef struct {
  char initialized_link[LINK_STRING_SIZE + 1];
  char started_link[LINK_STRING_SIZE + 1];
  char region[REGION_STRING_SIZE + 1];
  char state[STATE_STRING_SIZE + 1];
  char time[TIME_STRING_SIZE + 1];
  char auto_connect[AUTOCONNECT_STRING_SIZE + 1];
} app_settings_sidewalk_t;

typedef struct {
  uint8_t wakeup_type;
  int8_t rssi;
  int8_t snr;
} app_settings_radio_t;

#if defined(SL_BLE_SUPPORTED)
typedef struct {
  size_t mtu;
  char random_mac[MAC_STRING_SIZE + 1];
  uint16_t output_power;
} app_settings_ble_t;

extern app_settings_ble_t app_settings_ble;
#endif

#if defined(SL_FSK_SUPPORTED)
typedef struct {
  char data_rate[FREQ_STRING_SIZE + 1];
  char min_freq[FREQ_STRING_SIZE + 1];
  char max_freq[FREQ_STRING_SIZE + 1];
  char region[REGION_STRING_SIZE + 1];
  char power_profile[POWER_PROFILE_SIZE + 1];
  size_t mtu;
  uint8_t offset;
  uint8_t range;
  uint8_t bcn_interval;
  uint16_t rx_window_count;
  uint32_t rx_window_separation;
  uint32_t rx_window_duration;
} app_settings_fsk_t;

extern app_settings_fsk_t app_settings_fsk;
#endif

#if defined(SL_CSS_SUPPORTED)
typedef struct {
  char bandwidth[FREQ_STRING_SIZE + 1];
  char min_freq[FREQ_STRING_SIZE + 1];
  char max_freq[FREQ_STRING_SIZE + 1];
  char region[REGION_STRING_SIZE + 1];
  char power_profile[POWER_PROFILE_SIZE + 1];
  size_t mtu;
  uint8_t rx_window_separation; // In seconds
  uint8_t rx_window_count;
} app_settings_css_t;

extern app_settings_css_t app_settings_css;
#endif

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

extern app_settings_sidewalk_t app_settings_sidewalk;
extern app_settings_radio_t app_settings_radio;

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/*******************************************************************************
 * sl_app_settings_get_initialized_link
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_initialized_link(char *value_str,
                                                 const char *key_str,
                                                 const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_started_link
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_started_link(char *value_str,
                                             const char *key_str,
                                             const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_region
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_region(char *value_str,
                                       const char *key_str,
                                       const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_set_region
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_set_region(const char *value_str,
                                       const char *key_str,
                                       const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_state
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_state(char *value_str,
                                      const char *key_str,
                                      const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_time
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_time(char *value_str,
                                     const char *key_str,
                                     const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_autoconnect
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_autoconnect(char *value_str,
                                            const char *key_str,
                                            const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_set_autoconnect
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_set_autoconnect(const char *value_str,
                                            const char *key_str,
                                            const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_frequency
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_frequency(char *value_str,
                                          const char *key_str,
                                          const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_set_wakeup_type
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_set_wakeup_type(const char *value_str,
                                            const char *key_str,
                                            const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_wakeup_type
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_wakeup_type(char *value_str,
                                            const char *key_str,
                                            const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_rssi
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_rssi(char *value_str,
                                     const char *key_str,
                                     const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_snr
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_snr(char *value_str,
                                    const char *key_str,
                                    const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_mtu_ble
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_mtu_ble(char *value_str,
                                        const char *key_str,
                                        const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_mtu_fsk
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_mtu_fsk(char *value_str,
                                        const char *key_str,
                                        const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_mtu_css
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_mtu_css(char *value_str,
                                        const char *key_str,
                                        const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_random_mac
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_random_mac(char *value_str,
                                           const char *key_str,
                                           const sl_sidewalk_cli_util_entry_t *entry);;

/*******************************************************************************
 * sl_app_settings_get_output_power
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_output_power(char *value_str,
                                             const char *key_str,
                                             const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_set_fsk_power_profile
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_set_fsk_power_profile(const char *value_str,
                                                  const char *key_str,
                                                  const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_fsk_power_profile
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_fsk_power_profile(char *value_str,
                                                  const char *key_str,
                                                  const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_set_css_power_profile
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_set_css_power_profile(const char *value_str,
                                                  const char *key_str,
                                                  const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_css_power_profile
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_css_power_profile(char *value_str,
                                                  const char *key_str,
                                                  const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_set_offset
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_set_offset(const char *value_str,
                                       const char *key_str,
                                       const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_offset
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_offset(char *value_str,
                                       const char *key_str,
                                       const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_set_range
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_set_range(const char *value_str,
                                      const char *key_str,
                                      const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_range
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_range(char *value_str,
                                      const char *key_str,
                                      const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_beacon_interval
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_beacon_interval(char *value_str,
                                                const char *key_str,
                                                const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_rx_window_count
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_fsk_rx_window_count(char *value_str,
                                                    const char *key_str,
                                                    const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_rx_window_separation
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_fsk_rx_window_separation(char *value_str,
                                                         const char *key_str,
                                                         const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_rx_window_separation
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_css_rx_window_separation(char *value_str,
                                                         const char *key_str,
                                                         const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_get_rx_window_count
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_get_css_rx_window_count(char *value_str,
                                                    const char *key_str,
                                                    const sl_sidewalk_cli_util_entry_t *entry);

/*******************************************************************************
 * sl_app_settings_set_rx_window_count
 *
 * @param[in] value_str
 * @param[in] key_str
 * @param[in] entry
 * @returns sl_status_t
 ******************************************************************************/
sl_status_t sl_app_settings_set_css_rx_window_count(const char *value_str,
                                                    const char *key_str,
                                                    const sl_sidewalk_cli_util_entry_t *entry);

#ifdef __cplusplus
}
#endif

#endif  // APP_CLI_SETTINGS_H

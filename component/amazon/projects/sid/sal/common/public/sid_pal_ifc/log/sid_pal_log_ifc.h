/*
 * Copyright 2020-2023 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_LOG_IFC_H
#define SID_PAL_LOG_IFC_H

/**
 * \addtogroup sid_ifc
 * @{
 */
/**
 * \addtogroup sid_pal_ifc
 * @{
 */
/**
 * \addtogroup sid_pal_log_ifc
 * @{
 */

#include <stdint.h>
#include <stdbool.h>
#include "sl_component_catalog.h"
#if defined(SL_CATALOG_SIDEWALK_LOG_STACK_RTT_PRESENT)
#include "sl_sidewalk_log_stack_rtt_config.h"
#elif defined(SL_CATALOG_SIDEWALK_LOG_STACK_VCOM_PRESENT)
#include "sl_sidewalk_log_stack_vcom_config.h"
#else
#if !defined(SL_SIDEWALK_UNIT_TEST)
#error "No logging interface defined for stack layer"
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * @addtogroup sid_pal_log_ifc_types Type definitions
 * @ingroup sid_pal_log_ifc
 * @{
 *****************************************************************************/
/**
 * @brief Log severity levels.
 *
 * This enum defines the severity levels for logging messages.
 */
typedef enum
{
  SID_PAL_LOG_SEVERITY_ERROR = 0,   /*!< Error severity level */
  SID_PAL_LOG_SEVERITY_WARNING = 1, /*!< Warning severity level */
  SID_PAL_LOG_SEVERITY_INFO = 2,    /*!< Info severity level */
  SID_PAL_LOG_SEVERITY_DEBUG = 3    /*!< Debug severity level */
} sid_pal_log_severity_t;

/** @} (end sid_pal_log_ifc_types) */

/**
 * @brief Printf style logging function
 *
 * @param[in]   severity        Severity of the log
 * @param[in]   num_args        Number of arguments to be logged
 * @param[in]   fmt             Format string to print with variables
 */
void sid_pal_log(sid_pal_log_severity_t severity, uint32_t num_args, const char* fmt, ...);

/**
 * @brief The function returns current logging level. Implemented in sid_log_control
 *
 * @retval log severity level
 *
 */
sid_pal_log_severity_t sid_log_control_get_current_log_level(void);

/**
 * @brief Flush log function
 *
 * The function flushes the log buffers to the output interface
 */
void sid_pal_log_flush(void);

/**
 * @brief String log pushing operation - in the case of JLink RTT logs since these are deferred,
 * they require that a special function be used in order to ensure that strings are
 * copied correctly in order for pushing then at a later time.
 *
 * For platforms that do not use deferred logging, this can remain unimplemented.
 *
 * @param[in] string  Pointer to the string that has to be copied.
 * @retval string pointer to the string that has to be copied
 */
char const *sid_pal_log_push_str(char *string);

/**
 * @brief Describes the log buffer that is retrieved from sid_pal_log_ifc implementation
 */
struct sid_pal_log_buffer {
    /** Raw buffer that the sid_pal_log implementation will copy the log string
     * into
     **/
    uint8_t *buf;
    /** Size of the above Raw buffer, @ref sid_pal_log_get_log_buffer will replace
     * the size value with the actual size of the log string that was copied in
     * bytes
     */
    uint8_t size;
    /** Index of log string
     */
    uint8_t idx;
};

/**
 * @brief Allows for retrival of log buffers from sid_pal_log implementation
 *
 * OPTIONAL If business logic wants to send logs over an external connection
 * they can use this api to pull out logs from sid_pal_log implementation if
 * implemented.
 * Internally the implementation can use a circular list of buffers to store
 * the logs as they come.
 *
 * @param[in] log_buffer Pointer to log buffer descriptor
 *
 * @retval true if log strings were copied into log_buffer, false otherwise
 */
bool sid_pal_log_get_log_buffer(struct sid_pal_log_buffer *const log_buffer);

/**
 * @def SID_PAL_HEXDUMP_MAX
 * @brief Define the maximum number of bytes per single line of sid_pal_hexdump() logging.
 * When the caller requests more bytes then the output will be split into multiple lines
 * using SID_PAL_HEXDUMP_MAX bytes per line, plus eventual remainder in the last line.
 */
#define SID_PAL_HEXDUMP_MAX (8)

/**
 * @brief Log raw data bytes.
 *
 * @param[in] severity Severity of the log
 * @param[in] address Pointer to data to be logged
 * @param[in] length The length of data to be logged (in bytes)
 */
void sid_pal_hexdump(sid_pal_log_severity_t severity, const void *address, int length);

/**
 * @def SID_PAL_VA_NARG
 * @brief Macro to count the number of arguments in a variadic macro.
 *
 * This macro counts the number of arguments passed to it. It uses a combination
 * of helper macros to achieve this.
 *
 * @param[in] ...  Variadic arguments.
 * @return The number of arguments passed.
 */
#define SID_PAL_VA_NARG(...) \
        (SID_PAL_VA_NARG_(_0, ## __VA_ARGS__, SID_PAL_RSEQ_N()))
/**
 * @def SID_PAL_VA_NARG_
 * @brief Helper macro to count the number of arguments in a variadic macro.
 *
 * This macro is used internally by SID_PAL_VA_NARG to count the number of arguments.
 *
 * @param[in] ...  Variadic arguments.
 * @return The number of arguments passed.
 */
#define SID_PAL_VA_NARG_(...) \
        SID_PAL_VA_ARG_N(__VA_ARGS__)
/**
 * @def SID_PAL_VA_ARG_N
 * @brief Helper macro to extract the number of arguments.
 *
 * This macro extracts the number of arguments from the list of arguments.
 *
 * @param[in] _1 First argument.
 * @param[in] _2 Second argument.
 * @param[in] _3 Third argument.
 * @param[in] _4 Fourth argument.
 * @param[in] _5 Fifth argument.
 * @param[in] _6 Sixth argument.
 * @param[in] _7 Seventh argument.
 * @param[in] _8 Eighth argument.
 * @param[in] _9 Ninth argument.
 * @param[in] _10 Tenth argument.
 * @param[in] _11 Eleventh argument.
 * @param[in] _12 Twelfth argument.
 * @param[in] _13 Thirteenth argument.
 * @param[in] _14 Fourteenth argument.
 * @param[in] _15 Fifteenth argument.
 * @param[in] _16 Sixteenth argument.
 * @param[in] _17 Seventeenth argument.
 * @param[in] _18 Eighteenth argument.
 * @param[in] _19 Nineteenth argument.
 * @param[in] _20 Twentieth argument.
 * @param[in] _21 Twenty-first argument.
 * @param[in] _22 Twenty-second argument.
 * @param[in] _23 Twenty-third argument.
 * @param[in] _24 Twenty-fourth argument.
 * @param[in] _25 Twenty-fifth argument.
 * @param[in] _26 Twenty-sixth argument.
 * @param[in] _27 Twenty-seventh argument.
 * @param[in] _28 Twenty-eighth argument.
 * @param[in] _29 Twenty-ninth argument.
 * @param[in] _30 Thirtieth argument.
 * @param[in] _31 Thirty-first argument.
 * @param[in] _32 Thirty-second argument.
 * @param[in] _33 Thirty-third argument.
 * @param[in] _34 Thirty-fourth argument.
 * @param[in] _35 Thirty-fifth argument.
 * @param[in] _36 Thirty-sixth argument.
 * @param[in] _37 Thirty-seventh argument.
 * @param[in] _38 Thirty-eighth argument.
 * @param[in] _39 Thirty-ninth argument.
 * @param[in] _40 Fortieth argument.
 * @param[in] _41 Forty-first argument.
 * @param[in] _42 Forty-second argument.
 * @param[in] _43 Forty-third argument.
 * @param[in] _44 Forty-fourth argument.
 * @param[in] _45 Forty-fifth argument.
 * @param[in] _46 Forty-sixth argument.
 * @param[in] _47 Forty-seventh argument.
 * @param[in] _48 Forty-eighth argument.
 * @param[in] _49 Forty-ninth argument.
 * @param[in] _50 Fiftieth argument.
 * @param[in] _51 Fifty-first argument.
 * @param[in] _52 Fifty-second argument.
 * @param[in] _53 Fifty-third argument.
 * @param[in] _54 Fifty-fourth argument.
 * @param[in] _55 Fifty-fifth argument.
 * @param[in] _56 Fifty-sixth argument.
 * @param[in] _57 Fifty-seventh argument.
 * @param[in] _58 Fifty-eighth argument.
 * @param[in] _59 Fifty-ninth argument.
 * @param[in] _60 Sixtieth argument.
 * @param[in] _61 Sixty-first argument.
 * @param[in] _62 Sixty-second argument.
 * @param[in] _63 Sixty-third argument.
 * @param[in] N The number of arguments passed.
 * @return The number of arguments passed.
 */
#define SID_PAL_VA_ARG_N( \
         _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
        _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
        _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
        _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
        _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
        _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
        _61,_62,_63,N,...) N
/**
 * @def SID_PAL_RSEQ_N
 * @brief Macro to define a reverse sequence of numbers from 62 to 0.
 *
 * This macro generates a sequence of numbers in descending order starting from 62 down to 0.
 * It can be used in various contexts where such a sequence is required.
 */
#define SID_PAL_RSEQ_N() \
        62, 61, 60,                             \
        59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
        49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
        39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
        29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
        19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
         9,  8,  7,  6,  5,  4,  3,  2,  1,  0

#if SID_PAL_LOG_ENABLED
/**
 * @def SID_PAL_LOG_FLUSH
 * @brief Flushes the log buffer.
 *
 * This macro calls the function to flush the log buffer.
 */
#define SID_PAL_LOG_FLUSH()  sid_pal_log_flush()
/**
 * @def SID_PAL_LOG_PUSH_STR
 * @brief Pushes a string to the log.
 *
 * @param x The string to be pushed to the log.
 */
#define SID_PAL_LOG_PUSH_STR(x)  sid_pal_log_push_str(x)
/**
 * @def SID_PAL_LOG_HIGHEST_SEVIRITY
 * @brief Logs a message with the highest severity level.
 *
 * @param level The severity level of the log message.
 * @param fmt_ The format string for the log message.
 * @param ... The arguments for the format string.
 */
#define SID_PAL_LOG_HIGHEST_SEVIRITY(level, fmt_, ...) \
            sid_pal_log(level, SID_PAL_VA_NARG(__VA_ARGS__), fmt_, ##__VA_ARGS__);
/**
 * @def SID_PAL_LOG
 * @brief Logs a message if the severity level is less than or equal to the configured log level.
 *
 * @param level The severity level of the log message.
 * @param fmt_ The format string for the log message.
 * @param ... The arguments for the format string.
 */
#define SID_PAL_LOG(level, fmt_, ...)                                                           \
    do {                                                                                        \
        if (level <= SID_PAL_LOG_LEVEL)  {  \
            sid_pal_log(level, SID_PAL_VA_NARG(__VA_ARGS__), fmt_, ##__VA_ARGS__);              \
        }                                                                                       \
    } while(0)
/**
 * @def SID_PAL_HEXDUMP
 * @brief Dumps a block of data in hexadecimal format.
 *
 * @param level_ The severity level of the log message.
 * @param data_ The data to be dumped.
 * @param len_ The length of the data to be dumped.
 */
#define SID_PAL_HEXDUMP(level_, data_, len_) sid_pal_hexdump(level_, data_, len_)

#else
/**
 * @def SID_PAL_LOG_HIGHEST_SEVIRITY
 * @brief Logs a message with the highest severity level.
 *
 * @param level The severity level of the log message.
 * @param fmt_ The format string for the log message.
 * @param ... The arguments for the format string.
 */
#define SID_PAL_LOG_HIGHEST_SEVIRITY(level, fmt_, ...)
/**
 * @def SID_PAL_LOG
 * @brief Logs a message if the severity level is less than or equal to the configured log level.
 *
 * @param level_ The severity level of the log message.
 * @param fmt_ The format string for the log message.
 * @param ... The arguments for the format string.
 */
#define SID_PAL_LOG(level_, fmt_, ...)
/**
 * @def SID_PAL_HEXDUMP
 * @brief Dumps a block of data in hexadecimal format.
 *
 * @param level_ The severity level of the log message.
 * @param data_ The data to be dumped.
 * @param len_ The length of the data to be dumped.
 */
#define SID_PAL_HEXDUMP(level_, data_, len_)
/**
 * @def SID_PAL_LOG_FLUSH
 * @brief Flushes the log buffer.
 *
 * This macro calls the function to flush the log buffer.
 */
#define SID_PAL_LOG_FLUSH()
/**
 * @def SID_PAL_LOG_PUSH_STR
 * @brief Pushes a string to the log.
 *
 * @param x The string to be pushed to the log.
 */
#define SID_PAL_LOG_PUSH_STR(x) (x)
#endif


/* Logging helpers to simplify logging APIs */
/**
 * @def SID_PAL_LOG_ERROR
 * @brief Logs an error message with the highest severity level.
 *
 * @param fmt_ The format string for the log message.
 * @param ... The arguments for the format string.
 */
#define SID_PAL_LOG_ERROR(fmt_, ...)           SID_PAL_LOG_HIGHEST_SEVIRITY(SID_PAL_LOG_SEVERITY_ERROR, fmt_, ##__VA_ARGS__)
#if SID_PAL_LOG_LEVEL >= SID_PAL_LOG_SEVERITY_WARNING
/**
 * @def SID_PAL_LOG_WARNING
 * @brief Logs a warning message if the severity level is greater than or equal to the warning level.
 *
 * @param fmt_ The format string for the log message.
 * @param ... The arguments for the format string.
 */
#define SID_PAL_LOG_WARNING(fmt_, ...)         SID_PAL_LOG(SID_PAL_LOG_SEVERITY_WARNING, fmt_, ##__VA_ARGS__)
#if SID_PAL_LOG_LEVEL >= SID_PAL_LOG_SEVERITY_INFO
/**
 * @def SID_PAL_LOG_INFO
 * @brief Logs an informational message if the severity level is greater than or equal to the info level.
 *
 * @param fmt_ The format string for the log message.
 * @param ... The arguments for the format string.
 */
#define SID_PAL_LOG_INFO(fmt_, ...)            SID_PAL_LOG(SID_PAL_LOG_SEVERITY_INFO,    fmt_, ##__VA_ARGS__)
#if SID_PAL_LOG_LEVEL >= SID_PAL_LOG_SEVERITY_DEBUG
/**
 * @def SID_PAL_LOG_DEBUG
 * @brief Logs a debug message if the severity level is greater than or equal to the debug level.
 *
 * @param fmt_ The format string for the log message.
 * @param ... The arguments for the format string.
 */
#define SID_PAL_LOG_DEBUG(fmt_, ...)           SID_PAL_LOG(SID_PAL_LOG_SEVERITY_DEBUG,   fmt_, ##__VA_ARGS__)
#endif /* WARNING */
#endif /* INFO */
#endif /* DEBUG */
/**
 * @def SID_PAL_LOG_TRACE
 * @brief Logs a trace message with file name, line number, and function name.
 *
 * This macro logs a trace message at the informational level, including the file name, line number, and function name where the macro is called.
 */
#define SID_PAL_LOG_TRACE()                    SID_PAL_LOG_INFO("%s:%i %s() TRACE --", __FILENAME__, __LINE__, __FUNCTION__)

#if SID_HAL_DISABLE_LOGS
#define SID_HAL_LOG_INFO(...)
#define SID_HAL_LOG_DEBUG(...)
#define SID_HAL_LOG_ERROR(...)
#define SID_HAL_LOG_WARNING(...)
#define SID_HAL_LOG_HEXDUMP_WARNING(data, len)
#define SID_HAL_LOG_HEXDUMP_INFO(data, len)
#define SID_HAL_LOG_HEXDUMP_DEBUG(data, len)
#define SID_HAL_LOG_FLUSH()
#else

/**
 * @def SID_HAL_LOG
 * @brief Logs a message if the severity level is less than or equal to the configured log level.
 *
 * @param level The severity level of the log message.
 * @param fmt_ The format string for the log message.
 * @param ... The arguments for the format string.
 */
#define SID_HAL_LOG(level, fmt_, ...)                                                           \
    do {                                                                                        \
            sid_pal_log(level, SID_PAL_VA_NARG(__VA_ARGS__), fmt_, ##__VA_ARGS__);              \
    } while(0)

/* Logging helpers to simplify logging APIs */
/**
 * @def SID_HAL_LOG_ERROR
 * @brief Logs an error message with the highest severity level.
 *
 * @param fmt_ The format string for the log message.
 * @param ... The arguments for the format string.
 */
#define SID_HAL_LOG_ERROR(fmt_, ...)                      SID_HAL_LOG(SID_PAL_LOG_SEVERITY_ERROR, fmt_, ##__VA_ARGS__)
/**
 * @def SID_HAL_LOG_WARNING
 * @brief Logs a warning message if the severity level is greater than or equal to the warning level.
 *
 * @param fmt_ The format string for the log message.
 * @param ... The arguments for the format string.
 */
#define SID_HAL_LOG_WARNING(fmt_, ...)                    SID_HAL_LOG(SID_PAL_LOG_SEVERITY_WARNING, fmt_, ##__VA_ARGS__)
/**
 * @def SID_HAL_LOG_INFO
 * @brief Logs an informational message if the severity level is greater than or equal to the info level.
 *
 * @param fmt_ The format string for the log message.
 * @param ... The arguments for the format string.
 */
#define SID_HAL_LOG_INFO(fmt_, ...)                       SID_HAL_LOG(SID_PAL_LOG_SEVERITY_INFO, fmt_, ##__VA_ARGS__)
/**
 * @def SID_HAL_LOG_DEBUG
 * @brief Logs a debug message if the severity level is greater than or equal to the debug level.
 *
 * @param fmt_ The format string for the log message.
 * @param ... The arguments for the format string.
 */
#define SID_HAL_LOG_DEBUG(fmt_, ...)                      SID_HAL_LOG(SID_PAL_LOG_SEVERITY_DEBUG, fmt_, ##__VA_ARGS__)
/**
 * @def SID_HAL_LOG_FLUSH
 * @brief Flushes the log buffer.
 *
 * This macro calls the function to flush the log buffer.
 */
#define SID_HAL_LOG_FLUSH                                 SID_PAL_LOG_FLUSH
/**
 * @def SID_HAL_LOG_PUSH_STR
 * @brief Pushes a string to the log.
 *
 * @param x The string to be pushed to the log.
 */
#define SID_HAL_LOG_PUSH_STR                              SID_PAL_LOG_PUSH_STR

/**
 * @def SID_HAL_LOG_HEXDUMP_ERROR
 * @brief Dumps a block of data in hexadecimal format with error severity.
 *
 * @param data_ The data to be dumped.
 * @param len_ The length of the data to be dumped.
 */
#define SID_HAL_LOG_HEXDUMP_ERROR(data_, len_)            sid_pal_hexdump(SID_PAL_LOG_SEVERITY_ERROR, data_, len_)
/**
 * @def SID_HAL_LOG_HEXDUMP_WARNING
 * @brief Dumps a block of data in hexadecimal format with warning severity.
 *
 * @param data_ The data to be dumped.
 * @param len_ The length of the data to be dumped.
 */
#define SID_HAL_LOG_HEXDUMP_WARNING(data_, len_)          sid_pal_hexdump(SID_PAL_LOG_SEVERITY_WARNING, data_, len_)
/**
 * @def SID_HAL_LOG_HEXDUMP_INFO
 * @brief Dumps a block of data in hexadecimal format with info severity.
 *
 * @param data_ The data to be dumped.
 * @param len_ The length of the data to be dumped.
 */
#define SID_HAL_LOG_HEXDUMP_INFO(data_, len_)             sid_pal_hexdump(SID_PAL_LOG_SEVERITY_INFO, data_, len_)
/**
 * @def SID_HAL_LOG_HEXDUMP_DEBUG
 * @brief Dumps a block of data in hexadecimal format with debug severity.
 *
 * @param data_ The data to be dumped.
 * @param len_ The length of the data to be dumped.
 */
#define SID_HAL_LOG_HEXDUMP_DEBUG(data_, len_)            sid_pal_hexdump(SID_PAL_LOG_SEVERITY_DEBUG, data_, len_)
#endif

#ifdef __cplusplus
}
#endif

#endif /* SID_PAL_LOG_IFC_H */

/** @} */ // end of sid_ifc group
/** @} */ // end of sid_pal_ifc group
/** @} */ // end of sid_pal_log_ifc group

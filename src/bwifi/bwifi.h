/**
 * @file
 *
 * @copyright Distributed under MIT Licence
 *   See https://github.com/house-of-abbey/Blackboard/blob/main/LICENCE.
 *
 * @brief Blackboard ESP32 Wi-Fi device driver
 *
 * @author J D Abbey & P A Abbey, 15 May 2026
 */

#ifndef _BWIFI_H_
#define _BWIFI_H_

#include "xparameters.h"

#ifdef XPAR_XUARTPS_NUM_INSTANCES

#include <xil_types.h>

#include "bwifi_proto.h"

// ================= EVENT HANDLERS =================

typedef void (*bwifi_data_handler_t)(const char* data, u32 len);
typedef void (*bwifi_log_handler_t)(bwifi_log_level_t level, const char* msg);
typedef void (*bwifi_status_handler_t)(bwifi_status_t status);

// ================= PUBLIC =================

/**
 * @brief Enter bootloader mode.
 *
 * This function puts the ESP32 into bootloader mode, allowing a new firmware to
 * be uploaded. In actuality, you should use the blackboard-esp-flasher
 * firmware.
 */
void BWifi_EnterBootloader();
/**
 * @brief Reset the Wi-Fi module.
 *
 * Starts the loaded program on the ESP32, this should be the WiFi module
 * firmware.
 */
void BWifi_Reset();

/**
 * @brief Initialise the Wi-Fi module.
 *
 * This function should be called before any other Wi-Fi operations.
 *
 * @return int XST_SUCCESS if successful, XST_FAILURE otherwise.
 */
int BWifi_Initialize();

/**
 * @brief Connect to a Wi-Fi network.
 *
 * This function also initialises relevant peripherals.
 *
 * @param ssid The SSID of the Wi-Fi network.
 * @param password The password of the Wi-Fi network.

 * @return int XST_SUCCESS if successful, XST_FAILURE otherwise.
 */
int BWifi_Connect(char* ssid, char* password);

/**
 * @brief Send data over the Wi-Fi module.
 *
 * This is a blocking call that waits for the data to be sent before returning.
 *
 * @param data The data to send.
 * @return int XST_SUCCESS if successful, XST_FAILURE otherwise.
 */
int BWifi_Send(u8* data);

/**
 * @brief Send a log message over the Wi-Fi module.
 *
 * This is a blocking call that waits for the log message to be sent before returning.
 *
 * @param level The log level.
 * @param data The log message.

 * @return int XST_SUCCESS if successful, XST_FAILURE otherwise.
 */
int BWifi_Log(bwifi_log_level_t level, char* data);

/**
 * @brief Send a formatted string over the Wi-Fi module.
 *
 * This is a blocking call that waits for the data to be sent before returning.
 *
 * @param format The format string.
 * @param ... The arguments to the format string.

 * @return int XST_SUCCESS if successful, XST_FAILURE otherwise.
 */
int BWifi_Sendf(const char* format, ...);

/**
 * @brief Send a formatted log message over the Wi-Fi module.
 *
 * This is a blocking call that waits for the data to be sent before returning.
 *
 * @param level The log level.
 * @param format The format string.
 * @param ... The arguments to the format string.

 * @return int XST_SUCCESS if successful, XST_FAILURE otherwise.
 */
int BWifi_Logf(bwifi_log_level_t level, const char* format, ...);

/**
 * @brief Set the data handler for the Wi-Fi module.
 *
 * @param handler The data handler function.
 */
void BWifi_SetHandler(bwifi_data_handler_t handler);

/**
 * @brief Set the log handler for the Wi-Fi module.
 *
 * @param handler The log handler function.
 */
void BWifi_SetLogHandler(bwifi_log_handler_t handler);

/**
 * @brief Set the status handler for the Wi-Fi module.
 *
 * @param handler The status handler function.
 */
void BWifi_SetStatusHandler(bwifi_status_handler_t handler);

/**
 * @brief Loop function for the Wi-Fi module.
 *
 * This function should be called periodically to process incoming data.
 * Or alternatively, it can be setup in a task scheduler.
 */
void BWifi_Loop();

/**
 * @brief Get state.
 */
bwifi_state_t BWifi_GetState();

/**
 * @brief Wait for the Wi-Fi module to connect to a network.
 *
 * This is a blocking call that waits for the Wi-Fi module to connect before
 * returning.
 *
 * If the Wi-Fi modules asks for new credentials, the function will return
 * XST_FAILURE.
 *
 * @return int XST_SUCCESS if successful, XST_FAILURE otherwise.
 */
int BWifi_WaitConnected();

/**
 * @brief Get the IP address of the Wi-Fi module.
 *
 * @return uint32_t The IP address of the Wi-Fi module (little endian not network byte order).
 */
u32 BWifi_GetAddress();

#endif

#endif

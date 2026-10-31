/**
 * @file
 *
 * @copyright Distributed under MIT Licence
 *   See https://github.com/house-of-abbey/Blackboard/blob/main/LICENCE.
 *
 * @brief Blackboard UNIX time driver.
 *
 * @author J D Abbey & P A Abbey, 15 May 2026
 */

#ifndef _BTIME_H_
#define _BTIME_H_

#include <stdint.h>

#include "xil_types.h"
#include "xiltimer.h"
#include "xparameters.h"

#define BTIME_TICKS_PER_MS     ((uint64_t)XPAR_CPU_CORE_CLOCK_FREQ_HZ / 2000ULL)
#define BTIME_TICKS_PER_SECOND ((uint64_t)XPAR_CPU_CORE_CLOCK_FREQ_HZ / 2ULL)

/**
 * @brief Initialise the Blackboard time driver.
 *
 * This resets the current wall-clock synchronisation state and leaves the
 * monotonic timer available for relative time measurements from boot/reset.
 *
 * @return XST_SUCCESS if successful, XST_FAILURE otherwise.
 */
int BTime_Initialize(void);

/**
 * @brief Check whether the wall-clock time has been set.
 *
 * @return Non-zero if Unix time has been set at least once, zero otherwise.
 */
int BTime_IsSet(void);

/**
 * @brief Set the current wall-clock time from a Unix timestamp in milliseconds.
 *
 * @param[in] unix_time_ms Unix time in milliseconds.
 */
void BTime_SetUnixTimeMs(uint64_t unix_time_ms);

/**
 * @brief Get the monotonic time since boot/reset in raw global timer ticks.
 *
 * @return Monotonic timer value in raw ticks.
 */
uint64_t BTime_GetMonotonicTicks(void);

/**
 * @brief Get the monotonic time since boot/reset in milliseconds.
 *
 * @return Monotonic time in milliseconds.
 */
uint64_t BTime_GetMonotonicMs(void);

/**
 * @brief Get the monotonic time since boot/reset in seconds.
 *
 * @return Monotonic time in seconds.
 */
uint64_t BTime_GetMonotonicSeconds(void);

/**
 * @brief Get the current Unix time in milliseconds.
 *
 * @return Current Unix time in milliseconds, or 0 if the time has not been set yet.
 */
uint64_t BTime_GetUnixTimeMs(void);

/**
 * @brief Get the current Unix time in seconds.
 *
 * @return Current Unix time in seconds, or 0 if the time has not been set yet.
 */
uint64_t BTime_GetUnixTime(void);

/**
 * @brief Convert a Unix timestamp in seconds to a packed FAT timestamp.
 *
 * The FAT timestamp packs the date and time into a 32-bit value with a base
 * year of 1980.
 *
 * @param[in] unix_seconds Unix time in whole seconds.
 *
 * @return Packed FAT timestamp.
 */
uint32_t BTime_UnixTimeToFatTime(uint64_t unix_seconds);

/**
 * @brief Get the current wall-clock time as a packed FAT timestamp.
 *
 * @return Packed FAT timestamp, or the FAT epoch if the wall-clock has not
 *         been set.
 */
uint32_t BTime_GetFatTime(void);

#endif

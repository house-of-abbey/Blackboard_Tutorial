/**
 * @file
 *
 * @copyright Distributed under MIT Licence
 *   See https://github.com/house-of-abbey/Blackboard/blob/main/LICENCE.
 *
 * @brief Keep track of time in UNIX format.
 *
 * @author J D Abbey & P A Abbey, 15 May 2026
 */

#include "btime.h"

#include <stdint.h>
#include <time.h>

#include "xil_exception.h"
#include "xil_types.h"
#include "xiltimer.h"
#include "xstatus.h"

static u64 g_sync_ticks   = 0;
static u64 g_sync_unix_ms = 0;

static int g_time_is_set = 0;

#define ticks_to_ms(ticks)      ((ticks) / BTIME_TICKS_PER_MS)
#define ticks_to_seconds(ticks) ((ticks) / BTIME_TICKS_PER_SECOND)

int BTime_Initialize(void) {
  Xil_ExceptionDisable();

  g_sync_ticks   = 0U;
  g_sync_unix_ms = 0U;
  g_time_is_set  = 0;

  Xil_ExceptionEnable();

  return XST_SUCCESS;
}

int BTime_IsSet(void) {
  int is_set;

  Xil_ExceptionDisable();
  is_set = g_time_is_set;
  Xil_ExceptionEnable();

  return is_set;
}

u64 BTime_GetMonotonicTicks(void) {
  XTime now;

  XTime_GetTime(&now);

  return (u64)now;
}

u64 BTime_GetMonotonicMs(void) { return ticks_to_ms(BTime_GetMonotonicTicks()); }

u64 BTime_GetMonotonicSeconds(void) { return ticks_to_seconds(BTime_GetMonotonicTicks()); }

void BTime_SetUnixTimeMs(u64 unix_time_ms) {
  u64 now_ticks;

  now_ticks = BTime_GetMonotonicTicks();

  Xil_ExceptionDisable();

  g_sync_ticks   = now_ticks;
  g_sync_unix_ms = unix_time_ms;
  g_time_is_set  = 1;

  Xil_ExceptionEnable();
}

u64 BTime_GetUnixTime(void) { return BTime_GetUnixTimeMs() / 1000ULL; }

u64 BTime_GetUnixTimeMs(void) {
  u64 now_ticks;
  u64 sync_ticks;
  u64 sync_unix_ms;
  u64 elapsed_ticks;
  u64 elapsed_ms;
  int time_is_set;

  now_ticks = BTime_GetMonotonicTicks();

  Xil_ExceptionDisable();

  sync_ticks   = g_sync_ticks;
  sync_unix_ms = g_sync_unix_ms;
  time_is_set  = g_time_is_set;

  Xil_ExceptionEnable();

  if (!time_is_set) {
    return 0U;
  }

  elapsed_ticks = now_ticks - sync_ticks;
  elapsed_ms    = ticks_to_ms(elapsed_ticks);

  return sync_unix_ms + elapsed_ms;
}

u32 BTime_UnixTimeToFatTime(u64 unix_seconds) {
  time_t     raw_time = (time_t)unix_seconds;
  struct tm* tm_utc   = gmtime(&raw_time);
  unsigned   year;

  if (tm_utc == NULL) {
    return 0U;
  }

  year = (unsigned)(tm_utc->tm_year + 1900);
  if (year < 1980U) {
    year = 1980U;
  }

  return ((u32)(year - 1980U) << 25) | ((u32)(tm_utc->tm_mon + 1) << 21) |
         ((u32)tm_utc->tm_mday << 16) | ((u32)tm_utc->tm_hour << 11) | ((u32)tm_utc->tm_min << 5) |
         ((u32)tm_utc->tm_sec >> 1);
}

u32 BTime_GetFatTime(void) {
  if (!BTime_IsSet()) {
    return 0U;
  }

  return BTime_UnixTimeToFatTime(BTime_GetUnixTime());
}

/**
 * Provide the time directly to the ff driver.
 *
 * @note Add "-Wl,--wrap=get_fattime" to the linker flags,
 *       this is because Xilinx provides `get_fattime` for
 *       us; Helpful! Until you want to override it.
 */
u32 __wrap_get_fattime() { return BTime_GetFatTime(); }

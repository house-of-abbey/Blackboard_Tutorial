//-----------------------------------------------------------------------------------
//
// Distributed under MIT Licence
//   See https://github.com/Blackboard_Examples/blob/main/LICENSE
//
//-----------------------------------------------------------------------------------
//
// Software test for the 7-segment display logic connected by GPIO IP.
//
//-----------------------------------------------------------------------------------

#include <time.h>
// https://github.com/Xilinx/embeddedsw/blob/master/XilinxProcessorIPLib/drivers/common/src/xstatus.h
#include <xstatus.h>
#include <xil_printf.h>
// https://github.com/Xilinx/embeddedsw/blob/master/lib/bsp/standalone/src/common/sleep.h
#include "sleep.h"
// https://github.com/Xilinx/embeddedsw/blob/master/XilinxProcessorIPLib/drivers/gpio/src/xgpio.h
#include "xgpio.h"
// "<PATH>\build\network_plat\ps7_cortexa9_0\standalone_ps7_cortexa9_0\bsp\include\xparameters.h"
#include "xparameters.h"

#include "bwifi.h"
#include "btime.h"

/*
The <time.h> header shall declare the structure tm, which shall include at least the following members:

int tm_sec   Seconds [0,60]. 
int tm_min   Minutes [0,59]. 
int tm_hour  Hour [0,23]. 
int tm_mday  Day of month [1,31]. 
int tm_mon   Month of year [0,11]. 
int tm_year  Years since 1900. 
int tm_wday  Day of week [0,6] (Sunday =0). 
int tm_yday  Day of year [0,365]. 
int tm_isdst Daylight Savings flag.
*/
struct tm* get_time() {
  time_t secs = (time_t)BTime_GetUnixTime();
  return localtime(&secs);
}

void data_handler(const char* data, u32 len) {
  print("Data: ");
  for (u32 i = 0; i < len; i++) {
    outbyte(data[i]);
  }
  print("\r\n");
}

void log_handler(bwifi_log_level_t level, const char* msg) {
  switch (level) {
    case BWIFI_LOG_LEVEL_INFO:
      xil_printf("\x1b[34mESP: %s\x1b[0m\r\n", msg);
      break;
    case BWIFI_LOG_LEVEL_WARN:
      xil_printf("\x1b[34mESP: WARNING: %s\x1b[0m\r\n", msg);
      break;
    case BWIFI_LOG_LEVEL_ERROR:
      xil_printf("\x1b[34mESP: ERROR: %s\x1b[0m\r\n", msg);
      break;
  }
}

// Handle status changes from the Wi-Fi module
void status_handler(bwifi_status_t status) {
  switch (status) {
    case BWIFI_STATUS_CONNECTING:
      print("Connecting...\r\n");
      break;
    case BWIFI_STATUS_CONNECTION_SUCCESSFUL:
      // Once connected the IP address is available, it is a u32
      u32 addr = BWifi_GetAddress();
      xil_printf(
        "Connected to Wi-Fi on IP Address: %d.%d.%d.%d\r\n", (u8)(addr >> 24), (u8)(addr >> 16), (u8)(addr >> 8), (u8)(addr)
      );
      break;
    case BWIFI_STATUS_CONNECTION_FAILED:
      print("\r\nFailed to connect to Wi-Fi.\r\n");
      break;
    case BWIFI_STATUS_DISCONNECTED:
      print("\r\nDisconnected from Wi-Fi.\r\n");
      break;
    case BWIFI_STATUS_AUTH_REQUIRED:
      // The Wi-Fi module signals that it needs the credentials.
      if (
        BWifi_Connect(
          "Test Hotspot",
          "TerriblePassword"
        ) != XST_SUCCESS
      ) {
        print("\r\nFailed to send credentials.\r\n");
      } else {
        print("Credentials sent.\r\n");
      }
      break;
    default:
      break;
  }
}

static XGpio g_gpio;

#define LED_ENABLE      0x00000001
#define LED_MODE_DIRECT 0x00000003

int BoardGpio_Init() {
  XGpio_Config* config = XGpio_LookupConfig(XPAR_AXI_GPIO_0_BASEADDR);
  if (config == NULL) {
    xil_printf("XGpio_LookupConfig failed\r\n");
    return XST_FAILURE;
  }

  int status = XGpio_CfgInitialize(&g_gpio, config, config->BaseAddress);
  if (status != XST_SUCCESS) {
    xil_printf("XGpio_CfgInitialize failed: %d\r\n", status);
    return status;
  }

  status = XGpio_SelfTest(&g_gpio);
  if (status != XST_SUCCESS) {
    xil_printf("XGpio_SelfTest failed: %d\r\n", status);
    return status;
  }

  // Configure the GPIO device? Should be hardcoded.
  // XGpio_SetDataDirection(&g_gpio, 1, 0x00000000); // Outputs
  // XGpio_SetDataDirection(&g_gpio, 2, 0x00000000); // Outputs

  // Seven-Segment Display
  // Basic Combinational Circuit Blocks
  // https://www.realdigital.org/doc/b49d5b28e4b41191f30d43e79430192c
  //
  // Bottom two bits only are connected:
  // Bit 0: Enable
  // Bit 1: Mode
  XGpio_DiscreteWrite(&g_gpio, 2, LED_ENABLE);
  // Mode bit:
  //   0. BCD assign as 4 x byte of "P---NNNN",
  //   1. Direct Assign as "PGFEDCBA"
  // Where:
  //    P = Decimal Point
  //    N = 0..15 Hex                                   
  //    
  //         A
  //        ---
  //     F |   | B
  //        ---
  //     E | G | C
  //        --- . P
  //         D
  //
  // E.g. Mode 0 for BCD data of 0x01820304 gives a "12.34" display
  XGpio_DiscreteWrite(&g_gpio, 1, 0x01820304);

  return XST_SUCCESS;
}

#define LED_POINT_SEP 0x00800000

// Assume 24 hour clock
void led_display_time(int8_t hours, int8_t mins) {
  XGpio_DiscreteWrite(&g_gpio, 2, LED_ENABLE);
  // Hours 0..23
  uint8_t digit3 = hours / 10;
  uint8_t digit2 = hours % 10;
  // Mins 0..59
  uint8_t digit1 = mins / 10;
  uint8_t digit0 = mins % 10;
  unsigned packed = (digit3 << 24) | (digit2 << 16) | LED_POINT_SEP | (digit1 << 8) | digit0 << 0;
  XGpio_DiscreteWrite(&g_gpio, 1, packed);
}

void led_display_direct(int8_t digit3, int8_t digit2, int8_t digit1, int8_t digit0) {
  XGpio_DiscreteWrite(&g_gpio, 2, LED_ENABLE | LED_MODE_DIRECT);
  // Without the "& 0xFF" the upper bits are set and this is undesirable for the OR operation
  unsigned packed  = ((((unsigned) digit3) & 0xFF) << 24) |
                     ((((unsigned) digit2) & 0xFF) << 16) |
                     ((((unsigned) digit1) & 0xFF) <<  8) |
                      (((unsigned) digit0) & 0xFF);
  XGpio_DiscreteWrite(&g_gpio, 1, packed);
}

int main(void) {
  xil_printf("\r\nTime Example Application\r\n");

  if (BoardGpio_Init() != XST_SUCCESS) {
    xil_printf("BoardGpio_Init failed\r\n");
    return XST_FAILURE;
  }

  /* Belt & Braces to reset values, must be called before BWifi_Initialize() */
  if (BTime_Initialize() != XST_SUCCESS) {
    print("\r\nFailed to setup Time\r\n");
    return XST_FAILURE;
  }

  if (BWifi_Initialize() != XST_SUCCESS) {
    print("\r\nFailed to initialize Wi-Fi\r\n");
    return XST_FAILURE;
  }

  // If no "BWifi_SetHandler" or "BWifi_SetLogHandler" are provided, data is discarded.
  // Data received by the Blackboard
  BWifi_SetHandler(data_handler);
  // Logs sent by the Blackboard
  BWifi_SetLogHandler(log_handler);
  // Handle a change in state where the state diagram is drawn in "bwifi_proto.h"
  // This handler is required, e.g. to provider SSID and password.
  BWifi_SetStatusHandler(status_handler);

  xil_printf("Waiting for connection...\r\n");
  // Wait for the Wi-Fi module to connect
  BWifi_WaitConnected();

  xil_printf("Waiting for network time...\r\n");
  // Print the time once every second
  u64 ticks;
  u64 last_print = 0;
  uint8_t segments = 0x01;
  while(!BTime_IsSet()) {
    // To maintain the connection, you must call BWifi_Loop() periodically
    BWifi_Loop();
    ticks = BTime_GetMonotonicTicks();
    // Do not u/sleep(), keep calling 'BWifi_Loop()' or time won't be set.
    if ((ticks - last_print) > (BTIME_TICKS_PER_SECOND / 10)) {
      last_print = ticks;
      led_display_direct(segments, segments, segments, segments);
      segments = (segments << 1);
      if (segments == 0x40) {
        // Skip segments G and P
        segments = 0x01;
      }
    }
  }

  struct tm *time = get_time();
  xil_printf("Time set at %02d:%02d:%02d\r\n", time->tm_hour, time->tm_min, time->tm_sec);
  xil_printf("Running...\r\n");

  while(1) {
    // To maintain the connection, you must call BWifi_Loop() periodically
    BWifi_Loop();
    // Generated froma 64-bit integer. If ticks never exceed 2^16-1, then the wrong timer is being used.
    ticks = BTime_GetMonotonicTicks();
    if ((ticks - last_print) > BTIME_TICKS_PER_SECOND) {
      last_print = ticks;
      time = get_time();
      led_display_time(time->tm_min, time->tm_sec);
    }
  }

  return XST_FAILURE;
}

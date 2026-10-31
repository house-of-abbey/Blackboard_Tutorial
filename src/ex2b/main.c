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

// https://github.com/Xilinx/embeddedsw/blob/master/XilinxProcessorIPLib/drivers/gpio/src/xgpio.h
#include "xgpio.h"
// https://github.com/Xilinx/embeddedsw/blob/master/XilinxProcessorIPLib/drivers/common/src/xstatus.h
#include "xstatus.h"
#include <xil_printf.h>
// "<PATH>\build\network_plat\ps7_cortexa9_0\standalone_ps7_cortexa9_0\bsp\include\xparameters.h"
#include "xparameters.h"

static XGpio g_gpio;

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
  XGpio_SetDataDirection(&g_gpio, 1, 0x00000000); // Outputs
  XGpio_SetDataDirection(&g_gpio, 2, 0x00000000); // Outputs

  // Seven-Segment Display
  // Basic Combinational Circuit Blocks
  // https://www.realdigital.org/doc/b49d5b28e4b41191f30d43e79430192c
  //
  // Bottom two bits only are connected:
  // Bit 0: Enable
  // Bit 1: Mode
  XGpio_DiscreteWrite(&g_gpio, 2, 0x00000002);
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
  XGpio_DiscreteWrite(&g_gpio, 1, 0x01820304); // All on

  return XST_SUCCESS;
}

int main(void) {
  xil_printf("\r\nTime Example Application\r\n");

  if (BoardGpio_Init() != XST_SUCCESS) {
    xil_printf("BoardGpio_Init failed\r\n");
    return XST_FAILURE;
  }

  while(1) {
    // Write something to the 4x7 segment display via GPIO
  }

  return XST_FAILURE;
}

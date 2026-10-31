//-----------------------------------------------------------------------------------
//
// Distributed under MIT Licence
//   See https://github.com/Blackboard_Examples/blob/main/LICENSE
//
//-----------------------------------------------------------------------------------
//
// Read the potentiometer rotation value via the XADC IP. Example 1a.
//
//-----------------------------------------------------------------------------------

#include <stdint.h>
// https://github.com/Xilinx/embeddedsw/blob/master/XilinxProcessorIPLib/drivers/common/src/xstatus.h
#include <xstatus.h>
// https://github.com/Xilinx/embeddedsw/blob/master/lib/bsp/standalone/src/common/xil_printf.h
#include "xil_printf.h"
#include "board_xadc.h"

int main(void) {
  xil_printf("\r\nExample 1a: Read potentiometer voltage.\r\n");

  if (BoardXadc_Init() != XST_SUCCESS) {
    xil_printf("BoardXadc_Init failed\r\n");
    return XST_FAILURE;
  }

  while(1) {
    uint16_t raw = BoardXadc_ReadPotRaw();
    xil_printf("Potentiometer: raw = %5d (%3d%%).\r\n", raw, BoardPot_ToPercent(raw));
  }

  return XST_FAILURE;
}

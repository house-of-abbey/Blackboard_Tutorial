//-----------------------------------------------------------------------------------
//
// Distributed under MIT Licence
//   See https://github.com/Blackboard_Examples/blob/main/LICENSE
//
//-----------------------------------------------------------------------------------
//
// Read the potentiometer rotation value via the XADC IP using FreeRTOS. Example 1c.
//
//-----------------------------------------------------------------------------------

#include "board_xadc.h"

// https://github.com/Xilinx/embeddedsw/blob/master/XilinxProcessorIPLib/drivers/common/src/xstatus.h
#include <xstatus.h>
// https://github.com/Xilinx/embeddedsw/blob/master/lib/bsp/standalone/src/common/xil_printf.h
#include "xil_printf.h"
// https://github.com/Xilinx/embeddedsw/blob/master/XilinxProcessorIPLib/drivers/sysmon/src/xsysmon.h
#include "xsysmon.h"
// "<PATH>\build\video_plat\ps7_cortexa9_0\standalone_ps7_cortexa9_0\bsp\include\xparameters.h"
#include "xparameters.h"

static XSysMon  g_sysmon;

//! Initialise the XADC IP.
//
int BoardXadc_Init(void) {
  XSysMon_Config* config = XSysMon_LookupConfig(XPAR_XADC_WIZ_I_BASEADDR);
  if (config == NULL) {
    xil_printf("XSysMon_LookupConfig failed\r\n");
    return XST_FAILURE;
  }

  int status = XSysMon_CfgInitialize(&g_sysmon, config, config->BaseAddress);
  if (status != XST_SUCCESS) {
    xil_printf("XSysMon_CfgInitialize failed: %d\r\n", status);
    return status;
  }

  status = XSysMon_SelfTest(&g_sysmon);
  if (status != XST_SUCCESS) {
    xil_printf("XSysMon_SelfTest failed: %d\r\n", status);
    return status;
  }

  XSysMon_SetSequencerMode(&g_sysmon, XSM_SEQ_MODE_SAFE);
  XSysMon_SetAlarmEnables(&g_sysmon, 0x0);
  XSysMon_SetSeqChEnables(&g_sysmon, XSM_SEQ_CH_VPVN);
  XSysMon_SetAdcClkDivisor(&g_sysmon, 32);
  XSysMon_SetSequencerMode(&g_sysmon, XSM_SEQ_MODE_CONTINPASS);

  return XST_SUCCESS;
}

//! Returns the raw value read from the Analogue to Digital converter IP where the Blackboard
//! connects the ADC input to a variable poteniometer. Hence this is a reading of the converted
//! voltage in the range 0..65535.
//!
//! @return A 16-bit value from the ADC, 0..65535
//
uint16_t BoardXadc_ReadPotRaw(void) {
  return (uint16_t)XSysMon_GetAdcData(&g_sysmon, XSM_CH_VPVN);
}

//! Convert a 16-bit unsigned value to a percentage.
//!
//! @param raw A 16-bit unsigned value.
//!
//! @return An unsigned integer of `raw` converted to a percentage and rounded down to the
//!         nearest integer.
//
uint32_t BoardPot_ToPercent(uint16_t raw) {
  return ((uint32_t)raw * 100U) / 65535U;
}

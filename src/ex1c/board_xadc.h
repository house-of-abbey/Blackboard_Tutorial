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

#ifndef BOARD_XADC_H
#define BOARD_XADC_H

#include <stdint.h>
// https://github.com/Xilinx/embeddedsw/blob/master/lib/bsp/standalone/src/common/xil_printf.h
#include "xil_printf.h"

//! Initialise the XADC IP.
//
int      BoardXadc_Init(void);

//! Returns the raw value read from the Analogue to Digital converter IP where the Blackboard
//! connects the ADC input to a variable poteniometer. Hence this is a reading of the converted
//! voltage in the range 0..65535.
//!
//! @return A 16-bit value from the ADC, 0..65535
//
uint16_t BoardXadc_ReadPotRaw(void);

//! Convert a 16-bit unsigned value to a percentage.
//!
//! @param raw A 16-bit unsigned value.
//!
//! @return An unsigned integer of `raw` converted to a percentage and rounded down to the
//!         nearest integer.
//
uint32_t BoardPot_ToPercent(uint16_t raw);

#endif

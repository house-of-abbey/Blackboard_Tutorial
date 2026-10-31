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

#ifndef UI_H
#define UI_H

#include <stdint.h>

void UI_Init(void);
void UI_SetPotValue(uint16_t raw, uint32_t percent);

#endif

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

#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_DEPTH              32
#define LV_USE_OS                   LV_OS_FREERTOS
#define LV_USE_FREERTOS_TASK_NOTIFY 1

#define LV_DRAW_BUF_ALIGN        64
#define LV_DRAW_BUF_STRIDE_ALIGN 64

#define LV_USE_LOG      0
#define LV_USE_SYSMON   0
#define LV_USE_PROFILER 0

#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_48 1

#define LV_USE_MENU     0
#define LV_USE_MSGBOX   0
#define LV_USE_SPINBOX  0
#define LV_USE_SPINNER  0
#define LV_USE_KEYBOARD 0
#define LV_USE_LIST     0
#define LV_USE_LOTTIE   0
#define LV_USE_TABVIEW  0
#define LV_USE_WIN      0

#endif

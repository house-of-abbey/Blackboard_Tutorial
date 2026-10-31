//-----------------------------------------------------------------------------------
//
// Distributed under MIT Licence
//   See https://github.com/Blackboard_Examples/blob/main/LICENSE
//
//-----------------------------------------------------------------------------------
//
// Read the potentiometer rotation value via the XADC IP using FreeRTOS. Example 1d.
//
//-----------------------------------------------------------------------------------

#include <stdint.h>
#include <xstatus.h>

#include "FreeRTOS.h"
#include "board_xadc.h"
#include "task.h"
#include "xil_printf.h"

#define ADC_SAMPLE_PERIOD_MS 10U
#define ADC_TASK_STACK_WORDS 512U
#define ADC_TASK_PRIORITY    (tskIDLE_PRIORITY + 3)
#define ADC_FILTER_WINDOW    8U

typedef struct {
  uint16_t raw;
  uint32_t percent;
} PotReading;

static volatile PotReading g_pot_reading;

static void UpdatePotReading(uint16_t raw) {
  g_pot_reading.raw     = raw;
  g_pot_reading.percent = BoardPot_ToPercent(raw);
}

static void AdcTask(void* context) {
  (void)context;

  uint16_t samples[ADC_FILTER_WINDOW] = {0};
  uint32_t sample_sum                 = 0;
  uint32_t sample_index               = 0;
  uint32_t sample_count               = 0;
  uint32_t debug_count                = 0;

  while (1) {
    uint16_t raw = BoardXadc_ReadPotRaw();

    // Average over last ADC_FILTER_WINDOW samples
    sample_sum -= samples[sample_index];
    samples[sample_index] = raw;
    sample_sum += raw;
    sample_index = (sample_index + 1U) % ADC_FILTER_WINDOW;
    if (sample_count < ADC_FILTER_WINDOW) {
      sample_count++;
    }

    UpdatePotReading((uint16_t)(sample_sum / sample_count));

    // Print every 50th value to the serial terminal.
    if ((debug_count++ % 50U) == 0U) {
      xil_printf("Potentiometer: raw = %5d (%3d%%).\r\n", g_pot_reading.raw, g_pot_reading.percent);
    }

    vTaskDelay(pdMS_TO_TICKS(ADC_SAMPLE_PERIOD_MS));
  }
}

int main(void) {
  xil_printf("\r\nvideo_lvgl demo starting\r\n");
  xil_printf("FreeRTOS heap: %u bytes\r\n", (unsigned int)xPortGetFreeHeapSize());

  if (BoardXadc_Init() != XST_SUCCESS) {
    xil_printf("BoardXadc_Init failed\r\n");
    return XST_FAILURE;
  }

  BaseType_t status = xTaskCreate(AdcTask, "adc", ADC_TASK_STACK_WORDS, NULL, ADC_TASK_PRIORITY, NULL);
  if (status != pdPASS) {
    xil_printf("Failed to create ADC task\r\n");
    return XST_FAILURE;
  }

  xil_printf("Heap after ADC task: %u bytes\r\n", (unsigned int)xPortGetFreeHeapSize());
  vTaskStartScheduler();

  xil_printf("Scheduler exited unexpectedly\r\n");
  return XST_FAILURE;
}

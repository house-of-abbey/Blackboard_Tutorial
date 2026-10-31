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

#include "board_video.h"

#include <stdint.h>

#include "FreeRTOS.h"
#include "lvgl.h"
#include "portmacro.h"
#include "task.h"
#include "xaxivdma.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xvtc.h"

#define FRAME_WIDTH     ((uint32_t)BOARD_VIDEO_WIDTH)
#define FRAME_HEIGHT    ((uint32_t)BOARD_VIDEO_HEIGHT)
#define BYTES_PER_PIXEL 4U
#define FRAME_STRIDE    (FRAME_WIDTH * BYTES_PER_PIXEL)
#define FRAME_BYTES     (FRAME_STRIDE * FRAME_HEIGHT)
#define FRAME_COUNT     2U

static XAxiVdma         g_vdma;
static XVtc             g_vtc;
static lv_display_t*    g_display;
static volatile int32_t g_current_frame;
static volatile int32_t g_pending_frame = -1;
static uint32_t         g_flush_debug_count;

static uint8_t g_framebuffers[FRAME_COUNT][FRAME_BYTES] __attribute__((aligned(64)));

static void FlushArea(const lv_area_t* area, uint8_t* framebuffer) {
  int32_t x1 = area->x1 < 0 ? 0 : area->x1;
  int32_t x2 = area->x2 >= (int32_t)FRAME_WIDTH ? (int32_t)FRAME_WIDTH - 1 : area->x2;
  int32_t y1 = area->y1 < 0 ? 0 : area->y1;
  int32_t y2 = area->y2 >= (int32_t)FRAME_HEIGHT ? (int32_t)FRAME_HEIGHT - 1 : area->y2;

  if (x1 > x2 || y1 > y2) {
    return;
  }

  uint32_t row_bytes = (uint32_t)(x2 - x1 + 1) * BYTES_PER_PIXEL;

  for (int32_t y = y1; y <= y2; ++y) {
    UINTPTR row_addr = (UINTPTR)(framebuffer + y * FRAME_STRIDE + x1 * BYTES_PER_PIXEL);
    Xil_DCacheFlushRange(row_addr, row_bytes);
  }
}

static int FrameIndexFromPtr(const void* ptr) {
  uintptr_t addr = (uintptr_t)ptr;

  for (uint32_t i = 0; i < FRAME_COUNT; ++i) {
    uintptr_t start = (uintptr_t)g_framebuffers[i];
    uintptr_t end   = start + FRAME_BYTES;
    if (addr >= start && addr < end) {
      return (int)i;
    }
  }

  return -1;
}

static void LvglFlush(lv_display_t* display, const lv_area_t* area, uint8_t* px_map) {
  int frame_index = FrameIndexFromPtr(px_map);
  if (frame_index < 0) {
    xil_printf("Flush buffer not recognized\r\n");
    lv_display_flush_ready(display);
    return;
  }

  FlushArea(area, g_framebuffers[frame_index]);

  if (!lv_display_flush_is_last(display)) {
    lv_display_flush_ready(display);
    return;
  }

  if ((g_flush_debug_count++ % 10U) == 0U) {
    xil_printf(
      "flush frame=%d area=(%d,%d)-(%d,%d)\r\n", frame_index, (int)area->x1, (int)area->y1,
      (int)area->x2, (int)area->y2
    );
  }

  g_pending_frame = frame_index;
  int status      = XAxiVdma_StartParking(&g_vdma, frame_index, XAXIVDMA_READ);
  if (status != XST_SUCCESS) {
    xil_printf("XAxiVdma_StartParking failed: %d\r\n", status);
    g_pending_frame = -1;
    lv_display_flush_ready(display);
  }
}

static void LvglFlushWait(lv_display_t* display) {
  (void)display;

  if (g_pending_frame < 0) {
    return;
  }

  uint32_t timeout_ticks = 20U;
  while (1) {
    g_current_frame = (int32_t)XAxiVdma_CurrFrameStore(&g_vdma, XAXIVDMA_READ);
    if (g_current_frame == g_pending_frame) {
      g_pending_frame = -1;
      break;
    }
    if (timeout_ticks == 0U) {
      xil_printf(
        "Flush wait timeout: current=%ld pending=%ld\r\n", (long)g_current_frame,
        (long)g_pending_frame
      );
      g_pending_frame = -1;
      break;
    }
    timeout_ticks--;
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

static int SetupInterrupts(void) {
  BaseType_t status = xPortInstallInterruptHandler(
    XPAR_FABRIC_AXI_VDMA_I_INTR, (XInterruptHandler)XAxiVdma_ReadIntrHandler, &g_vdma
  );
  if (status != pdPASS) {
    return XST_FAILURE;
  }

  vPortEnableInterrupt(XPAR_FABRIC_AXI_VDMA_I_INTR);

  return XST_SUCCESS;
}

static int SetupVtc(void) {
  XVtc_Config* config = XVtc_LookupConfig(XPAR_V_TC_I_BASEADDR);
  if (config == NULL) {
    return XST_FAILURE;
  }

  int status = XVtc_CfgInitialize(&g_vtc, config, config->BaseAddress);
  if (status != XST_SUCCESS) {
    return status;
  }

  status = XVtc_SelfTest(&g_vtc);
  if (status != XST_SUCCESS) {
    return status;
  }

  XVtc_SetGeneratorVideoMode(&g_vtc, XVTC_VMODE_720P);
  XVtc_Enable(&g_vtc);

  return XST_SUCCESS;
}

static int SetupVdma(void) {
  XAxiVdma_Config* config = XAxiVdma_LookupConfig(XPAR_AXI_VDMA_I_BASEADDR);
  if (config == NULL) {
    return XST_FAILURE;
  }

  int status = XAxiVdma_CfgInitialize(&g_vdma, config, config->BaseAddress);
  if (status != XST_SUCCESS) {
    return status;
  }

  status = XAxiVdma_Selftest(&g_vdma);
  if (status != XST_SUCCESS) {
    return status;
  }

  XAxiVdma_DmaSetup dma_setup      = {0};
  dma_setup.VertSizeInput          = FRAME_HEIGHT;
  dma_setup.HoriSizeInput          = FRAME_STRIDE;
  dma_setup.Stride                 = FRAME_STRIDE;
  dma_setup.FrameDelay             = 0;
  dma_setup.EnableCircularBuf      = 0;
  dma_setup.EnableSync             = 1;
  dma_setup.PointNum               = 0;
  dma_setup.EnableFrameCounter     = 0;
  dma_setup.FrameStoreStartAddr[0] = (UINTPTR)g_framebuffers[0];
  dma_setup.FrameStoreStartAddr[1] = (UINTPTR)g_framebuffers[1];
  dma_setup.FixedFrameStoreAddr    = 1;
  dma_setup.GenLockRepeat          = 1;
  dma_setup.EnableVFlip            = 0;

  status = XAxiVdma_StartReadFrame(&g_vdma, &dma_setup);
  if (status != XST_SUCCESS) {
    xil_printf("XAxiVdma_StartReadFrame failed: %d\r\n", status);
    return status;
  }

  g_current_frame = (int32_t)XAxiVdma_CurrFrameStore(&g_vdma, XAXIVDMA_READ);
  return XST_SUCCESS;
}

int BoardVideo_Init(void) {
  if (g_display != NULL) {
    return XST_SUCCESS;
  }

  for (uint32_t i = 0; i < FRAME_COUNT; ++i) {
    for (uint32_t j = 0; j < FRAME_BYTES; ++j) {
      g_framebuffers[i][j] = 0;
    }
    Xil_DCacheFlushRange((UINTPTR)g_framebuffers[i], FRAME_BYTES);
  }

  int status = SetupVtc();
  if (status != XST_SUCCESS) {
    xil_printf("SetupVtc failed: %d\r\n", status);
    return status;
  }

  status = SetupInterrupts();
  if (status != XST_SUCCESS) {
    xil_printf("SetupInterrupts failed: %d\r\n", status);
    return status;
  }

  status = SetupVdma();
  if (status != XST_SUCCESS) {
    xil_printf("SetupVdma failed: %d\r\n", status);
    return status;
  }

  g_display = lv_display_create(FRAME_WIDTH, FRAME_HEIGHT);
  lv_display_set_color_format(g_display, LV_COLOR_FORMAT_XRGB8888);
  lv_display_set_buffers(
    g_display, g_framebuffers[0], g_framebuffers[1], FRAME_BYTES, LV_DISPLAY_RENDER_MODE_FULL
  );
  lv_display_set_flush_cb(g_display, LvglFlush);
  lv_display_set_flush_wait_cb(g_display, LvglFlushWait);

  return XST_SUCCESS;
}

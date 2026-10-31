/**
 * @file
 *
 * @copyright Distributed under MIT Licence
 *   See https://github.com/house-of-abbey/Blackboard/blob/main/LICENCE.
 *
 * @brief Blackboard ESP32 Wi-Fi device driver
 *
 * @author J D Abbey & P A Abbey, 15 May 2026
 */

#include "bwifi.h"

#ifdef XPAR_XUARTPS_NUM_INSTANCES

#include <stdarg.h>
#include <stdio.h>
#include <xil_printf.h>
#include <xil_types.h>
#include <xstatus.h>
#include <xuartps_hw.h>

#include "btime.h"
#include "bwifi_proto.h"
#include "sleep.h"
#include "xgpiops.h"
#include "xil_exception.h"
#include "xparameters.h"
#include "xscugic.h"
#include "xuartps.h"

#define ESP32_BOOT_PIN 25
#define ESP32_EN_PIN   26

#define UART_ESP_BASEADDR XPAR_XUARTPS_0_BASEADDR
#define UART_ESP_IRQ_ID   XPAR_XUARTPS_0_INTR
#define SCUGIC_BASEADDR   XPAR_XSCUGIC_0_BASEADDR
#define GPIOPS_BASEADDR   XPAR_XGPIOPS_0_BASEADDR

static XUartPs UartESP;
static XGpioPs GpioPs;
static XScuGic ScuGic;

// ================= COBS =================

/** COBS encode data to buffer
        @param data Pointer to input data to encode
        @param length Number of bytes to encode
        @param buffer Pointer to encoded output buffer
        @return Encoded buffer length in bytes
        @note Does not output delimiter byte
        @ref https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
*/
static u32 cobs_encode(const void* data, u32 length, u8* buffer) {
  u8* encode = buffer;    // Encoded byte pointer
  u8* codep  = encode++;  // Output code pointer
  u8  code   = 1;         // Code value

  for (const u8* byte = (const u8*)data; length--; ++byte) {
    if (*byte) {  // Byte not zero, write it
      *encode++ = *byte, ++code;
    }

    if (!*byte || code == 0xff)  // Input is zero or block completed, restart
    {
      *codep = code, code = 1, codep = encode;
      if (!*byte || length) {
        ++encode;
      }
    }
  }
  *codep = code;  // Write final code value

  return (u32)(encode - buffer);
}

/** COBS decode data from buffer
 * @param buffer Pointer to encoded input bytes
 * @param length Number of bytes to decode
 * @param data Pointer to decoded output data
 * @return Number of bytes successfully decoded
 * @note Stops decoding if delimiter byte is found
 * @ref https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
 */
static u32 cobs_decode(const u8* buffer, u32 length, void* data) {
  const u8* byte   = buffer;     // Encoded input byte pointer
  u8*       decode = (u8*)data;  // Decoded output byte pointer

  for (u8 code = 0xff, block = 0; byte < buffer + length; --block) {
    if (block) {  // Decode block byte
      *decode++ = *byte++;
    } else {
      block = *byte++;                // Fetch the next block length
      if (block && (code != 0xff)) {  // Encoded zero, write it unless it's
                                      // delimiter.
        *decode++ = 0;
      }
      code = block;
      if (!code) {  // Delimiter code found
        break;
      }
    }
  }

  return (u32)(decode - (u8*)data);
}

// ================= Ring Buffer =================

#define RB_SIZE 2048

typedef struct {
  u8           buf[RB_SIZE];
  volatile int head;
  volatile int tail;
} ringbuf_t;

static ringbuf_t rb;

static inline void rb_push(ringbuf_t* r, u8 b) {
  int next = (r->head + 1) % RB_SIZE;
  if (next != r->tail) {
    r->buf[r->head] = b;
    r->head         = next;
  }
}

static inline int rb_pop(ringbuf_t* r, u8* b) {
  if (r->tail == r->head) {
    return 0;
  }
  *b      = r->buf[r->tail];
  r->tail = (r->tail + 1) % RB_SIZE;
  return 1;
}

// ================= Interrupt Handler =================

static void rx_to_rb() {
  u8  tmp[64];
  int len = XUartPs_Recv(&UartESP, tmp, sizeof(tmp));

  for (int i = 0; i < len; i++) {
    rb_push(&rb, tmp[i]);
  }
}

static void rx_handler(
  __attribute__((unused)) void* CallBackRef, u32 Event, __attribute__((unused)) u32 EventData
) {
  if (Event == XUARTPS_EVENT_RECV_DATA || Event == XUARTPS_EVENT_RECV_TOUT) {
    rx_to_rb();
  }
}

// ================= EVENT HANDLERS =================

static bwifi_data_handler_t   data_handler   = NULL;
static bwifi_log_handler_t    log_handler    = NULL;
static bwifi_status_handler_t status_handler = NULL;

// ================= PROCESS =================

static bwifi_state_t state      = BWIFI_STATE_UNINITIALISED;
static u32           ip_address = 0;

static void handle_packet(u8* packet, int len) {
  if (len < 1) {
    return;
  }

  // xil_printf("Packet: %x %d\r\n", packet[0], len);
  bwifi_packet_type_t type = (bwifi_packet_type_t)packet[0];
  switch (type) {
    case BWIFI_PACKET_TYPE_VERSION:
      if (len < 3) {
        return;
      }
      u16 version = packet[1] << 8 | packet[2];
      if (version != BWIFI_VERSION) {
        print("[ERROR]: Version Miss-match!\r\n");
        Xil_AssertVoidAlways();
      }
      break;
    case BWIFI_PACKET_TYPE_STATUS:
      if (len < 2) {
        return;
      }
      bwifi_status_t status = (bwifi_status_t)packet[1];
      switch (status) {
        case BWIFI_STATUS_INITIALISING:
          state = BWIFI_STATE_INITIALISING;
          // print("[DEBUG]: BWIFI_STATUS_INITIALISING\r\n");
          break;
        case BWIFI_STATUS_AUTH_REQUIRED:
          state = BWIFI_STATE_AUTH_REQUIRED;
          // print("[DEBUG]: BWIFI_STATUS_AUTH_REQUIRED\r\n");
          break;
        case BWIFI_STATUS_CONNECTING:
          state = BWIFI_STATE_CONNECTING;
          // print("[DEBUG]: BWIFI_STATUS_CONNECTING\r\n");
          break;
        case BWIFI_STATUS_CONNECTION_FAILED:
          state = BWIFI_STATE_AUTH_REQUIRED;
          // print("[DEBUG]: BWIFI_STATUS_CONNECTION_FAILED\r\n");
          break;
        case BWIFI_STATUS_CONNECTION_SUCCESSFUL:
          state = BWIFI_STATE_CONNECTED;
          // print("[DEBUG]: BWIFI_STATUS_CONNECTION_SUCCESSFUL\r\n");
          break;
        case BWIFI_STATUS_DISCONNECTED:
          state = BWIFI_STATE_CONNECTING;
          // print("[DEBUG]: BWIFI_STATUS_DISCONNECTED\r\n");
          break;
        default:
          break;
      }
      if (status_handler) {
        status_handler(status);
      }
      break;
    // case BWIFI_PACKET_TYPE_AUTH: We never receive this
    case BWIFI_PACKET_TYPE_LOG:
      if (len < 2) {
        return;
      }
      bwifi_log_level_t level = (bwifi_log_level_t)packet[1];
      const char*       msg   = (const char*)&packet[2];
      if (log_handler) {
        log_handler(level, msg);
      }
      break;
    case BWIFI_PACKET_TYPE_DATA:
      if (len < 1) {
        return;
      }
      const char* data = (const char*)&packet[1];
      if (data_handler) {
        data_handler(data, len - 1);
      }
      break;
    case BWIFI_PACKET_TYPE_ADDRESS:
      if (len < 5) {
        return;
      }
      ip_address = (packet[1] << 24) | (packet[2] << 16) | (packet[3] << 8) | packet[4];
      break;
    case BWIFI_PACKET_TYPE_TIME:
      if (len < 9) {
        return;
      }

      u64 time = ((u64)packet[1] << 56) | ((u64)packet[2] << 48) | ((u64)packet[3] << 40) |
                 ((u64)packet[4] << 32) | ((u64)packet[5] << 24) | ((u64)packet[6] << 16) |
                 ((u64)packet[7] << 8) | (u64)packet[8];
      // xil_printf("Time Sync: %llu\r\n", time);
      BTime_SetUnixTimeMs(time);
      break;
    default:
      break;
  }
}

#define FRAME_MAX 512

static u8  frame_buf[FRAME_MAX];
static int frame_len = 0;

static u8 decoded[FRAME_MAX];

static void process_uart_stream(void) {
  u8 Data;

  while (rb_pop(&rb, &Data)) {
    // outbyte(Data);
    if (Data == 0) {
      // End of COBS packet
      // xil_printf("\r\nFrame: %d\r\n", frame_len);

      if (frame_len > 0) {
        int out_len = cobs_decode(frame_buf, frame_len, decoded);

        handle_packet(decoded, out_len);
      }

      frame_len = 0;  // reset for next packet
    } else {
      if (frame_len < FRAME_MAX) {
        frame_buf[frame_len++] = Data;
      } else {
        // print("Overflow!\r\n");
        // overflow → drop packet
        frame_len = 0;
        // discard any detritus
        u8 d;
        while (rb_pop(&rb, &d)) {
          if (d == 0) {
            break;
          }
        }
      }
    }
  }
}

static u32 send_data(u8* data, u32 len) {
  // print("Sent: ");
  u32 sent = 0;

  while (sent < len) {
    sent += XUartPs_Send(&UartESP, &data[sent], len - sent);
    // outbyte(data[sent]);
    // sent += XUartPs_Send(&UartESP, &data[sent], 1);

    while (XUartPs_IsSending(&UartESP));
  }

  // print("\r\n");
  return sent;
  // return XUartPs_Send(&UartESP, data, length);
}

// ================= INITIALISATION =================

static int scugic_init() {
  XScuGic_Config* Config = XScuGic_LookupConfig(SCUGIC_BASEADDR);
  if (Config == NULL) {
    return XST_FAILURE;
  }
  int Status = XScuGic_CfgInitialize(&ScuGic, Config, Config->CpuBaseAddress);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }

  Xil_ExceptionInit();
  Xil_ExceptionRegisterHandler(
    XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, &ScuGic
  );
  Xil_ExceptionEnable();

  return XST_SUCCESS;
}

static void enable_uartps_interrupts() {
  XUartPs_SetInterruptMask(&UartESP, XUARTPS_IXR_RXFULL | XUARTPS_IXR_RXOVR | XUARTPS_IXR_TOUT);
}

static void disable_uartps_interrupts() { XUartPs_SetInterruptMask(&UartESP, 0); }

static int uartps_init() {
  XUartPs_Config* Config = XUartPs_LookupConfig(UART_ESP_BASEADDR);
  if (Config == NULL) {
    return XST_FAILURE;
  }
  int Status = XUartPs_CfgInitialize(&UartESP, Config, Config->BaseAddress);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }

  XUartPs_SetDataFormat(
    &UartESP, &(XUartPsFormat){
                .BaudRate = 115200,
                .DataBits = XUARTPS_FORMAT_8_BITS,
                .Parity   = XUARTPS_FORMAT_NO_PARITY,
                .StopBits = XUARTPS_FORMAT_1_STOP_BIT,
              }
  );

  XUartPs_SetHandler(&UartESP, rx_handler, NULL);

  Status = XScuGic_Connect(
    &ScuGic, UART_ESP_IRQ_ID, (Xil_ExceptionHandler)XUartPs_InterruptHandler, &UartESP
  );
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }

  XScuGic_Enable(&ScuGic, UART_ESP_IRQ_ID);

  enable_uartps_interrupts();

  return XST_SUCCESS;
}

static int gpiops_init() {
  XGpioPs_Config* Config = XGpioPs_LookupConfig(GPIOPS_BASEADDR);
  if (Config == NULL) {
    return XST_FAILURE;
  }
  int Status = XGpioPs_CfgInitialize(
    &GpioPs, Config,
    Config->BaseAddr
  );  // NB. different spelling, why‽
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }

  // Set BOOT and EN pins as outputs
  XGpioPs_SetDirectionPin(&GpioPs, ESP32_EN_PIN, 1);
  XGpioPs_SetOutputEnablePin(&GpioPs, ESP32_EN_PIN, 1);
  XGpioPs_SetDirectionPin(&GpioPs, ESP32_BOOT_PIN, 1);
  XGpioPs_SetOutputEnablePin(&GpioPs, ESP32_BOOT_PIN, 1);

  return XST_SUCCESS;
}

// ================= PUBLIC =================

void BWifi_EnterBootloader() {
  XGpioPs_WritePin(&GpioPs, ESP32_EN_PIN, 0);    // Pull EN low
  XGpioPs_WritePin(&GpioPs, ESP32_BOOT_PIN, 0);  // Pull IO0 low
  usleep(50000);                                 // Wait 50ms
  XGpioPs_WritePin(&GpioPs, ESP32_EN_PIN, 1);    // Pull EN high (reset)
  // IO0 remains low, ESP32 enters download mode

  state = BWIFI_STATE_UNINITIALISED;
}

void BWifi_Reset() {
  // Disable the interrupts UART interrupts
  disable_uartps_interrupts();

  XGpioPs_WritePin(&GpioPs, ESP32_EN_PIN, 0);    // Pull EN low
  XGpioPs_WritePin(&GpioPs, ESP32_BOOT_PIN, 1);  // Pull IO0 high
  usleep(50000);                                 // Wait 50ms
  XGpioPs_WritePin(&GpioPs, ESP32_EN_PIN, 1);    // Pull EN high (reset)
  // IO0 is high, ESP32 runs the loaded program

  state = BWIFI_STATE_BOOTING;

  // Handle all the logs from the bootloader starting our firmware.
  // Then count 4 0xFF
  u8  d;
  int count = 0;
  while (1) {
    if (XUartPs_Recv(&UartESP, &d, 1)) {
      // outbyte(d);
      if (d == 0xFF) {
        count++;
        if (count == 4) {
          break;
        }
      } else {
        count = 0;
      }
    }
  }

  enable_uartps_interrupts();
}

int BWifi_Initialize() {
  if (state != BWIFI_STATE_UNINITIALISED) {
    return XST_FAILURE;
  }

  // Initialize peripherals
  if (scugic_init() != XST_SUCCESS) {
    return XST_FAILURE;
  }
  if (uartps_init() != XST_SUCCESS) {
    return XST_FAILURE;
  }
  if (gpiops_init() != XST_SUCCESS) {
    return XST_FAILURE;
  }

  BWifi_Reset();

  return XST_SUCCESS;
}

int BWifi_Connect(char* ssid, char* password) {
  if (state == BWIFI_STATE_AUTH_SENT) {
    return XST_SUCCESS;
  }
  if (state < BWIFI_STATE_AUTH_REQUIRED) {
    return XST_FAILURE;
  }

  // Make packet
  u8 packet[254];
  packet[0] = BWIFI_PACKET_TYPE_AUTH;
  // copy ssid and password into packet
  u32 len   = 1 + strlen(ssid) + 1 + strlen(password) + 1;
  if (len > sizeof(packet)) {
    return XST_FAILURE;
  }
  memcpy(packet + 1, ssid, strlen(ssid));
  packet[1 + strlen(ssid)] = 0;
  memcpy(packet + 1 + strlen(ssid) + 1, password, strlen(password));
  packet[1 + strlen(ssid) + 1 + strlen(password)] = 0;

  // COBS encode packet
  u8  encoded[256];
  u32 cobs_len = cobs_encode(packet, len, encoded);
  if (cobs_len < 0 || cobs_len >= sizeof(encoded)) {
    return XST_FAILURE;
  }
  encoded[cobs_len++] = 0;

  // Send packet
  if (send_data(encoded, cobs_len) != cobs_len) {
    return XST_FAILURE;
  }

  state = BWIFI_STATE_AUTH_SENT;

  return XST_SUCCESS;
}

int BWifi_Send(u8* data) {
  // split long data into multiple packets
  u8* _data = data;
  while (*_data != 0) {
    u8 packet[254];
    packet[0] = BWIFI_PACKET_TYPE_DATA;
    // u32 len = 2 + strlen((char*)_data);
    // if (len > sizeof(packet)) {
    //   len = sizeof(packet);
    // }
    // memcpy(packet + 1, _data, len - 2);
    // packet[len - 1] = 0;
    // _data += len - 2;
    u32 len   = 1 + strlen((char*)_data);
    if (len > sizeof(packet)) {
      len = sizeof(packet);
    }
    memcpy(packet + 1, _data, len - 1);
    // packet[len - 1] = 0;
    _data += len - 1;
    u8  encoded[256];
    u32 cobs_len = cobs_encode(packet, len, encoded);
    if (cobs_len < 0 || cobs_len >= sizeof(encoded)) {
      return XST_FAILURE;
    }
    encoded[cobs_len++] = 0;
    u32 bytes_sent      = send_data(encoded, cobs_len);
    if (bytes_sent != cobs_len) {
      return XST_FAILURE;
    }
  }

  return XST_SUCCESS;
}

int BWifi_Log(bwifi_log_level_t level, char* msg) {
  // split long data into multiple packets
  char* data = msg;
  while (*data != 0) {
    char packet[254];
    packet[0] = BWIFI_PACKET_TYPE_LOG;
    packet[1] = level;
    u32 len   = 2 + strlen(data);
    if (len > sizeof(packet)) {
      len = sizeof(packet);
    }
    strncpy(packet + 2, data, len - 2);
    packet[len - 1] = 0;
    data += len - 2;
    u8  encoded[256];
    u32 cobs_len = cobs_encode(packet, len, encoded);
    if (cobs_len < 0 || cobs_len >= sizeof(encoded)) {
      return XST_FAILURE;
    }
    encoded[cobs_len++] = 0;
    if (send_data(encoded, cobs_len) != cobs_len) {
      return XST_FAILURE;
    }
  }

  return XST_SUCCESS;
}

int BWifi_Sendf(const char* format, ...) {
  va_list args;
  va_start(args, format);
  char msg[256];
  vsnprintf(msg, sizeof(msg), format, args);
  va_end(args);
  return BWifi_Send((u8*)msg);
}

int BWifi_Logf(bwifi_log_level_t level, const char* format, ...) {
  va_list args;
  va_start(args, format);
  char msg[256];
  vsnprintf(msg, sizeof(msg), format, args);
  va_end(args);
  return BWifi_Log(level, msg);
}

void BWifi_SetHandler(bwifi_data_handler_t handler) {
  if (handler) {
    data_handler = handler;
  }
}

void BWifi_SetLogHandler(bwifi_log_handler_t handler) {
  if (handler) {
    log_handler = handler;
  }
}

void BWifi_SetStatusHandler(bwifi_status_handler_t handler) {
  if (handler) {
    status_handler = handler;
  }
}

void BWifi_Loop() {
  if (XUartPs_IsReceiveData(UART_ESP_BASEADDR)) {
    rx_to_rb();
  }

  process_uart_stream();
}

bwifi_state_t BWifi_GetState() { return state; }

int BWifi_WaitConnected() {
  while (state != BWIFI_STATE_CONNECTED) {
    BWifi_Loop();
    if (state == BWIFI_STATE_AUTH_REQUIRED) {
      return XST_FAILURE;
    }
  }
  return XST_SUCCESS;
}

u32 BWifi_GetAddress() {
  if (state == BWIFI_STATE_CONNECTED) {
    return ip_address;
  }
  return 0;
}

#endif

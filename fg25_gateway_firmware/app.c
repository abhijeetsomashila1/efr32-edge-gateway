#include <stdio.h>
#include "sl_assert.h"
#include <string.h>
#include <stdbool.h>

#include "app.h"
#include "socket.h"
#include "arpa/inet.h"
#include "sl_cmsis_os2_common.h"
#include "sl_iostream.h"


extern sl_iostream_instance_info_t sl_iostream_instance_esp32_info;

#define UDP_DESTINATION_PORT 5000

//Global Variables
static sl_iostream_t *esp32_stream = NULL;
static int udp_socket_id = -1;
static sockaddr_in6_t udp_destination;

static void udp_init(void)
{
  int result;

  /* Create an IPv6 UDP socket */
  udp_socket_id = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

  if (udp_socket_id < 0) {
    printf("ERROR: UDP socket() failed\r\n");
    return;
  }

  printf("UDP socket created: %d\r\n", udp_socket_id);

  /* Clear destination structure */
  memset(&udp_destination, 0, sizeof(udp_destination));

  /* Destination = IPv6 */
  udp_destination.sin6_family = AF_INET6;

  /* Destination = UDP port 5000 */
  udp_destination.sin6_port = htons(UDP_DESTINATION_PORT);

  /*
   * Raspberry Pi Border Router IPv6 address.
   *
   * This is the address you showed earlier.
   */
  result = inet_pton(
      AF_INET6,
      "fd12:3456::92fd:9fff:feee:9d54",
      &udp_destination.sin6_addr);

  if (result != 1) {
    printf("ERROR: inet_pton() failed\r\n");

    close(udp_socket_id);
    udp_socket_id = -1;

    return;
  }

  printf("UDP destination configured\r\n");
}

static void udp_send_test(void)
{
  const char message[] = "hello from FG25";

  ssize_t result;

  if (udp_socket_id < 0) {
    printf("ERROR: UDP socket is not available\r\n");
    return;
  }

  result = sendto(
      udp_socket_id,
      message,
      sizeof(message) - 1,
      0,
      (const struct sockaddr *)&udp_destination,
      sizeof(udp_destination));

  if (result < 0) {
    printf("ERROR: sendto() failed\r\n");
  } else {
    printf("UDP packet sent: %ld bytes\r\n", (long)result);
  }
}


// static void esp32_uart_test(void)
// {
//   char c;

//   if (esp32_stream == NULL) {
//     return;
//   }

//   while (sl_iostream_getchar(esp32_stream, &c) == SL_STATUS_OK) {
//     printf("ESP32 RX: %c\r\n", c);
//   }
// }

// static void esp32_uart_process(void)
// {
//   char c;

//   while (sl_iostream_getchar(
//              sl_iostream_instance_esp32_info.handle,
//              &c) == SL_STATUS_OK) {

//     printf("ESP32 RX: %c\r\n", c);
//   }
// }

// static void esp32_uart_send_test(void)
// {
//   static uint32_t counter = 0;

//   const char message[] = "FG25 UART LOOPBACK\r\n";

//   sl_status_t status = sl_iostream_write(
//       sl_iostream_instance_esp32_info.handle,
//       message,
//       sizeof(message) - 1);

//   printf("UART TX test %lu, status=%ld\r\n",
//          (unsigned long)counter++,
//          (long)status);

// }

static void esp32_uart_task(void *args)
{
  (void)args;

  char c;

  char rx_line[128];
  size_t rx_length = 0;

  printf("ESP32 UART task started\r\n");

  while (1) {

    /*
     * This function can block while waiting for a character.
     *
     * That is OK here because this is a dedicated UART task.
     */
    if (sl_iostream_getchar(
          sl_iostream_instance_esp32_info.handle,
          &c) == SL_STATUS_OK) {

      /*
       * Ignore carriage return.
       */
      if (c == '\r') {
        continue;
      }

      /*
       * Newline = complete UART frame.
       */
      if (c == '\n') {

        rx_line[rx_length] = '\0';

        printf("ESP32 RX: %s\r\n", rx_line);

        rx_length = 0;
      }

      /*
       * Store normal characters.
       */
      else if (rx_length < (sizeof(rx_line) - 1)) {

        rx_line[rx_length++] = c;
      }

      /*
       * Buffer overflow.
       */
      else {

        printf("ESP32 UART buffer overflow\r\n");

        rx_length = 0;
      }
    }
  }
}

static const osThreadAttr_t esp32_uart_task_attr = {
  .name = "ESP32UART",
  .attr_bits = osThreadDetached,
  .cb_mem = NULL,
  .cb_size = 0,
  .stack_mem = NULL,
  .stack_size = 1024,
  .priority = osPriorityNormal,
  .tz_module = 0
};

void app_task(void *args)
{
   (void)args;

  sl_wisun_app_core_util_connect_and_wait();

  printf("Wi-SUN connected\r\n");

  udp_init();

  esp32_stream = sl_iostream_get_handle("esp32");

  if (esp32_stream == NULL) {
    printf("ERROR: ESP32 IO Stream not found\r\n");
    return;
  }

  printf("ESP32 IO Stream ready\r\n");

  /*
   * Create a dedicated UART task.
   */
  osThreadId_t uart_task_id = osThreadNew(
      esp32_uart_task,
      NULL,
      &esp32_uart_task_attr);

  if (uart_task_id == NULL) {
    printf("ERROR: Failed to create ESP32 UART task\r\n");
    return;
  }

  printf("ESP32 UART task created\r\n");

  /*
   * Main application/Wi-SUN task.
   */
  while (1) {

    sl_wisun_app_core_util_dispatch_thread();

    osDelay(10);
  }
}
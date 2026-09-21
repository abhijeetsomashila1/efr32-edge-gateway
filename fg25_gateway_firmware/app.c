#include <stdio.h>
#include <string.h>

#include "app.h"
#include "socket.h"
#include "arpa/inet.h"
#include "sl_cmsis_os2_common.h"

#define UDP_DESTINATION_PORT 5000

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

void app_task(void *args)
{
  (void)args;

  /*
   * Wait until the FG25 has successfully joined the Wi-SUN network.
   */
  sl_wisun_app_core_util_connect_and_wait();

  printf("Wi-SUN connected\r\n");

  /*
   * Create our own UDP client socket.
   */
  udp_init();

  while (1) {

    /*
     * Send one test packet.
     */
    udp_send_test();

    /*
     * Allow the Wi-SUN event system to process events.
     */
    sl_wisun_app_core_util_dispatch_thread();

    /*
     * Wait 10 seconds before sending another packet.
     */
    osDelay(10000);
  }
}
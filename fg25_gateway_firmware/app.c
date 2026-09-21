/***************************************************************************//**
 * @file app.c
 * @brief Application code
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdio.h>

#include "app.h"
#include "socket.h"
#include "arpa/inet.h"
#include <string.h>


// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
static int udp_sock = -1;
static sockaddr_in6_t br_addr;
// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

static void udp_init(void)
{
  udp_sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

  if (udp_sock < 0) {
    printf("Failed to create UDP socket\r\n");
    return;
  }

  memset(&br_addr, 0, sizeof(br_addr));

  br_addr.sin6_family = AF_INET6;
  br_addr.sin6_port = htons(5000);

  inet_pton(AF_INET6,
            "fd12:3456::92fd:9fff:feee:9d54", //Border Router IPv6
            &br_addr.sin6_addr);

  printf("UDP socket created\r\n");
}

static void udp_send_test(void)
{
  const char *msg = "hello from fg25";

  if (udp_sock < 0) {
    return;
  }

  int ret = sendto(udp_sock,
                   msg,
                   strlen(msg),
                   0,
                   (const struct sockaddr *)&br_addr,
                   sizeof(br_addr));

  printf("UDP send result = %d\r\n", ret);
}

/* App task function */
void app_task(void *args)
{
  (void) args;

  // connect to the wisun network
  sl_wisun_app_core_util_connect_and_wait();

   printf("Wi-SUN connected\r\n");

  udp_init();


  while (1) {
    ///////////////////////////////////////////////////////////////////////////
    // Put your application code here!                                       //
    ///////////////////////////////////////////////////////////////////////////
    udp_send_test();

    osDelay(10000);

    sl_wisun_app_core_util_dispatch_thread();
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

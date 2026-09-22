# FG25 Wi-SUN UART Gateway — Project Progress

## Project Overview

This project is a proof-of-concept gateway that will eventually forward serial data from an ESP32 through an EFR32FG25 over a Wi-SUN FAN 1.1 network to a Raspberry Pi Border Router.

### Target Architecture

```text
ESP32
  │ UART
  ▼
EFR32FG25 (BRD4271A)
  │ Wi-SUN / IPv6 / UDP
  ▼
Raspberry Pi
  │
  ▼
Application / Data Collector
```

---

## Hardware

- EFR32FG25 Radio Board — **BRD4271A**
- Silicon Labs Wireless Starter Kit — **WSTK**
- ESP32 development board
- Raspberry Pi running as a Wi-SUN Border Router

The BRD4271A is mounted on the WSTK.

---

## Software

- **Simplicity Studio:** 6
- **Simplicity SDK Suite:** v2025.12.3
- **Wi-SUN Stack:** 2.11.1
- **Wi-SUN FAN:** 1.1
- **Regional configuration:** EU533
- **Development environment:** VS Code + CMake / Simplicity Studio

---

# 1. GitHub Repository

Repository name:

```text
wisun-uart-gateway
```

The project is being tracked with Git/GitHub.

Current FG25 firmware project structure:

```text
fg25_gateway_firmware/
├── .vscode/
│   └── settings.json
├── autogen/
│   ├── AGENTS-SLAB.md
│   ├── linkerfile.ld
│   ├── rail_config.c
│   ├── rail_config.h
│   ├── sl_wisun_config.c
│   ├── sl_wisun_config.h
│   ├── sl_event_handler.c
│   ├── sl_event_handler.h
│   ├── sl_cli_*.c
│   ├── sl_cli_*.h
│   ├── sl_iostream_*.c
│   ├── sl_iostream_*.h
│   ├── sli_*_autogen.h
│   ├── sbom/
│   └── .slc_state/
├── cmake_gcc/
│   ├── CMakeLists.txt
│   ├── CMakePresets.json
│   ├── toolchain.cmake
│   ├── autogen_toolchain.cmake
│   ├── fg25_gateway_firmware.cmake
│   └── build/
├── config/
│   ├── FreeRTOSConfig.h
│   ├── pin_config.h
│   ├── sl_*_config.h
│   ├── rail/
│   └── wisun/
├── app.c
├── app.h
├── app_init.c
├── app_custom_callback.c
├── app_custom_callback.h
├── main.c
├── sl_wisun_regdb.c
├── fg25_gateway_firmware.slcp
├── fg25_gateway_firmware.slps
├── fg25_gateway_firmware.pintool
├── readme.md
└── vscode.conf
```

---

# 2. Starting Example

The project was created from the Silicon Labs example:

```text
Wi-SUN - SoC Socket
```

This example was selected because the final application requires IPv6 UDP socket communication.

The original application task was:

```c
void app_task(void *args)
{
  (void) args;

  sl_wisun_app_core_util_connect_and_wait();

  while (1) {
    sl_wisun_app_core_util_dispatch_thread();
  }
}
```

The application task is declared in `app.h` as:

```c
void app_task(void *args);
```

---

# 3. Existing Wi-SUN Border Router

The Raspberry Pi Border Router was already running before development began.

Other Wi-SUN nodes are already connected to it.

The FG25 was added to the existing network without changing the Border Router configuration.

Wi-SUN network name:

```text
SCRC_FAN11_FSK_TEST
```

Observed join sequence:

```text
[Join state: Acquire PAN Config (3)]
[Connecting to "SCRC_FAN11_FSK_TEST"]
[Join state: Configure Routing (4)]
[Join state: Operational (5)]
```

`Operational (5)` confirmed that the FG25 successfully joined the Wi-SUN network.

---

# 4. IPv6 Addresses

FG25 global IPv6 address:

```text
fd12:3456::b635:22ff:fe98:29aa
```

FG25 link-local IPv6 address:

```text
fe80::b635:22ff:fe98:29aa
```

Raspberry Pi / Border Router IPv6 address:

```text
fd12:3456::92fd:9fff:feee:9d54
```

The Border Router address is currently being used as the UDP destination for testing.

---

# 5. Wi-SUN Socket Example Status

The SoC Socket application showed:

```text
[UDP Server port: 1234]
UDP server socket() [3] done.
UDP server bind() [0] done.

[TCP Server port: 4567]
TCP server socket() [4] done.
TCP server bind() [0] done.
TCP server listen() [0] done.
[Waiting for connection request]
```

The example also exposes the following CLI commands:

```text
udp_client
socket_close
socket_write
socket_read
```

The UDP `socket_write` syntax is:

```text
wisun socket_write <Socket Id> <Remote address> <Remote port> <Data>
```

---

# 6. Wi-SUN Connectivity Test

The FG25 was tested against the Border Router using:

```text
wisun ping fd12:3456::92fd:9fff:feee:9d54
```

Result:

```text
PING fd12:3456::92fd:9fff:feee:9d54: 40 data bytes

[40 bytes from fd12:3456::92fd:9fff:feee:9d54: seq=256 time=122 ms]
[40 bytes from fd12:3456::92fd:9fff:feee:9d54: seq=512 time=97 ms]
[40 bytes from fd12:3456::92fd:9fff:feee:9d54: seq=768 time=96 ms]
[40 bytes from fd12:3456::92fd:9fff:feee:9d54: seq=1024 time=101 ms]

Ping statistics for fd12:3456::92fd:9fff:feee:9d54:
  Packets: Sent = 4, Received = 4, Lost = 0, (0% loss)

Approximate round trip times:
  Minimum = 96ms
  Maximum = 122ms
  Average = 104ms
```

This confirmed IPv6 connectivity between the FG25 and Border Router.

---

# 7. UDP CLI Test

A UDP client socket was created using:

```text
wisun udp_client
```

Result:

```text
[Socket created: 5]
```

A packet was then sent using:

```text
wisun socket_write 5 fd12:3456::92fd:9fff:feee:9d54 5000 hello
```

The Raspberry Pi successfully received:

```text
hello
```

This verified:

```text
FG25
  ↓
Wi-SUN
  ↓
Border Router
  ↓
UDP
  ↓
Raspberry Pi
```

---

# 8. Automatic UDP Transmission from Firmware

After the CLI test succeeded, the FG25 firmware was modified to create its own UDP socket and periodically send:

```text
hello from FG25
```

The application uses the socket API:

```c
socket()
sendto()
close()
```

and IPv6 address conversion using:

```c
inet_pton()
```

Current destination:

```text
IPv6:
fd12:3456::92fd:9fff:feee:9d54

UDP port:
5000
```

The FG25 VCOM output showed:

```text
UDP socket created: 3
UDP destination configured
ESP32 IO Stream ready
UDP packet sent: 15 bytes
```

The Raspberry Pi successfully received:

```text
Hello from fg25
```

This proves that the UDP path works from our own application code, not just through the CLI.

---

# 9. `app.c` Threading

The project uses CMSIS-RTOS2 infrastructure.

`app_init.c` includes:

```c
#include "sl_cmsis_os2_common.h"
```

and creates the application thread using:

```c
osThreadNew(app_task, ...);
```

The correct application task definition is:

```c
void app_task(void *args)
```

An earlier attempt incorrectly used:

```c
void *app_task(void *args)
```

which caused this compiler error:

```text
error: conflicting types for 'app_task'
```

because `app.h` declares:

```c
void app_task(void *args);
```

The return type was corrected.

---

# 10. Application Callbacks

`app_init.c` registers the following Wi-SUN callbacks:

```c
app_wisun_em_custom_callback_register(
    SL_WISUN_MSG_CONNECTED_IND_ID,
    app_custom_connected_callback);

app_wisun_em_custom_callback_register(
    SL_WISUN_MSG_SOCKET_DATA_IND_ID,
    app_custom_socket_data_callback);

app_wisun_em_custom_callback_register(
    SL_WISUN_MSG_SOCKET_DATA_SENT_IND_ID,
    app_custom_socket_data_sent_callback);
```

The callbacks in `app_custom_callback.c` are currently empty:

```c
void app_custom_connected_callback(sl_wisun_evt_t *evt)
{
  (void) evt;
}

void app_custom_socket_data_callback(sl_wisun_evt_t *evt)
{
  (void) evt;
}

void app_custom_socket_data_sent_callback(sl_wisun_evt_t *evt)
{
  (void) evt;
}
```

These callbacks are not currently being used for the main UDP transmit path.

---

# 11. ESP32 UART Interface

A second Silicon Labs IO Stream EUSART instance was added for communication with the ESP32.

The existing VCOM interface was kept unchanged.

The project now conceptually has two serial interfaces:

```text
EUSART0 / VCOM
    ↓
WSTK USB
    ↓
PC terminal / debug output
```

and:

```text
EUSART1 / ESP32
    ↓
ESP32 UART
```

---

# 12. ESP32 EUSART Configuration

The new IO Stream EUSART instance is named:

```text
esp32
```

Current configuration:

```text
Peripheral:     EUSART1
Baud rate:      115200
Data bits:      8
Parity:         None
Stop bits:      1
Flow control:   None
RX buffer:      32 bytes
```

Generated configuration includes:

```c
#define SL_IOSTREAM_EUSART_ESP32_BAUDRATE              115200

#define SL_IOSTREAM_EUSART_ESP32_PARITY \
    SL_IOSTREAM_EUSART_UART_NO_PARITY

#define SL_IOSTREAM_EUSART_ESP32_STOP_BITS \
    SL_IOSTREAM_EUSART_UART_STOP_BITS_1

#define SL_IOSTREAM_EUSART_ESP32_FLOW_CONTROL_TYPE \
    SL_IOSTREAM_EUSART_UART_FLOW_CTRL_NONE

#define SL_IOSTREAM_EUSART_ESP32_RX_BUFFER_SIZE        32
```

Reception while sleeping is enabled:

```c
#define SL_IOSTREAM_EUSART_ESP32_RESTRICT_ENERGY_MODE_TO_ALLOW_RECEPTION 1
```

---

# 13. ESP32 UART Pin Configuration

The current pin configuration is:

```text
Peripheral: EUSART1

TX = PA07
RX = PA06

CTS = None
RTS = None
```

The generated configuration confirms:

```c
#define SL_IOSTREAM_EUSART_ESP32_TX_PORT SL_GPIO_PORT_A
#define SL_IOSTREAM_EUSART_ESP32_TX_PIN  7

#define SL_IOSTREAM_EUSART_ESP32_RX_PORT SL_GPIO_PORT_A
#define SL_IOSTREAM_EUSART_ESP32_RX_PIN  6
```

The planned WSTK breakout mapping is:

```text
PA6 → P44
PA7 → P45
```

Therefore the intended physical wiring is:

```text
ESP32 TX  →  FG25 PA6 / EUSART1 RX / P44

ESP32 RX  ←  FG25 PA7 / EUSART1 TX / P45

ESP32 GND →  FG25 GND
```

TX and RX are intentionally crossed.

---

# 14. Generated EUSART Files

The second EUSART instance caused Simplicity Studio to generate files including:

```text
autogen/sl_iostream_init_eusart_instances.c
autogen/sl_iostream_init_eusart_instances.h
config/sl_iostream_eusart_esp32_config.h
```

The generated source contains the initialization of the ESP32 EUSART instance, including:

- EUSART peripheral
- RX buffer
- baud rate
- flow control
- TX pin
- RX pin
- RX interrupt
- TX interrupt
- DMA configuration

The generated configuration confirms:

```text
EUSART1
PA7 → TX
PA6 → RX
```

---

# 15. EUSART Configuration Build Test

After adding the second EUSART instance and changing its pins to PA6/PA7, the project built successfully.

The firmware also continued to operate normally after flashing.

Observed VCOM output:

```text
UDP socket created: 3
UDP destination configured
ESP32 IO Stream ready
UDP packet sent: 15 bytes
```

The existing UDP server also continued to initialize:

```text
[UDP Server port: 1234]
UDP server socket() [4] done.
UDP server bind() [0] done.

[TCP Server port: 4567]
TCP server socket() [5] done.
TCP server bind() [0] done.
TCP server listen() [0] done.
[Waiting for connection request]
```

The Raspberry Pi continued receiving:

```text
Hello from fg25
```

Therefore, the addition of the second EUSART instance did not break the existing Wi-SUN application.

---

# 16. Current Project Status

The following items have been successfully verified:

```text
✅ BRD4271A + WSTK setup
✅ Simplicity Studio 6
✅ Simplicity SDK Suite v2025.12.3
✅ Wi-SUN FAN 1.1
✅ EU533
✅ Wi-SUN Stack 2.11.1
✅ FG25 joins existing Wi-SUN Border Router
✅ IPv6 connectivity to Border Router
✅ CLI UDP transmission
✅ Firmware UDP transmission
✅ Raspberry Pi receives firmware UDP packets
✅ Second IO Stream EUSART instance added
✅ EUSART1 configured for ESP32
✅ PA6/PA7 selected for ESP32 UART
✅ Firmware builds successfully
✅ Existing Wi-SUN/UDP functionality remains operational
```

---

# 17. Immediate Next Task

The next milestone is:

```text
ESP32 → FG25 UART reception
```

The first goal is to verify:

```text
ESP32
  │
  │ 115200 8N1
  ▼
FG25 EUSART1 RX (PA6)
  │
  ▼
IO Stream "esp32"
  │
  ▼
app.c
  │
  ▼
VCOM debug output
```

The ESP32 should initially send a simple test message such as:

```text
HELLO FROM ESP32
```

The FG25 should display the received data on VCOM.

Only after UART reception is verified will the received data be forwarded through the Wi-SUN UDP socket.

---

# 18. Final Intended Architecture

The completed system will look like:

```text
                  ┌────────────────────┐
                  │       ESP32        │
                  │                    │
                  │ Sensor/Application │
                  └─────────┬──────────┘
                            │
                            │ UART
                            ▼
                  ┌────────────────────┐
                  │     EFR32FG25      │
                  │     BRD4271A       │
                  │                    │
                  │ EUSART1            │
                  │    ↓               │
                  │ UART Frame Buffer  │
                  │    ↓               │
                  │ sendto()           │
                  │    ↓               │
                  │ Wi-SUN FAN 1.1     │
                  └─────────┬──────────┘
                            │
                            │ IPv6 / UDP
                            ▼
                  ┌────────────────────┐
                  │ Raspberry Pi       │
                  │ Border Router      │
                  └─────────┬──────────┘
                            │
                            ▼
                  ┌────────────────────┐
                  │ UDP Application     │
                  │ / Data Collector   │
                  └────────────────────┘
```

---

# 19. Planned UART Data Format

For the initial test, plain text will be used:

```text
HELLO FROM ESP32\n
```

Later, the UART payload can be changed to a structured format such as JSON:

```json
{
  "node": "esp32",
  "temp": 25.4,
  "humidity": 61
}
```

or a compact binary protocol if bandwidth, latency, or memory requirements make that preferable.

---

# 20. Development Strategy

The project is being developed incrementally:

```text
1. Verify Wi-SUN join
        ↓
2. Verify IPv6 connectivity
        ↓
3. Verify UDP using CLI
        ↓
4. Verify UDP using firmware
        ↓
5. Configure ESP32 EUSART
        ↓
6. Verify ESP32 → FG25 UART
        ↓
7. Buffer UART frames
        ↓
8. Forward UART frames with sendto()
        ↓
9. Verify complete ESP32 → FG25 → Wi-SUN → RPi path
        ↓
10. Add final data format / application processing
```

This approach keeps each subsystem independently testable.

---

# 21. Current Checkpoint

At the current checkpoint:

```text
Wi-SUN network       → WORKING
IPv6 routing         → WORKING
UDP transmission     → WORKING
Raspberry Pi RX      → WORKING
ESP32 EUSART setup   → WORKING
Firmware build       → WORKING
```

The next implementation step is to connect the ESP32 to the configured EUSART1 pins and implement UART reception in `app.c`.

No changes to the working Wi-SUN network configuration are required for the UART test.
# Wi-SUN UART Gateway

A proof-of-concept project that forwards data from an ESP32 to a Raspberry Pi over a Wi-SUN network using an EFR32FG25 node.

## Architecture

```text
ESP32
  │ UART
  ▼
EFR32FG25 (Wi-SUN Node)
  │ UDP / IPv6
  ▼
Raspberry Pi (Wi-SUN Border Router)
  │
  ▼
Application / Data Collector
```

## Goal

The objective of this project is to:

1. Receive data from an ESP32 over UART.
2. Forward the received data through a Wi-SUN FAN 1.1 network.
3. Deliver the data to a Raspberry Pi connected through a Wi-SUN Border Router.

## Hardware

* Silicon Labs EFR32FG25 Explorer Kit (BRD4271A)
* ESP32 Development Board
* Raspberry Pi running a Wi-SUN Border Router
* UART connection between ESP32 and EFR32FG25

## Software

* Simplicity Studio 6
* Simplicity SDK Suite v2025.12.3
* Wi-SUN FAN 1.1
* Python 3 (for Raspberry Pi receiver)

## Repository Structure

```text
.
├── fg25/          # EFR32FG25 firmware
├── esp32/         # ESP32 firmware
├── raspberrypi/   # UDP receiver and utilities
└── docs/          # Design notes and documentation
```

## Development Roadmap

* [ ] Create Wi-SUN SoC Socket baseline application
* [ ] Join FAN 1.1 network
* [ ] Send test UDP packets to Raspberry Pi
* [ ] Add UART interface on EFR32FG25
* [ ] Receive UART frames from ESP32
* [ ] Forward UART frames over Wi-SUN
* [ ] Parse and process data on Raspberry Pi

## Current Status

🚧 Project setup and initial Wi-SUN integration in progress.

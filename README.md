# WiFi Control for Icom IC-PCR1000 using ESP32

>


**A modern wireless web interface to control the Icom IC-PCR1000 communications receiver**

</div>

## Description

This project enables **wireless control** of the Icom IC-PCR1000 receiver using an **ESP32** microcontroller.

The ESP32 runs in Access Point (AP) mode, creating its own WiFi network and hosting a responsive web interface. The clean, mobile-friendly UI allows full real-time control of the receiver from any smartphone, tablet, or computer — without needing a PC or proprietary software.

Communication is handled via WebSockets for fast and responsive performance.

## Key Features

- Modern, professional web interface with ICOM-inspired design
- Fully responsive (optimized for mobile devices)
- Real-time control using WebSockets
- Numeric keypad for direct frequency entry
- 5 memory slots with save & recall
- Graphical S-Meter with color gradient
- Volume and Squelch sliders
- Mode and filter selection
- Toggle functions: NB, AGC, ATT, ANF
- Automatic PCR1000 initialization
- Standalone operation (no computer required)

## Hardware Required

- ESP32 development board
- Icom IC-PCR1000 receiver
- **MAX232** RS232-to-TTL level converter (required)
- DB9 serial cable
- Jumper wires (Dupont)
- Power supply for the receiver

## Wiring

### ESP32 → MAX232

| ESP32       | MAX232     |
|-------------|------------|
| GPIO17 (TX) | RX         |
| GPIO16 (RX) | TX         |
| GND         | GND        |
| 5V or 3.3V  | VCC        |

**Note:** TX and RX lines must be crossed.

### MAX232 → IC-PCR1000 (DB9)

| MAX232   | DB9 Pin       |
|----------|---------------|
| TX       | Pin 2 (RX)    |
| RX       | Pin 3 (TX)    |
| GND      | Pin 5         |

## Installation

### 1. Arduino IDE Setup

Add ESP32 board support in **File → Preferences**:

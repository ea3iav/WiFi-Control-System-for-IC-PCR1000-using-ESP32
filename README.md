# WiFi Control for Icom IC-PCR1000 using ESP32

<div align="center">

**A modern wireless web interface for the Icom IC-PCR1000 communications receiver**

</div>

## Description

This project allows **wireless control** of the classic Icom IC-PCR1000 receiver using an ESP32 microcontroller.

The ESP32 creates its own WiFi Access Point and hosts a clean, responsive web interface optimized for smartphones and tablets. Thanks to WebSockets, all commands are sent and received in real time.

No PC or original software is required — full control directly from your mobile browser.

## ✨ Key Features

- Modern and professional UI with ICOM-inspired design  
- Fully responsive mobile interface  
- Real-time communication via WebSockets  
- Numeric keypad for direct frequency input  
- Graphical S-Meter with color gradient  
- 5 memory slots (save & recall)  
- Volume and Squelch sliders  
- Mode selection (AM, NFM, WFM, USB, LSB, CW)  
- Filter selection (2.8kHz to 230kHz)  
- Toggle functions: NB, AGC, ATT, ANF  
- Automatic receiver initialization  
- Standalone operation  

## 🛠️ Hardware Required

- ESP32 development board  
- Icom IC-PCR1000 receiver  
- **MAX232** level converter (**mandatory**)  
- DB9 serial cable  
- Jumper wires (Dupont)  
- Stable power supply for the PCR1000  

## 🔌 Wiring

### ESP32 → MAX232

| ESP32         | MAX232   |
|---------------|----------|
| GPIO17 (TX)   | RX       |
| GPIO16 (RX)   | TX       |
| GND           | GND      |
| 5V or 3.3V    | VCC      |

**Important:** TX and RX must be crossed.

### MAX232 → IC-PCR1000 (DB9)

| MAX232   | DB9 Pin      |
|----------|--------------|
| TX       | Pin 2 (RX)   |
| RX       | Pin 3 (TX)   |
| GND      | Pin 5        |
Put a jumper between pin 7-8 on the DB9
## 🚀 Installation

### 1. Arduino IDE Setup

Add the ESP32 board URL in **File → Preferences**:

### 2. Board Selection

**Tools → Board → ESP32 Arduino → ESP32 Dev Module**

### 3. Upload

- Connect the ESP32 via USB  
- Select the correct COM port  
- Click **Upload**

## 📡 How to Connect

1. Power on the ESP32  
2. Connect to the WiFi network:  
   - **SSID**: `prc1000`  
   - **Password**: `12345678`  
3. Open your browser and go to:
        http://192.168.4.1
   
## 🎛️ Interface Overview

- **Frequency**: Tap the large display to open numeric keypad  
- **Tuning**: F+ / F− buttons + Step selector (1kHz to 100kHz)  
- **Modes**: AM • NFM • WFM • USB • LSB • CW  
- **Filters**: 2.8k • 6k • 15k • 50k • 230k  
- **Functions**: NB • AGC • ATT • ANF (toggle)  
- **Audio**: Smooth Volume and Squelch sliders  
- **S-Meter**: Real-time bar with green → yellow → red gradient  
- **Memories**: 5 slots (M1–M5) — Press **M** to save current settings  

## 📋 Important Notes

- The **MAX232** is required. Direct connection from ESP32 to PCR1000 will **not** work.  
- Baud rate: **9600 bps**  
- Verify all wiring before powering on  
- Use a stable 12V power supply for the receiver  

---

**Developed by EA3IAV**


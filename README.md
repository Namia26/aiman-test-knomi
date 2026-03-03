# KNOMI Display Project

Custom firmware for the **BTT KNOMI V2** smart display, built from scratch with PlatformIO + Arduino framework.

---

## What is KNOMI V2?

KNOMI V2 is a small round smart display made by BigTreeTech (BTT), designed for 3D printers. It has a round 240×240 screen, touch input, Wi-Fi, a built-in accelerometer, and an optional camera connector. This project repurposes it as a general-purpose display you can program freely.

---

## MCU

**ESP32-S3R8**
- Dual-core Xtensa LX7 @ 240MHz
- 8MB PSRAM (external, octal SPI)
- 16MB Flash
- Wi-Fi + Bluetooth 5
- Native USB (can also use USB-to-serial via CH340)

---

## Hardware Components

### 1. Display — GC9A01
- **Type:** Round TFT LCD, 240×240 pixels
- **Interface:** SPI
- **Driver chip:** GC9A01
- **Library:** `TFT_eSPI` by Bodmer
- **Config file:** `src/tft_setup.h` (must be included before `<TFT_eSPI.h>`)
- **⚠ Important:** On ESP32-S3, `USE_FSPI_PORT` must be defined in `tft_setup.h` or the board crashes on boot with an address error (0x00000010). This is the #1 gotcha for this board.
- **⚠ Important:** `tft.invertDisplay(1)` must be called or colours are inverted.

| Pin | GPIO |
|-----|------|
| MOSI | 14 |
| SCLK | 18 |
| CS | 20 |
| DC | 19 |
| RST | 21 |

---

### 2. Backlight — AW9346
- **Type:** LED controller IC
- **Interface:** Single GPIO, pulse protocol (NOT standard PWM)
- **16 brightness levels** — controlled by pulsing the pin LOW
- **Do not use analogWrite or ledcWrite** — it will not work
- **Pin:** GPIO 12
- Use the `tft_backlight_init()` and `tft_set_backlight(0–16)` helper functions

---

### 3. Touch Screen — CST816S
- **Type:** Capacitive single-touch
- **Interface:** I2C (Bus 0)
- **I2C Address:** `0x15`
- **Library:** `CST816S` (search PlatformIO registry)
- **Supports:** tap, swipe, long press gestures

| Pin | GPIO |
|-----|------|
| SDA | 2 |
| SCL | 1 |
| IRQ (interrupt) | 17 |
| RST (reset) | 16 |

```cpp
// Example usage
#include "CST816S.h"
CST816S touch(16, 17, &Wire); // RST, IRQ, Wire bus
Wire.begin(2, 1);              // SDA, SCL
touch.begin();
```

---

### 4. Accelerometer — LIS2DW12
- **Type:** 3-axis MEMS accelerometer
- **Interface:** I2C (Bus 0, shared with touch)
- **I2C Address:** `0x19`
- **Library:** `LIS2DW12` by STMicroelectronics / stm32duino
- **⚠ Important:** `begin()` returns `0` on **success** (not 1 like most Arduino libs) — this is STM32Duino style
- Built directly onto the KNOMI board, no external wiring needed

| Pin | GPIO |
|-----|------|
| SDA | 2 |
| SCL | 1 |

```cpp
// Example usage
#include <LIS2DW12Sensor.h>
LIS2DW12Sensor accel(&Wire);
Wire.begin(2, 1);
if (accel.begin() == 0) {  // 0 = success
    accel.Enable_X();
}
```

---

### 5. Camera — OV2640 (optional, external connector)
- **Type:** 2MP image sensor
- **Interface:** DVP parallel (8-bit data bus) + SCCB control
- **SCCB** is I2C-compatible, uses a **separate I2C bus (Bus 1)**
- **Library:** `espressif/esp32-camera`
- **Output format:** BGR565 raw (not RGB565 — R and B channels are swapped!)
- **Not soldered on board** — requires plugging in an OV2640 module to the connector

| Pin | GPIO | Notes |
|-----|------|-------|
| PWDN | 10 | Power down |
| RESET | 11 | Hardware reset |
| XCLK | 7 | Clock input to camera |
| SIOD (SDA) | 4 | SCCB data (I2C1) |
| SIOC (SCL) | 3 | SCCB clock (I2C1) |
| D0–D7 | 45,42,38,40,41,39,47,5 | Pixel data |
| VSYNC | 15 | Frame sync |
| HREF | 9 | Line sync |
| PCLK | 48 | Pixel clock |

---

## I2C Buses Summary

| Bus | SDA | SCL | Devices |
|-----|-----|-----|---------|
| I2C0 (Wire) | GPIO 2 | GPIO 1 | Touch (CST816S @ 0x15), Accelerometer (LIS2DW12 @ 0x19) |
| I2C1 | GPIO 4 | GPIO 3 | Camera SCCB (OV2640) |

---

## Project File Structure

```
knomi_display/
├── platformio.ini      — board, libraries, build flags
├── README.md           — this file
└── src/
    ├── main.cpp        — your application code goes here
    ├── tft_setup.h     — TFT_eSPI display config (pins, driver, fonts)
    └── pins.h          — all pin number definitions for quick reference
```

---

## How to Set Up

1. Install [VSCode](https://code.visualstudio.com/)
2. Install the **PlatformIO IDE** extension in VSCode
3. Open this folder (`File → Open Folder`)
4. PlatformIO auto-installs all libraries on first build
5. Connect KNOMI via USB
6. Check your COM port in Device Manager, update `upload_port` in `platformio.ini` if not COM51
7. Click **Upload** (→) in the bottom toolbar

---

## Common Errors

| Error | Cause | Fix |
|-------|-------|-----|
| Crash at `0x00000010` on boot | Missing `USE_FSPI_PORT` | Already defined in `tft_setup.h` |
| Screen is all white or colours wrong | `invertDisplay()` not called | Add `tft.invertDisplay(1)` in setup |
| Garbled serial output | Wrong baud rate in monitor | Use 115200 |
| Upload fails — port not found | Board not in bootloader mode | Hold BOOT, press RESET, then upload |
| `LIS2DW12 begin()` returns 1 but sensor works | Expecting Arduino-style (1=ok) | For this lib, `0 = success` |
| Camera colours wrong (red looks blue) | OV2640 outputs BGR565, not RGB565 | Swap R and B channels in software |

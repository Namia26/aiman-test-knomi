#pragma once

// ============================================================
//  KNOMI V2 Pin Reference
//  MCU: ESP32-S3R8 (8MB PSRAM, 16MB Flash)
// ============================================================

// ── TFT Display (GC9A01 round 240×240) ──────────────────────
#define PIN_TFT_MOSI    14
#define PIN_TFT_SCLK    18
#define PIN_TFT_CS      20
#define PIN_TFT_DC      19
#define PIN_TFT_RST     21

// ── Backlight (AW9346 LED controller, pulse protocol) ───────
//    NOT a simple PWM pin — use the tft_set_backlight() func
//    16 brightness levels, pulse LOW to decrease
#define PIN_LCD_BL      12

// ── I2C Bus 0 — Touch + Accelerometer ───────────────────────
#define PIN_I2C0_SDA     2
#define PIN_I2C0_SCL     1

// ── Touch Screen (CST816S, I2C0 address 0x15) ───────────────
#define PIN_TOUCH_IRQ   17   // interrupt, active LOW
#define PIN_TOUCH_RST   16   // reset, active LOW

// ── Accelerometer (LIS2DW12, I2C0 address 0x19) ─────────────
//    Uses PIN_I2C0_SDA / PIN_I2C0_SCL above
//    NOTE: begin() returns 0 on SUCCESS (STM32Duino style)

// ── I2C Bus 1 — Camera SCCB ─────────────────────────────────
#define PIN_I2C1_SDA     4
#define PIN_I2C1_SCL     3

// ── Camera (OV2640, DVP parallel interface) ──────────────────
#define PIN_CAM_PWDN    10
#define PIN_CAM_RESET   11
#define PIN_CAM_XCLK     7
#define PIN_CAM_SIOD     4   // SCCB SDA = I2C1
#define PIN_CAM_SIOC     3   // SCCB SCL = I2C1
#define PIN_CAM_D0      45
#define PIN_CAM_D1      42
#define PIN_CAM_D2      38
#define PIN_CAM_D3      40
#define PIN_CAM_D4      41
#define PIN_CAM_D5      39
#define PIN_CAM_D6      47
#define PIN_CAM_D7       5
#define PIN_CAM_VSYNC   15
#define PIN_CAM_HREF     9
#define PIN_CAM_PCLK    48

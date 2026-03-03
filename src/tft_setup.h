#pragma once

#define USER_SETUP_LOADED
#define USER_SETUP_ID       46

// Display driver
#define GC9A01_DRIVER
#define TFT_WIDTH           240
#define TFT_HEIGHT          240

// SPI pins (KNOMI V2)
#define TFT_MOSI            14
#define TFT_SCLK            18
#define TFT_CS              20
#define TFT_DC              19
#define TFT_RST             21

// ESP32-S3 requires this to avoid crash on boot
#define USE_FSPI_PORT

// Fonts
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF   // FreeFonts (setFreeFont)

// SPI speed
#define SPI_FREQUENCY       80000000
#define SPI_READ_FREQUENCY   5000000

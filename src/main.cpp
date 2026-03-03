#include <Arduino.h>
#include "tft_setup.h"
#include <TFT_eSPI.h>
#include "pins.h"
#include <CST816S.h>

TFT_eSPI tft;
CST816S touch(PIN_I2C0_SDA, PIN_I2C0_SCL, PIN_TOUCH_RST, PIN_TOUCH_IRQ);
static int8_t bl_level = -1;
static bool inverted = true;
static uint32_t last_touch_ms = 0;

void tft_backlight_init()
{
    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_LCD_BL, LOW);
    delay(3);
    bl_level = 0;
}

void tft_set_backlight(int8_t to)
{
    if (to > 16)
        to = 16;
    if (to < 0)
        to = 0;
    if (bl_level == to)
        return;
    if (to == 0)
    {
        digitalWrite(PIN_LCD_BL, LOW);
        delay(3);
        bl_level = 0;
        return;
    }
    if (bl_level <= 0)
    {
        digitalWrite(PIN_LCD_BL, HIGH);
        delayMicroseconds(25);
        bl_level = 16;
    }
    if (bl_level < to)
        bl_level += 16;
    int8_t p = bl_level - to;
    for (int8_t i = 0; i < p; i++)
    {
        digitalWrite(PIN_LCD_BL, LOW);
        delayMicroseconds(1);
        digitalWrite(PIN_LCD_BL, HIGH);
        delayMicroseconds(1);
    }
    bl_level = to;
}

void setup()
{
    touch.begin();

    tft.begin();
    tft.invertDisplay(inverted);
    tft.fillScreen(TFT_BLACK);

    tft_backlight_init();
    tft_set_backlight(16);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(&FreeMono12pt7b);
    tft.drawString("Hello World", 120, 120);
}

void loop()
{
    if (touch.available() && (millis() - last_touch_ms > 300)) {
        last_touch_ms = millis();
        inverted = !inverted;
        tft.invertDisplay(inverted);
    }
}

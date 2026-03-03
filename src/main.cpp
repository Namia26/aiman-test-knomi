#include <Arduino.h>
#include <TFT_eSPI.h>

#define LCD_BL_PIN  12

TFT_eSPI tft;
static int8_t bl_level = -1;

void tft_backlight_init() {
    pinMode(LCD_BL_PIN, OUTPUT);
    digitalWrite(LCD_BL_PIN, LOW);
    delay(3);
    bl_level = 0;
}

void tft_set_backlight(int8_t to) {
    if (to > 16) to = 16;
    if (to < 0)  to = 0;
    if (bl_level == to) return;
    if (to == 0) { digitalWrite(LCD_BL_PIN, LOW); delay(3); bl_level = 0; return; }
    if (bl_level <= 0) { digitalWrite(LCD_BL_PIN, HIGH); delayMicroseconds(25); bl_level = 16; }
    if (bl_level < to) bl_level += 16;
    int8_t p = bl_level - to;
    for (int8_t i = 0; i < p; i++) {
        digitalWrite(LCD_BL_PIN, LOW);  delayMicroseconds(1);
        digitalWrite(LCD_BL_PIN, HIGH); delayMicroseconds(1);
    }
    bl_level = to;
}

void setup() {
    tft.begin();
    tft.invertDisplay(1);
    tft.fillScreen(TFT_BLACK);

    tft_backlight_init();
    tft_set_backlight(16);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(&FreeMono12pt7b);
    tft.drawString("Hello World", 120, 120);
}

void loop() {
}

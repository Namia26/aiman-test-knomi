#include <Arduino.h>
#include "tft_setup.h"
#include <TFT_eSPI.h>
#include "pins.h"
#include <Wire.h>
#include <CST816S.h>
#include <LIS2DW12Sensor.h>
#include <math.h>

TFT_eSPI       tft;
TFT_eSprite    spr(&tft);
CST816S        touch(PIN_I2C0_SDA, PIN_I2C0_SCL, PIN_TOUCH_RST, PIN_TOUCH_IRQ);
LIS2DW12Sensor accel(&Wire);

static int8_t   bl_level      = -1;
static bool     inverted      = true;
static uint32_t last_touch_ms = 0;
static uint32_t last_frame_ms = 0;
static uint32_t last_accel_ms = 0;
static uint8_t  frame         = 0;

enum DriveState { IDLE, SMOOTH, ACCEL, CORNER };
static DriveState state = IDLE;

// ── Backlight ────────────────────────────────────────────────────
void tft_backlight_init() {
    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_LCD_BL, LOW);
    delay(3);
    bl_level = 0;
}

void tft_set_backlight(int8_t to) {
    if (to > 16) to = 16;
    if (to < 0)  to = 0;
    if (bl_level == to) return;
    if (to == 0) { digitalWrite(PIN_LCD_BL, LOW); delay(3); bl_level = 0; return; }
    if (bl_level <= 0) { digitalWrite(PIN_LCD_BL, HIGH); delayMicroseconds(25); bl_level = 16; }
    if (bl_level < to) bl_level += 16;
    int8_t p = bl_level - to;
    for (int8_t i = 0; i < p; i++) {
        digitalWrite(PIN_LCD_BL, LOW);  delayMicroseconds(1);
        digitalWrite(PIN_LCD_BL, HIGH); delayMicroseconds(1);
    }
    bl_level = to;
}

// ── State classifier (ax=X axis, az=Z axis, both in mg) ─────────
// Y axis is ignored — gravity (~-970mg) sits there due to board orientation.
// X and Z are the two horizontal axes. Thresholds to tune after axis test:
//   IDLE threshold : 180mg  (idle noise is ~106mg, give margin)
//   ACCEL threshold: 300mg  (forward/brake axis — tune after testing)
//   CORNER threshold: 300mg (lateral axis       — tune after testing)
DriveState classify(int32_t ax, int32_t az) {
    float total = sqrtf((float)(ax * ax) + (float)(az * az));
    if (total < 180.0f)            return IDLE;
    if (fabsf((float)az) > 300.0f) return ACCEL;   // Z = forward/brake axis (confirmed)
    if (fabsf((float)ax) > 300.0f) return CORNER;  // X = lateral axis (confirm with side jerk)
    return SMOOTH;
}

// ── Shared tyre drawing helper ───────────────────────────────────
void draw_tyre(int cx, int cy, int R_out, int R_rim, int R_hub, float rot) {
    spr.fillCircle(cx, cy, R_out,     0x2104);  // tyre edge (very dark)
    spr.fillCircle(cx, cy, R_out - 7, 0x3186);  // tyre body (dark gray)

    // Tread dots rotating with wheel
    for (int i = 0; i < 10; i++) {
        float a = rot + i * 2.0f * M_PI / 10.0f;
        int x = cx + (R_out - 5) * cosf(a);
        int y = cy + (R_out - 5) * sinf(a);
        spr.fillRect(x - 3, y - 3, 6, 6, 0x1082);
    }

    // Rim
    spr.fillCircle(cx, cy, R_rim, 0xB5B6);  // silver

    // 5 spokes
    for (int i = 0; i < 5; i++) {
        float a = rot + i * 2.0f * M_PI / 5.0f;
        int x1 = cx + R_hub * cosf(a),        y1 = cy + R_hub * sinf(a);
        int x2 = cx + (R_rim - 5) * cosf(a),  y2 = cy + (R_rim - 5) * sinf(a);
        spr.drawWideLine(x1, y1, x2, y2, 5, 0x9CD3);
    }

    // Hub cap
    spr.fillCircle(cx, cy, R_hub, 0xDEDB);
    spr.drawCircle(cx, cy, R_hub, 0x7BEF);
}

// ── IDLE: Sleepy face + floating ZZZ ────────────────────────────
void draw_sleepy() {
    spr.fillSprite(TFT_BLACK);

    // Face
    spr.fillCircle(120, 112, 68, 0xFEA0);
    spr.drawCircle(120, 112, 68, 0xFD20);

    // Closed eyes (rounded rects)
    spr.fillRoundRect(78,  96, 30, 10, 5, TFT_BLACK);
    spr.fillRoundRect(132, 96, 30, 10, 5, TFT_BLACK);

    // Blush cheeks
    spr.fillCircle(78,  138, 13, 0xF810);
    spr.fillCircle(162, 138, 13, 0xF810);

    // Flat mouth
    spr.fillRoundRect(100, 148, 40, 6, 3, TFT_BLACK);

    // ZZZ — appear one by one then reset
    uint8_t zz = (frame >> 3) % 4;
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.setTextDatum(TL_DATUM);
    if (zz >= 1) { spr.setTextFont(2); spr.setTextSize(1); spr.drawString("z", 168, 90); }
    if (zz >= 2) { spr.setTextSize(2); spr.drawString("z", 178, 70); }
    if (zz >= 3) { spr.setTextSize(3); spr.drawString("z", 190, 46); }
    spr.setTextSize(1);

    // Label
    spr.setTextDatum(MC_DATUM);
    spr.setFreeFont(&FreeMono12pt7b);
    spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
    spr.drawString("IDLE", 120, 205);
}

// ── SMOOTH: Gently rolling tyre ──────────────────────────────────
void draw_smooth() {
    spr.fillSprite(TFT_BLACK);

    float rot = frame * 6.0f * M_PI / 180.0f;
    draw_tyre(120, 112, 82, 56, 20, rot);

    spr.setTextDatum(MC_DATUM);
    spr.setFreeFont(&FreeMono12pt7b);
    spr.setTextColor(TFT_GREEN, TFT_BLACK);
    spr.drawString("SMOOTH", 120, 210);
}

// ── ACCEL: Burning tyre with flames ──────────────────────────────
void draw_accel() {
    spr.fillSprite(TFT_BLACK);

    uint8_t flick = frame % 4;
    float   rot   = frame * 18.0f * M_PI / 180.0f;  // fast spin

    // Smoke puffs
    spr.fillCircle(88  + (flick & 1) * 6, 174, 16, 0x39C7);
    spr.fillCircle(120,                   182, 12, 0x4208);
    spr.fillCircle(152 - (flick & 1) * 5, 173, 15, 0x39C7);

    // Flickering flames (6 triangles)
    const uint16_t fcols[4] = {TFT_RED, 0xFD20, TFT_YELLOW, 0xFD20};
    const int fxs[6]  = {72, 88, 104, 120, 136, 152};
    const int fhs[6]  = {50, 70,  55,  75,  50,  65};
    for (int i = 0; i < 6; i++) {
        int h = fhs[i] + ((i + flick) % 4) * 12;
        spr.fillTriangle(fxs[i], 148, fxs[i] + 15, 148, fxs[i] + 7, 148 - h,
                         fcols[(i + flick) % 4]);
    }

    // Tyre on top of flames
    draw_tyre(120, 82, 55, 36, 13, rot);

    // Labels
    spr.setTextDatum(MC_DATUM);
    spr.setFreeFont(&FreeMono12pt7b);
    spr.setTextColor(TFT_RED, TFT_BLACK);
    spr.drawString("BURN!", 120, 200);
    spr.setTextFont(2);
    spr.setTextColor(TFT_ORANGE, TFT_BLACK);
    spr.drawString("Hard Acceleration", 120, 218);
}

// ── CORNER: Flashing warning triangle ───────────────────────────
void draw_corner() {
    spr.fillSprite(TFT_BLACK);

    bool     flash_on = (frame / 6) % 2;
    uint16_t inner    = flash_on ? TFT_YELLOW : 0xFD20;
    uint16_t label    = flash_on ? TFT_RED    : TFT_ORANGE;

    // Layered triangle (red border → yellow/orange fill → black cutout)
    spr.fillTriangle(120, 25,   20, 195,  220, 195,  TFT_RED);
    spr.fillTriangle(120, 42,   36, 185,  204, 185,  inner);
    spr.fillTriangle(120, 60,   53, 175,  187, 175,  TFT_BLACK);

    // Exclamation mark
    spr.fillRoundRect(111, 80, 18, 62, 4, TFT_BLACK);  // bar
    spr.fillCircle(120, 160, 10, TFT_BLACK);            // dot

    // Labels
    spr.setTextDatum(MC_DATUM);
    spr.setFreeFont(&FreeMono12pt7b);
    spr.setTextColor(label, TFT_BLACK);
    spr.drawString("HIGH G!", 120, 213);
    spr.setTextFont(2);
    spr.setTextColor(TFT_ORANGE, TFT_BLACK);
    spr.drawString("Sharp Corner", 120, 228);
}

// ── Setup ────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    Wire.begin(PIN_I2C0_SDA, PIN_I2C0_SCL);
    touch.begin();

    if (accel.begin() == 0) {  // 0 = success for this library
        accel.Enable_X();
    }

    tft.begin();
    tft.invertDisplay(inverted);
    tft.fillScreen(TFT_BLACK);

    spr.setColorDepth(16);
    spr.createSprite(240, 240);

    tft_backlight_init();
    tft_set_backlight(16);
}

// ── Loop ─────────────────────────────────────────────────────────
void loop() {
    // Touch: toggle display invert
    if (touch.available() && (millis() - last_touch_ms > 300)) {
        last_touch_ms = millis();
        inverted = !inverted;
        tft.invertDisplay(inverted);
    }

    // Accelerometer: classify state every 100ms
    if (millis() - last_accel_ms >= 100) {
        last_accel_ms = millis();
        int32_t acc[3];
        accel.Get_X_Axes(acc);
        state = classify(acc[0], acc[2]);  // X and Z — Y has gravity
        Serial.printf("X: %6ld mg  Y: %6ld mg  Z: %6ld mg  → %s\n",
            acc[0], acc[1], acc[2],
            state == IDLE   ? "IDLE"   :
            state == SMOOTH ? "SMOOTH" :
            state == ACCEL  ? "ACCEL"  : "CORNER");
    }

    // Animation: render at ~20fps
    if (millis() - last_frame_ms >= 50) {
        last_frame_ms = millis();
        frame++;

        switch (state) {
            case IDLE:   draw_sleepy(); break;
            case SMOOTH: draw_smooth(); break;
            case ACCEL:  draw_accel();  break;
            case CORNER: draw_corner(); break;
        }

        spr.pushSprite(0, 0);
    }
}

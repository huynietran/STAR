// The OLED plus small drawing helpers shared by every screen and game.
// Owner: Turtle Brain lead
#pragma once
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

// The library allocates a 1,024-byte frame buffer at begin(); the compiler's
// "Global variables use X bytes" does NOT include it. Keep that figure under ~750.
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);

// Draw a 1-bit PROGMEM bitmap, optionally pixel-doubled (scale 2, 3...).
void drawSprite(int16_t x, int16_t y, const uint8_t* bmp, uint8_t w, uint8_t h, uint8_t scale) {
  if (scale <= 1) { display.drawBitmap(x, y, bmp, w, h, SSD1306_WHITE); return; }
  uint8_t bytesPerRow = (w + 7) / 8;
  for (uint8_t r = 0; r < h; r++)
    for (uint8_t c = 0; c < w; c++)
      if (pgm_read_byte(bmp + r * bytesPerRow + c / 8) & (0x80 >> (c % 8)))
        display.fillRect(x + c * scale, y + r * scale, scale, scale, SSD1306_WHITE);
}

// 22x5 stat bar
void drawBar(int16_t x, int16_t y, uint8_t value) {
  display.drawRect(x, y, 22, 5, SSD1306_WHITE);
  display.fillRect(x + 1, y + 1, (uint16_t)value * 20 / 100, 3, SSD1306_WHITE);
}

// Print a value stored as tenths (e.g. 314 -> "31.4")
void printTenths(int16_t v10) {
  if (v10 < 0) { display.print('-'); v10 = -v10; }
  display.print(v10 / 10);
  display.print('.');
  display.print(v10 % 10);
}

void textAt(int16_t x, int16_t y, const __FlashStringHelper* s) {
  display.setCursor(x, y);
  display.print(s);
}

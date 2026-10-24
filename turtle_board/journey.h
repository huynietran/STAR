// Journey page: a dot follows Lucky, a real Kemp's ridley tagged in August 2026 at
// the eastern tip of Massachusetts, from Cape Cod through Long Island Sound toward
// New York City (12 weekly positions, days 0-77). Data: journey_data.h, made with
// tools/journey_to_progmem.py. Map bounds: lon -74.0..-69.5, lat 40.5..42.0.
// Owner: Games, art and sound (data: Build, story and test)
#pragma once
#include "config.h"
#include "screen.h"
#include "journey_data.h"

const bool JOURNEY_IS_PLACEHOLDER = false;   // true shows "(sample)" for a made-up route
unsigned long journeyStart = 0;

void startJourney() { journeyStart = millis(); }

// Every other pixel, so the coast reads lighter than the turtle's solid trail
void dottedLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  int16_t steps = max(abs(x1 - x0), abs(y1 - y0));
  if (steps == 0) { display.drawPixel(x0, y0, SSD1306_WHITE); return; }
  for (int16_t s = 0; s <= steps; s += 2)
    display.drawPixel(x0 + (x1 - x0) * s / steps, y0 + (y1 - y0) * s / steps, SSD1306_WHITE);
}

void drawJourney() {
  // Coastline, dotted
  for (uint8_t i = 0; i + 1 < COAST_N; i++)
    dottedLine(pgm_read_byte(&COAST_XY[i][0]), pgm_read_byte(&COAST_XY[i][1]),
               pgm_read_byte(&COAST_XY[i + 1][0]), pgm_read_byte(&COAST_XY[i + 1][1]));

  // Lucky's trail so far, solid: one waypoint per second, then hold on the last one
  uint8_t idx = min((unsigned long)(TRACK_N - 1), (millis() - journeyStart) / 1000);
  for (uint8_t i = 0; i < idx; i++)
    display.drawLine(pgm_read_byte(&TRACK_XY[i][0]), pgm_read_byte(&TRACK_XY[i][1]),
                     pgm_read_byte(&TRACK_XY[i + 1][0]), pgm_read_byte(&TRACK_XY[i + 1][1]),
                     SSD1306_WHITE);
  display.fillCircle(pgm_read_byte(&TRACK_XY[idx][0]), pgm_read_byte(&TRACK_XY[idx][1]), 2, SSD1306_WHITE);

  // Labels: name and day on top, places on the map, tag date at the bottom
  textAt(0, 0, F("Lucky"));
  display.setCursor(80, 0); display.print(F("Day ")); display.print(pgm_read_word(&TRACK_DAY[idx]));
  textAt(80, 34, F("Cape Cod"));
  textAt(0, 54, F("NYC"));
  if (JOURNEY_IS_PLACEHOLDER) textAt(40, 54, F("(sample)"));
  else                        textAt(34, 54, F("Tagged Aug 2026"));
}

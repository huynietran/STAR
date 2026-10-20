// Journey page: a dot follows a tracked Kemp's ridley along the Gulf coast.
// journey_data.h currently holds a PLACEHOLDER route along the coast, not a real
// track. Replace it with real positions from seaturtle.org using
// tools/journey_to_progmem.py (see README).
// Owner: Games, art and sound (data: Build, story and test)
#pragma once
#include "config.h"
#include "screen.h"
#include "journey_data.h"

const bool JOURNEY_IS_PLACEHOLDER = true;   // set false once real data is in
unsigned long journeyStart = 0;

void startJourney() { journeyStart = millis(); }

void drawJourney() {
  // Coastline
  for (uint8_t i = 0; i + 1 < COAST_N; i++) {
    display.drawLine(pgm_read_byte(&COAST_XY[i][0]), pgm_read_byte(&COAST_XY[i][1]),
                     pgm_read_byte(&COAST_XY[i + 1][0]), pgm_read_byte(&COAST_XY[i + 1][1]),
                     SSD1306_WHITE);
  }
  // Path so far: one waypoint per second, then hold on the last one
  uint8_t idx = min((unsigned long)(TRACK_N - 1), (millis() - journeyStart) / 1000);
  for (uint8_t i = 0; i < idx; i++) {
    int16_t x0 = pgm_read_byte(&TRACK_XY[i][0]), y0 = pgm_read_byte(&TRACK_XY[i][1]);
    int16_t x1 = pgm_read_byte(&TRACK_XY[i + 1][0]), y1 = pgm_read_byte(&TRACK_XY[i + 1][1]);
    for (uint8_t s = 0; s <= 4; s++)   // dotted line
      display.drawPixel(x0 + (x1 - x0) * s / 4, y0 + (y1 - y0) * s / 4, SSD1306_WHITE);
  }
  display.fillCircle(pgm_read_byte(&TRACK_XY[idx][0]), pgm_read_byte(&TRACK_XY[idx][1]), 2, SSD1306_WHITE);

  display.setCursor(64, 40); display.print(F("Day ")); display.print(pgm_read_word(&TRACK_DAY[idx]));
  if (JOURNEY_IS_PLACEHOLDER) textAt(64, 52, F("(sample)"));
  else if (idx == TRACK_N - 1) textAt(30, 52, F("Feeding grounds!"));
}

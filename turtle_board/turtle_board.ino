// Ridley's Reef -- Turtle Board (Board 1)
// A Kemp's ridley virtual pet whose health reacts to the Reef Station's readings.
//
// Before uploading:
//   - SIM_ENV in config.h: 1 = fake sensor data, 0 = read the Reef Station link
//   - DEMO in config.h:    1 = short demo timings
// Wiring: docs/pinmap.md   Link format: docs/link-protocol.md
//
// Rule for this board: never call delay(). Everything runs off millis().

#include <Wire.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"
#include "env.h"
#include "buttons.h"
#include "pet.h"
#include "sound.h"
#include "sprites.h"
#include "screen.h"
#include "games.h"
#include "journey.h"
#include "ui.h"

unsigned long lastTick = 0, lastFrame = 0;

void failBlink() {   // OLED didn't start: out of RAM or wrong address
  pinMode(LED_BUILTIN, OUTPUT);
  for (;;) { digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); delay(150); }
}

void setup() {
  setupButtons();
  pinMode(PIN_SOUND, OUTPUT);
  link.begin(9600);
  randomSeed(analogRead(A0));   // A0 is unconnected on this board: free noise
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) failBlink();
  newEgg();
  screen = SCR_EGG;
  lastTick = millis();
}

void loop() {
#if !SIM_ENV
  pollLink();
#endif
  pollButtons();
  updateSound();

  unsigned long now = millis();
  if (now - lastTick >= TICK_MS) {
    lastTick += TICK_MS;
#if SIM_ENV
    simTick();
#endif
    petTick();
  }
  if (now - lastFrame >= FRAME_MS) {
    lastFrame = now;
    updateScreen();
    clearButtonEvents();
    drawScreen();
  }
}

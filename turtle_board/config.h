// Ridley's Reef -- Turtle Board (Board 1) settings.
// Pins must match docs/pinmap.md. Change both or neither.
#pragma once
#include <Arduino.h>

// ---------- Build switches ----------
#define DEMO     1   // 1 = short demo timings for judging, 0 = "real-time" timings
#define SIM_ENV  1   // 1 = fake sensor data (no Reef Station needed), 0 = read the link wire

// ---------- Pins ----------
const uint8_t PIN_BTN_A   = 2;   // A: next / open menu
const uint8_t PIN_BTN_B   = 3;   // B: select / pet
const uint8_t PIN_BTN_C   = 4;   // C: back
const uint8_t PIN_LINK_TX = 7;   // unused, but SoftwareSerial needs a TX pin
const uint8_t PIN_LINK_RX = 8;   // from Reef Station D1, through 1 kOhm
const uint8_t PIN_SOUND   = 9;   // -> 10k pot (volume) -> amplifier input

// ---------- Display ----------
const uint8_t SCREEN_W  = 128;
const uint8_t SCREEN_H  = 64;
const uint8_t OLED_ADDR = 0x3C;  // blank screen? run an I2C scanner and check this

// ---------- Timing ----------
const unsigned long TICK_MS         = 1000;  // one stat tick
const unsigned long FRAME_MS        = 40;    // about 25 frames per second
const unsigned long LINK_TIMEOUT_MS = 5000;  // no data this long = pause environment effects
const unsigned long COMBO_HOLD_MS   = 3000;  // hold time for two-button combos

#if DEMO
const uint16_t EGG_SECONDS       = 20;    // nest window (sex is set) = seconds 6-13
const uint16_t STAGE_SECONDS     = 90;    // seconds of good care per growth stage
const uint16_t RESCUE_SECONDS    = 60;
const uint16_t SUNSCREEN_SECONDS = 20;    // stands in for 2 hours (short to fit the 4-minute demo)
#else
const uint16_t EGG_SECONDS       = 600;
const uint16_t STAGE_SECONDS     = 1800;
const uint16_t RESCUE_SECONDS    = 300;
const uint16_t SUNSCREEN_SECONDS = 7200;  // 2 hours (dermatologists' guidance)
#endif
const uint16_t GAME_SECONDS      = 20;    // Crab Catch and Net Escape round length

// ---------- Rules (tune during testing) ----------
// Sex-from-temperature curve: FEMALE_PERMILLE table in pet.h (50% female at 30.0 C)
// No float math on this board: it costs ~2 KB of flash the games need.
const uint8_t SUN_THRESHOLD = 60;    // `sun` % that counts as "in the sun"
const uint8_t START_STAT    = 70;    // all four stats when a new egg starts
const uint8_t GROW_MIN_STAT = 60;    // growth counts only while EVERY stat is at least this
const uint8_t CARE_RESET    = 40;    // average below this resets growth progress

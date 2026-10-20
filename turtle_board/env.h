// Live readings from the Reef Station, or fake ones when SIM_ENV is 1.
// Line format and keys: docs/link-protocol.md
// Owner: Turtle Brain lead
#pragma once
#include <SoftwareSerial.h>
#include "config.h"

enum WaterVerdict : uint8_t { WQ_CLEAN = 0, WQ_CAUTION = 1, WQ_AVOID = 2, WQ_NO_SAMPLE = 3 };

struct Env {
  int16_t  tds        = 0;     // ppm (1000 = at or over the sensor's limit)
  int16_t  sal10      = -1;    // ppt x10, -1 = not sent (DIY salt probe only)
  uint8_t  cloud      = 0;     // % vs clear-water baseline
  int16_t  water10    = 250;   // C x10
  int16_t  air10      = 250;   // C x10
  uint8_t  rh         = 50;    // %
  uint8_t  sun        = 0;     // %
  int16_t  heatIndexF = 77;    // F, sun-corrected by the Reef Station
  int16_t  nest10     = 250;   // C x10, inside the egg prop
  uint8_t  wq         = WQ_NO_SAMPLE;
  unsigned long lastSeen = 0;  // millis() of the last good line
};

Env env;

bool envFresh() {
#if SIM_ENV
  return true;
#else
  return env.lastSeen != 0 && millis() - env.lastSeen < LINK_TIMEOUT_MS;
#endif
}

// ---------------- Link wire (real mode) ----------------
// Pairs are parsed as characters arrive, so no whole-line buffer is needed:
// that saves ~80 bytes of the RAM the OLED needs.
SoftwareSerial link(PIN_LINK_RX, PIN_LINK_TX);
char keyBuf[8], valBuf[10];
uint8_t keyLen = 0, valLen = 0;
bool inValue = false, lineHadData = false;

// "31.4" -> 314, "-2" -> -20. Integer-only on purpose: float math costs ~2 KB of flash.
int16_t tenths(const char* s) {
  bool neg = (*s == '-');
  if (neg) s++;
  int16_t whole = 0, frac = 0;
  while (*s >= '0' && *s <= '9') whole = whole * 10 + (*s++ - '0');
  if (*s == '.' && s[1] >= '0' && s[1] <= '9') frac = s[1] - '0';
  int16_t v = whole * 10 + frac;
  return neg ? -v : v;
}

void applyPair() {
  keyBuf[keyLen] = 0;
  valBuf[valLen] = 0;
  if (!keyLen || !valLen) return;
  const char* k = keyBuf;
  const char* v = valBuf;
  if      (!strcmp(k, "tds"))   env.tds = atoi(v);
  else if (!strcmp(k, "sal"))   env.sal10 = tenths(v);
  else if (!strcmp(k, "cloud")) env.cloud = constrain(atoi(v), 0, 100);
  else if (!strcmp(k, "water")) env.water10 = tenths(v);
  else if (!strcmp(k, "air"))   env.air10 = tenths(v);
  else if (!strcmp(k, "rh"))    env.rh = constrain(atoi(v), 0, 100);
  else if (!strcmp(k, "sun"))   env.sun = constrain(atoi(v), 0, 100);
  else if (!strcmp(k, "hi"))    env.heatIndexF = atoi(v);
  else if (!strcmp(k, "nest"))  env.nest10 = tenths(v);
  else if (!strcmp(k, "wq"))    env.wq = constrain(atoi(v), 0, 3);
  else return;                  // unknown keys are ignored on purpose
  lineHadData = true;
}

void feedLink(char c) {
  if (c == '\r') return;
  if (c == ':' && !inValue) { inValue = true; return; }
  if (c == ',' || c == '\n') {
    if (inValue) applyPair();
    keyLen = valLen = 0;
    inValue = false;
    if (c == '\n') {
      if (lineHadData) env.lastSeen = millis();
      lineHadData = false;
    }
    return;
  }
  if (!inValue) { if (keyLen < sizeof(keyBuf) - 1) keyBuf[keyLen++] = c; }
  else          { if (valLen < sizeof(valBuf) - 1) valBuf[valLen++] = c; }
}

// For testing: feed a whole line as if it arrived on the wire.
void parseLine(const char* s) {
  while (*s) feedLink(*s++);
  feedLink('\n');
}

// Call every loop. Never blocks.
void pollLink() {
  while (link.available()) feedLink(link.read());
}

// ---------------- Fake data (SIM_ENV) ----------------
// Hold B + C for 3 s to cycle scenarios.
uint8_t simScenario = 0;   // 0 calm, 1 dirty water, 2 heat + sun, 3 warm nest
const uint8_t SIM_COUNT = 4;

void simTick() {
  env.lastSeen = millis();
  env.rh = 55;
  env.water10 = 240;
  switch (simScenario) {
    case 0:  env.tds = 1000; env.cloud = 3;  env.wq = WQ_CLEAN; env.sun = 20; env.air10 = 260; env.heatIndexF = 79;  env.nest10 = 270; break;
    case 1:  env.tds = 420;  env.cloud = 70; env.wq = WQ_AVOID; env.sun = 20; env.air10 = 260; env.heatIndexF = 79;  env.nest10 = 270; break;
    case 2:  env.tds = 1000; env.cloud = 3;  env.wq = WQ_CLEAN; env.sun = 90; env.air10 = 340; env.heatIndexF = 104; env.nest10 = 300; break;
    default: env.tds = 1000; env.cloud = 3;  env.wq = WQ_CLEAN; env.sun = 40; env.air10 = 300; env.heatIndexF = 86;  env.nest10 = 315; break;
  }
  env.cloud += (millis() / 1000) % 3;  // a little wobble so the numbers look alive
}

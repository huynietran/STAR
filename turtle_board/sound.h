// Non-blocking jingles through tone() on PIN_SOUND.
// A melody is {frequency Hz, duration ms} pairs; frequency 0 = rest; {0, 0} ends it.
// Owner: Games, art and sound
#pragma once
#include "config.h"

struct Note { uint16_t freq; uint16_t ms; };

// Pitches sit around 1-2.5 kHz, where small speakers are loudest; low notes barely
// come through a tiny speaker. Each sound ends on a held note so it reads as a "ding".
const Note MEL_HATCH[]  PROGMEM = {{1047, 150}, {1319, 150}, {1568, 150}, {0, 60}, {2093, 600}, {0, 0}};
const Note MEL_EAT[]    PROGMEM = {{1568, 70}, {2093, 110}, {0, 0}};
const Note MEL_BAD[]    PROGMEM = {{880, 180}, {0, 40}, {659, 380}, {0, 0}};
const Note MEL_GROW[]   PROGMEM = {{1319, 120}, {1568, 120}, {1976, 120}, {0, 60}, {2637, 500}, {0, 0}};
const Note MEL_ALERT[]  PROGMEM = {{2000, 150}, {0, 100}, {2000, 150}, {0, 0}};
const Note MEL_RESCUE[] PROGMEM = {{1760, 250}, {1319, 250}, {1760, 250}, {1319, 250}, {0, 0}};

const Note* curMelody = nullptr;
uint8_t curNote = 0;
unsigned long noteEnds = 0;

void playMelody(const Note* m) {
  curMelody = m;
  curNote = 0;
  noteEnds = 0;
}

// Call every loop.
void updateSound() {
  if (!curMelody || millis() < noteEnds) return;
  Note n;
  memcpy_P(&n, &curMelody[curNote], sizeof(Note));
  if (n.ms == 0) { noTone(PIN_SOUND); curMelody = nullptr; return; }
  if (n.freq) tone(PIN_SOUND, n.freq); else noTone(PIN_SOUND);
  noteEnds = millis() + n.ms;
  curNote++;
}

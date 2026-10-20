// Non-blocking jingles through tone() on PIN_SOUND.
// A melody is {frequency Hz, duration ms} pairs; frequency 0 = rest; {0, 0} ends it.
// Owner: Games, art and sound
#pragma once
#include "config.h"

struct Note { uint16_t freq; uint16_t ms; };

const Note MEL_HATCH[]  PROGMEM = {{523, 100}, {659, 100}, {784, 100}, {1047, 250}, {0, 0}};
const Note MEL_EAT[]    PROGMEM = {{880, 60}, {1175, 80}, {0, 0}};
const Note MEL_BAD[]    PROGMEM = {{220, 150}, {196, 250}, {0, 0}};
const Note MEL_GROW[]   PROGMEM = {{659, 100}, {784, 100}, {988, 100}, {1319, 300}, {0, 0}};
const Note MEL_ALERT[]  PROGMEM = {{1000, 120}, {0, 80}, {1000, 120}, {0, 0}};
const Note MEL_RESCUE[] PROGMEM = {{880, 200}, {660, 200}, {880, 200}, {660, 200}, {0, 0}};

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

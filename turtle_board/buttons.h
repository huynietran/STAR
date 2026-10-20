// Three debounced buttons plus hold-for-3-seconds combos.
// Events stay latched until clearButtonEvents() runs after the screen update,
// so a press between frames is never lost.
// Owner: Turtle Brain lead
#pragma once
#include "config.h"

struct Button {
  uint8_t pin;
  bool down;                 // debounced: currently held
  bool pressed;              // latched: went down since the last frame
  bool rawLast;
  unsigned long rawChanged;
};

Button btnA = {PIN_BTN_A, false, false, false, 0};
Button btnB = {PIN_BTN_B, false, false, false, 0};
Button btnC = {PIN_BTN_C, false, false, false, 0};

enum Combo : uint8_t { COMBO_NONE, COMBO_RESET, COMBO_SKIP, COMBO_SIM };
Combo comboFired = COMBO_NONE;   // latched until consumed
uint8_t comboHeld = COMBO_NONE;
unsigned long comboStart = 0;
bool comboLatched = false;

void setupButtons() {
  pinMode(PIN_BTN_A, INPUT_PULLUP);
  pinMode(PIN_BTN_B, INPUT_PULLUP);
  pinMode(PIN_BTN_C, INPUT_PULLUP);
}

void pollButton(Button& b) {
  bool raw = digitalRead(b.pin) == LOW;   // pressed connects the pin to GND
  if (raw != b.rawLast) { b.rawLast = raw; b.rawChanged = millis(); }
  if (millis() - b.rawChanged > 25 && raw != b.down) {
    b.down = raw;
    if (raw) b.pressed = true;
  }
}

void pollButtons() {
  pollButton(btnA);
  pollButton(btnB);
  pollButton(btnC);

  uint8_t held = COMBO_NONE;
  if      (btnA.down && btnC.down && !btnB.down) held = COMBO_RESET;  // A + C: new egg
  else if (btnA.down && btnB.down && !btnC.down) held = COMBO_SKIP;   // A + B: next stage
  else if (btnB.down && btnC.down && !btnA.down) held = COMBO_SIM;    // B + C: next sim scenario
  if (held != comboHeld) { comboHeld = held; comboStart = millis(); comboLatched = false; }
  if (held != COMBO_NONE && !comboLatched && millis() - comboStart >= COMBO_HOLD_MS) {
    comboFired = (Combo)held;
    comboLatched = true;
  }
}

// A single tap: pressed while no other button is held (so combos don't also tap).
bool tapped(const Button& b) {
  return b.pressed && (btnA.down + btnB.down + btnC.down) <= 1;
}

void clearButtonEvents() {
  btnA.pressed = btnB.pressed = btnC.pressed = false;
  comboFired = COMBO_NONE;
}

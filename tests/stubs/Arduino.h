// Minimal stand-ins so the Turtle Board logic compiles on a laptop for tests.
#pragma once
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#define PROGMEM
#define pgm_read_byte(p) (*(const uint8_t*)(p))
#define pgm_read_word(p) (*(const uint16_t*)(p))
#define constrain(a,l,h) ((a)<(l)?(l):((a)>(h)?(h):(a)))
extern unsigned long fakeMillis;
inline unsigned long millis() { return fakeMillis; }
inline long random(long n) { return rand() % n; }
inline long random(long a, long b) { return a + rand() % (b - a); }

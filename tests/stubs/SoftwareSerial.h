// Minimal stand-ins so the Turtle Board logic compiles on a laptop for tests.
#pragma once
#include "Arduino.h"
struct SoftwareSerial { SoftwareSerial(int, int) {} void begin(long) {} int available() { return 0; } int read() { return -1; } };

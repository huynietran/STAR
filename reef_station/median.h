// Rolling median of the last 9 readings: ignores one-off spikes from noisy
// water sensors without lagging much (about 2 s at a 200 ms sample rate).
#pragma once
#include <Arduino.h>

const uint8_t MEDIAN_N = 9;

struct Median {
  int v[MEDIAN_N];
  uint8_t next = 0;
  uint8_t count = 0;

  void add(int x) {
    v[next] = x;
    next = (next + 1) % MEDIAN_N;
    if (count < MEDIAN_N) count++;
  }

  int get() const {
    if (count == 0) return 0;
    int s[MEDIAN_N];
    for (uint8_t k = 0; k < count; k++) s[k] = v[k];
    for (uint8_t a = 1; a < count; a++) {   // insertion sort, tiny array
      int x = s[a];
      int8_t b = a - 1;
      while (b >= 0 && s[b] > x) { s[b + 1] = s[b]; b--; }
      s[b + 1] = x;
    }
    return s[count / 2];
  }
};

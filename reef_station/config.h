// Ridley's Reef -- Reef Station (Board 2) settings and calibration.
// Pins must match docs/pinmap.md. Calibration values come from docs/calibration.md.
// Owner: Reef Station lead
#pragma once
#include <Arduino.h>

// ---------- Which water sensors are installed ----------
#define USE_DIY_CLOUD 0   // 1 = LED + photoresistor cup instead of the turbidity sensor
#define USE_DIY_SALT  0   // 1 = two-screw salt probe instead of the TDS sensor
#define NO_SALT_SENSOR 1  // 1 = no salt sensor at all: the verdict comes from cloudiness only
                          //     (clear = CLEAN, slightly cloudy = CAUTION, murky = AVOID)
#define SIM_SALT 0        // with NO_SALT_SENSOR: a pushbutton picks the salt reading for the demo.
                          //     The LCD marks it "(sim)". Each press toggles SEAWATER / FRESH.
const uint8_t PIN_SIM_SALT_BTN = 9;   // pushbutton between D9 and GND (no resistor needed)

// ---------- Pins ----------
const uint8_t LCD_RS = 12, LCD_E = 11, LCD_D4 = 5, LCD_D5 = 4, LCD_D6 = 3, LCD_D7 = 2;
const uint8_t PIN_TDS        = A0;
const uint8_t PIN_TURBIDITY  = A1;   // also the DIY cloudiness photoresistor
const uint8_t PIN_LIGHT      = A2;   // photoresistor "sun" sensor
const uint8_t PIN_NEST       = A3;   // TMP36 inside the egg prop
const uint8_t PIN_DHT        = 7;
const uint8_t PIN_ONEWIRE    = 8;    // DS18B20, 4.7 kOhm pull-up to 5 V
const uint8_t PIN_LED_GREEN  = 6;
const uint8_t PIN_LED_YELLOW = A4;
const uint8_t PIN_LED_RED    = A5;
const uint8_t PIN_DIY_CLOUD_LED = 13;   // DIY cloudiness cell's white LED
const uint8_t PIN_SALT_DRIVE_A  = 9;    // DIY salt probe, electrode 1
const uint8_t PIN_SALT_DRIVE_B  = 10;   // DIY salt probe, 1 kOhm to electrode 2
const uint8_t PIN_SALT_SENSE    = A0;   // A4 if you also keep the TDS sensor (then yellow LED -> D13)

// ---------- Calibration: fill these in during the Wednesday session ----------
const float VREF = 5.0;

// Turbidity sensor: volts in clear water, and in your murkiest milk standard
const float CLOUD_V_CLEAR = 4.1;
const float CLOUD_V_MURKY = 2.5;

// DIY cloudiness cell: (LED on - LED off) raw signal in clear water
const int DIY_CLOUD_CLEAR_SIGNAL = 400;

// DIY salt probe: raw readings in 0, 5, 15 and 35 g/L salt water
const int   SALT_CAL_RAW[4] = {120, 700, 850, 950};
const float SALT_CAL_PPT[4] = {0, 5, 15, 35};
const int   SALT_DRY_RAW    = 10;      // below this the probe is out of the water
const float SAL_SALTY_MIN   = 20.0;    // ppt that counts as salt water

// Verdict thresholds
const uint8_t CLOUD_CAUTION = 15;   // %
const uint8_t CLOUD_AVOID   = 40;   // %
const int     TDS_DRY_MAX   = 20;   // ppm: below this the probe is dry
const int     TDS_FRESH_MAX = 900;  // ppm: above this counts as salt water

// Light sensor: raw reading under room light, and with the lamp/flashlight on it
const int LIGHT_ROOM = 300;
const int LIGHT_SUN  = 900;

// TMP36 offset: compare with the DHT11 at room temperature
const float NEST_OFFSET_C = 0.0;

// Full sun can add up to 15 F to the heat index (NWS)
const float SUN_HEAT_BONUS_F = 15.0;

// ---------- Timing ----------
const unsigned long SAMPLE_MS = 200;
const unsigned long SEND_MS   = 1000;
const unsigned long DHT_MS    = 2000;
const unsigned long PAGE_MS   = 3000;

// ---------- Calibration mode ----------
// 1 = also print a "RAW ..." line every second with the numbers the calibration
// session needs (docs/calibration.md). The Turtle Board ignores that line.
// Set back to 0 after calibrating.
#define CAL_MODE 1

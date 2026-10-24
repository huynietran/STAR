// Ridley's Reef -- Reef Station (Board 2)
// Reads every sensor, decides the water and heat verdicts, shows them on the LCD
// and traffic light, and sends one line per second to the Turtle Board.
// The same line goes to USB: open Tools > Serial Plotter (9600) for a live graph.
//
// Wiring: docs/pinmap.md   Link format: docs/link-protocol.md
// Calibration: docs/calibration.md, values in config.h
// Owner: Reef Station lead

#include <LiquidCrystal.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"
#include "median.h"

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
DHT dht(PIN_DHT, DHT11);
OneWire oneWire(PIN_ONEWIRE);
DallasTemperature waterProbe(&oneWire);

enum { WQ_CLEAN = 0, WQ_CAUTION = 1, WQ_AVOID = 2, WQ_NO_SAMPLE = 3 };

Median saltRaw, cloudRaw, lightRaw, nestRaw;

// Latest computed values
int     tds = 0;          // ppm (TDS sensor)
float   salPpt = 0;       // ppt (DIY salt probe)
uint8_t cloud = 0;        // %
float   waterC = 25.0, airC = 25.0, rh = 50, heatBaseF = 77, heatF = 77, nestC = 25.0;
uint8_t sunPct = 0;
uint8_t wq = WQ_NO_SAMPLE;
#if NO_SALT_SENSOR && SIM_SALT
bool simSalty = true;            // simulated salt reading: true = seawater, false = fresh
bool simBtnLast = HIGH;
unsigned long simBtnChanged = 0;
#endif
bool    dhtOk = false;

unsigned long lastSample = 0, lastSend = 0, lastDht = 0, lastPage = 0;
uint8_t page = 0;

#if USE_DIY_CLOUD
bool diyLedOn = false;
int diyOnReading = 0;
#endif

// ---------------- Sensors ----------------
#if USE_DIY_SALT
// Pulse current one way, read, then reverse it so the screws don't polarize.
int readSaltProbe() {
  digitalWrite(PIN_SALT_DRIVE_A, HIGH); digitalWrite(PIN_SALT_DRIVE_B, LOW);
  delay(1);
  int r = analogRead(PIN_SALT_SENSE);
  digitalWrite(PIN_SALT_DRIVE_A, LOW); digitalWrite(PIN_SALT_DRIVE_B, HIGH);
  delay(1);
  digitalWrite(PIN_SALT_DRIVE_B, LOW);
  return r;
}

// Interpolate salinity from conductance (proportional to r / (1023 - r)).
float saltFromRaw(int r) {
  auto g = [](int x) { return (float)x / (1024 - x); };
  float gr = g(r);
  if (gr <= g(SALT_CAL_RAW[0])) return SALT_CAL_PPT[0];
  for (uint8_t i = 0; i < 3; i++) {
    float g0 = g(SALT_CAL_RAW[i]), g1 = g(SALT_CAL_RAW[i + 1]);
    if (gr <= g1) return SALT_CAL_PPT[i] + (gr - g0) / (g1 - g0) * (SALT_CAL_PPT[i + 1] - SALT_CAL_PPT[i]);
  }
  return SALT_CAL_PPT[3];
}
#endif

void sampleSensors() {
#if USE_DIY_SALT
  saltRaw.add(readSaltProbe());
#else
  saltRaw.add(analogRead(PIN_TDS));
#endif

#if USE_DIY_CLOUD
  // Alternate LED on/off samples; the difference cancels room light.
  if (!diyLedOn) {
    int offReading = analogRead(PIN_TURBIDITY);
    cloudRaw.add(diyOnReading - offReading);
    digitalWrite(PIN_DIY_CLOUD_LED, HIGH);
  } else {
    diyOnReading = analogRead(PIN_TURBIDITY);
    digitalWrite(PIN_DIY_CLOUD_LED, LOW);
  }
  diyLedOn = !diyLedOn;
#else
  cloudRaw.add(analogRead(PIN_TURBIDITY));
#endif

  lightRaw.add(analogRead(PIN_LIGHT));
  nestRaw.add(analogRead(PIN_NEST));
}

void readDht() {
  float h = dht.readHumidity();
  float tF = dht.readTemperature(true);
  if (isnan(h) || isnan(tF)) { dhtOk = false; return; }
  dhtOk = true;
  rh = h;
  airC = (tF - 32.0) / 1.8;
  heatBaseF = dht.computeHeatIndex(tF, h, true);
}

void computeValues() {
  // Water temperature: collect last second's conversion, start the next one
  float t = waterProbe.getTempCByIndex(0);
  if (t > -100) waterC = t;   // -127 means no probe found
  waterProbe.requestTemperatures();

  bool dry, salty;
#if NO_SALT_SENSOR
  // No salt sensor: never "dry", and treat every sample as seawater,
  // so the verdict depends only on cloudiness.
  dry = false;
#if SIM_SALT
  salty = simSalty;
#else
  salty = true;
#endif
#elif USE_DIY_SALT
  int r = saltRaw.get();
  dry = r < SALT_DRY_RAW;
  salPpt = dry ? 0 : saltFromRaw(r);
  salty = salPpt >= SAL_SALTY_MIN;
#else
  // DFRobot SEN0244 conversion with temperature compensation
  float v = saltRaw.get() * VREF / 1023.0;
  float comp = v / (1.0 + 0.02 * (waterC - 25.0));
  long ppm = (long)((133.42 * comp * comp * comp - 255.86 * comp * comp + 857.39 * comp) * 0.5);
  tds = constrain(ppm, 0, 1000);
  dry = tds < TDS_DRY_MAX;
  salty = tds > TDS_FRESH_MAX;
#endif

#if USE_DIY_CLOUD
  cloud = constrain(100L - (long)cloudRaw.get() * 100 / DIY_CLOUD_CLEAR_SIGNAL, 0L, 100L);
#else
  float cv = cloudRaw.get() * VREF / 1023.0;
  cloud = constrain((CLOUD_V_CLEAR - cv) / (CLOUD_V_CLEAR - CLOUD_V_MURKY) * 100.0, 0.0, 100.0);
#endif

  sunPct = constrain(map(lightRaw.get(), LIGHT_ROOM, LIGHT_SUN, 0, 100), 0L, 100L);
  nestC = (nestRaw.get() * VREF / 1023.0 - 0.5) * 100.0 + NEST_OFFSET_C;
  heatF = heatBaseF + SUN_HEAT_BONUS_F * sunPct / 100.0;

  if (dry)                        wq = WQ_NO_SAMPLE;
  else if (cloud >= CLOUD_AVOID)  wq = WQ_AVOID;
  else if (cloud >= CLOUD_CAUTION || !salty) wq = WQ_CAUTION;   // fresh water at a beach = runoff
  else                            wq = WQ_CLEAN;
}

// ---------------- Outputs ----------------
void setLights() {
  digitalWrite(PIN_LED_GREEN,  wq == WQ_CLEAN);
  digitalWrite(PIN_LED_YELLOW, wq == WQ_CAUTION);
  digitalWrite(PIN_LED_RED,    wq == WQ_AVOID);
}

// One line, docs/link-protocol.md. Also readable by the Arduino Serial Plotter.
void sendLine() {
#if NO_SALT_SENSOR
#if SIM_SALT
  Serial.print(F("tds:"));     Serial.print(simSalty ? 1000 : 300);   // simulated
  Serial.print(F(",cloud:"));  Serial.print(cloud);
#else
  Serial.print(F("cloud:"));   Serial.print(cloud);
#endif
#else
#if USE_DIY_SALT
  Serial.print(F("sal:"));     Serial.print(salPpt, 1);
#else
  Serial.print(F("tds:"));     Serial.print(tds);
#endif
  Serial.print(F(",cloud:"));  Serial.print(cloud);
#endif
  Serial.print(F(",water:"));  Serial.print(waterC, 1);
  Serial.print(F(",air:"));    Serial.print(airC, 1);
  Serial.print(F(",rh:"));     Serial.print((int)rh);
  Serial.print(F(",sun:"));    Serial.print(sunPct);
  Serial.print(F(",hi:"));     Serial.print((int)lround(heatF));
  Serial.print(F(",nest:"));   Serial.print(nestC, 1);
  Serial.print(F(",wq:"));     Serial.println(wq);

#if CAL_MODE
  // Raw numbers for docs/calibration.md. Uses '=' (no ':'), so the Turtle Board ignores it.
  Serial.print(F("RAW"));
#if !NO_SALT_SENSOR
  Serial.print(F(" salt="));  Serial.print(saltRaw.get());
#endif
#if USE_DIY_CLOUD
  Serial.print(F(" cloudSignal=")); Serial.print(cloudRaw.get());
#else
  Serial.print(F(" turbV="));    Serial.print(cloudRaw.get() * VREF / 1023.0, 2);
#endif
  Serial.print(F(" light="));    Serial.print(lightRaw.get());
  // At room temperature (egg not under the lamp), this is the value for NEST_OFFSET_C
  Serial.print(F(" nestOffset=")); Serial.println(airC - (nestC - NEST_OFFSET_C), 1);
#endif
}

const __FlashStringHelper* verdictWord() {
  switch (wq) {
    case WQ_CLEAN:   return F("CLEAN");
    case WQ_CAUTION: return F("CAUTION");
    case WQ_AVOID:   return F("AVOID");
    default:         return F("NO SAMPLE");
  }
}

const __FlashStringHelper* heatWord() {   // NWS heat-index bands
  if (heatF >= 125) return F("EXTREME DANGER");
  if (heatF >= 103) return F("DANGER");
  if (heatF >= 90)  return F("EXTREME CAUTION");
  if (heatF >= 80)  return F("CAUTION");
  return F("OK");
}

// 16 characters x 2 lines per page
void showPage() {
  lcd.clear();
  switch (page) {
    case 0:
      lcd.print(F("Water: ")); lcd.print(verdictWord());
      lcd.setCursor(0, 1);
      if (wq == WQ_NO_SAMPLE) lcd.print(F("Dip a probe!"));
      else { lcd.print(F("Cloudiness ")); lcd.print(cloud); lcd.print('%'); }
      break;
    case 1: {
#if NO_SALT_SENSOR
#if SIM_SALT
      lcd.print(F("Salt (sim)"));
      lcd.setCursor(0, 1);
      lcd.print(simSalty ? F("SEAWATER") : F("FRESH"));
#else
      lcd.print(F("Water ")); lcd.print(waterC, 1); lcd.print('C');
      lcd.setCursor(0, 1);
      lcd.print(F("Cloud ")); lcd.print(cloud); lcd.print('%');
#endif
      break;
#endif
#if USE_DIY_SALT
      lcd.print(F("Sal ")); lcd.print(salPpt, 1); lcd.print(F(" ppt"));
      bool salty = salPpt >= SAL_SALTY_MIN;
#else
      lcd.print(F("TDS ")); if (tds >= 1000) lcd.print('>'); lcd.print(tds); lcd.print(F(" ppm"));
      bool salty = tds > TDS_FRESH_MAX;
#endif
      lcd.setCursor(0, 1);
      if (wq == WQ_NO_SAMPLE) lcd.print(F("Salt: --"));
      else lcd.print(salty ? F("Salt: SEAWATER") : F("Salt: FRESH"));
      break;
    }
    case 2:
      lcd.print(F("Water ")); lcd.print(waterC, 1); lcd.print('C');
      lcd.setCursor(0, 1);
      if (!dhtOk) { lcd.print(F("Air sensor error")); break; }
      lcd.print(F("Air ")); lcd.print((int)lround(airC)); lcd.print(F("C RH ")); lcd.print((int)rh); lcd.print('%');
      break;
    case 3:
      lcd.print(F("Feels like ")); lcd.print((int)lround(heatF)); lcd.print('F');
      lcd.setCursor(0, 1); lcd.print(heatWord());
      break;
    default:
      lcd.print(F("Sun ")); lcd.print(sunPct); lcd.print('%');
      lcd.setCursor(0, 1); lcd.print(F("Nest ")); lcd.print(nestC, 1); lcd.print('C');
      break;
  }
}

#if NO_SALT_SENSOR && SIM_SALT
// Demo button: each press flips the simulated salt reading and shows it right away.
void pollSimSaltButton(unsigned long now) {
  bool b = digitalRead(PIN_SIM_SALT_BTN);
  if (b != simBtnLast && now - simBtnChanged > 50) {   // 50 ms debounce
    simBtnChanged = now;
    simBtnLast = b;
    if (b == LOW) {                                     // pressed
      simSalty = !simSalty;
      lcd.clear();
      lcd.print(F("Salt (sim) set:"));
      lcd.setCursor(0, 1);
      lcd.print(simSalty ? F("SEAWATER") : F("FRESH"));
      lastPage = now;                                   // hold this message one page-time
      computeValues();
      setLights();
    }
  }
}
#endif

// ---------------- Main ----------------
void setup() {
  Serial.begin(9600);   // D1 (TX) also feeds the Turtle Board
  lcd.begin(16, 2);
  lcd.print(F("Ridley's Reef"));
  lcd.setCursor(0, 1);
  lcd.print(F("Reef Station"));
  dht.begin();
  waterProbe.begin();
  waterProbe.setWaitForConversion(false);
  waterProbe.requestTemperatures();
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
#if USE_DIY_CLOUD
  pinMode(PIN_DIY_CLOUD_LED, OUTPUT);
#endif
#if USE_DIY_SALT
  pinMode(PIN_SALT_DRIVE_A, OUTPUT);
  pinMode(PIN_SALT_DRIVE_B, OUTPUT);
#endif
#if NO_SALT_SENSOR && SIM_SALT
  pinMode(PIN_SIM_SALT_BTN, INPUT_PULLUP);
#endif
  lastPage = millis();
}

void loop() {
  unsigned long now = millis();
#if NO_SALT_SENSOR && SIM_SALT
  pollSimSaltButton(now);
#endif
  if (now - lastSample >= SAMPLE_MS) { lastSample = now; sampleSensors(); }
  if (now - lastDht >= DHT_MS)       { lastDht = now; readDht(); }
  if (now - lastSend >= SEND_MS) {
    lastSend = now;
    computeValues();
    setLights();
    sendLine();
  }
  if (now - lastPage >= PAGE_MS) {
    lastPage = now;
    showPage();
    page = (page + 1) % 5;
  }
}

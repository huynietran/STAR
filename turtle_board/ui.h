// Screens, menu and alerts. updateScreen() handles input and transitions;
// drawScreen() redraws the whole frame.
// Controls: Home: A = menu, B = pet. Menu: A = next, B = select, C = back.
// Hold 3 s: A+C = new egg, A+B = next stage, B+C = next sim scenario.
// Owner: Turtle Brain lead
#pragma once
#include "config.h"
#include "env.h"
#include "buttons.h"
#include "pet.h"
#include "sound.h"
#include "sprites.h"
#include "screen.h"
#include "games.h"
#include "journey.h"

enum Screen : uint8_t {
  SCR_EGG, SCR_HOME, SCR_MENU, SCR_STATS, SCR_WATER, SCR_JOURNEY,
  SCR_GAME_CRAB, SCR_GAME_NET, SCR_ALERT, SCR_MESSAGE, SCR_RESCUE
};
Screen screen = SCR_EGG;

enum MenuItem : uint8_t { M_FEED, M_PLAY, M_PET, M_STATS, M_WATER, M_SUNSCREEN, M_JOURNEY, M_COUNT };
uint8_t menuIndex = 0;

Alert shownAlert = ALERT_NONE;
unsigned long screenUntil = 0, lastPet = 0;
const __FlashStringHelper* msg1 = nullptr;
const __FlashStringHelper* msg2 = nullptr;

void goHome() {
  if (pet.stage == STAGE_EGG) screen = SCR_EGG;
  else if (pet.inRescue)      screen = SCR_RESCUE;
  else                        screen = SCR_HOME;
}

void showMessage(const __FlashStringHelper* a, const __FlashStringHelper* b) {
  msg1 = a; msg2 = b;
  screen = SCR_MESSAGE;
  screenUntil = millis() + 2500;
}

void petTurtle() {
  if (millis() - lastPet < 10000UL && lastPet != 0) { showMessage(F("Zzz..."), F("Pet again soon")); return; }
  lastPet = millis();
  addTo(pet.happy, 2);
  showHeart();
  playMelody(MEL_EAT);
  showMessage(F("Happy turtle!"), F("Happiness +2"));
}

void printMenuName(uint8_t i) {
  switch (i) {
    case M_FEED:      display.print(F("Feed: Crab Catch")); break;
    case M_PLAY:      display.print(F("Play: Net Escape")); break;
    case M_PET:       display.print(F("Pet")); break;
    case M_STATS:     display.print(F("Stats")); break;
    case M_WATER:     display.print(F("Water check")); break;
    case M_SUNSCREEN: display.print(F("Sunscreen")); break;
    default:          display.print(F("Journey")); break;
  }
}

void selectMenu() {
  switch (menuIndex) {
    case M_FEED:      startCrab(); screen = SCR_GAME_CRAB; break;
    case M_PLAY:      startNet(); screen = SCR_GAME_NET; break;
    case M_PET:       petTurtle(); break;
    case M_STATS:     screen = SCR_STATS; break;
    case M_WATER:     screen = SCR_WATER; break;
    case M_SUNSCREEN: applySunscreen(); playMelody(MEL_EAT); showMessage(F("Sunscreen on!"), F("Reapply in 2 hours")); break;
    default:          startJourney(); screen = SCR_JOURNEY; break;
  }
}

void playAlertSound(Alert a) {
  switch (a) {
    case ALERT_HATCHED:  playMelody(MEL_HATCH); break;
    case ALERT_GREW:
    case ALERT_RELEASED: playMelody(MEL_GROW); break;
    case ALERT_RESCUED:  playMelody(MEL_RESCUE); break;
    case ALERT_DIRTY:    playMelody(MEL_BAD); break;
    default:             playMelody(MEL_ALERT); break;
  }
}

void showSimName() {
#if SIM_ENV
  switch (simScenario) {
    case 0:  showMessage(F("Sim scenario:"), F("calm")); break;
    case 1:  showMessage(F("Sim scenario:"), F("dirty water")); break;
    case 2:  showMessage(F("Sim scenario:"), F("heat + sun")); break;
    default: showMessage(F("Sim scenario:"), F("warm nest")); break;
  }
#endif
}

// ---------------- Input and transitions ----------------
void updateScreen() {
  unsigned long now = millis();

  // Combos work on every screen
  if (comboFired == COMBO_RESET) { newEgg(); screen = SCR_EGG; playMelody(MEL_ALERT); return; }
  if (comboFired == COMBO_SKIP)  { growUp(); return; }
#if SIM_ENV
  if (comboFired == COMBO_SIM)   { simScenario = (simScenario + 1) % SIM_COUNT; showSimName(); return; }
#endif

  // Alerts interrupt everything except a game in progress
  if (pendingAlert != ALERT_NONE && screen != SCR_GAME_CRAB && screen != SCR_GAME_NET && screen != SCR_ALERT) {
    shownAlert = pendingAlert;
    pendingAlert = ALERT_NONE;
    screen = SCR_ALERT;
    screenUntil = now + 3500;
    playAlertSound(shownAlert);
    return;
  }

  switch (screen) {
    case SCR_EGG:
      break;   // petTick() hatches it
    case SCR_HOME:
      if (tapped(btnA))      screen = SCR_MENU;
      else if (tapped(btnB)) petTurtle();
      break;
    case SCR_MENU:
      if (tapped(btnA))      menuIndex = (menuIndex + 1) % M_COUNT;
      else if (tapped(btnB)) selectMenu();
      else if (tapped(btnC)) screen = SCR_HOME;
      break;
    case SCR_STATS:
    case SCR_WATER:
    case SCR_JOURNEY:
      if (tapped(btnB) || tapped(btnC)) screen = SCR_MENU;
      break;
    case SCR_GAME_CRAB:
      if (updateCrab()) goHome();
      break;
    case SCR_GAME_NET:
      if (updateNet()) goHome();
      break;
    case SCR_ALERT:
    case SCR_MESSAGE:
      if ((long)(now - screenUntil) >= 0 || tapped(btnB)) goHome();
      break;
    case SCR_RESCUE:
      if (!pet.inRescue) goHome();
      break;
  }
}

// ---------------- Drawing ----------------
// Four 8x8 stat icons, each followed by its bar: health, happiness, food, comfort.
void drawStatBars() {
  drawSprite(0,  4, SPR_ICON_HEALTH,  8, 8, 1); drawBar(10,  5, pet.health);
  drawSprite(32, 4, SPR_ICON_HAPPY,   8, 8, 1); drawBar(42,  5, pet.happy);
  drawSprite(64, 4, SPR_ICON_FOOD,    8, 8, 1); drawBar(74,  5, pet.hunger);
  drawSprite(96, 4, SPR_ICON_COMFORT, 8, 8, 1); drawBar(106, 5, pet.comfort);
}

// The mood bubble beside the turtle. Most urgent first; nullptr = no bubble.
const uint8_t* currentEmote() {
  bool fresh = envFresh();
  if (fresh && env.wq == WQ_AVOID)    return SPR_EMO_SICK;
  if (fresh && env.heatIndexF >= 90)  return SPR_EMO_HOT;
  if (pet.sunscreenDue)               return SPR_EMO_SUN;
  if (pet.hunger < 30)                return SPR_EMO_HUNGRY;
  if (millis() < heartUntil)          return SPR_EMO_HEART;
  return nullptr;
}

// The turtle for its life stage, centred on (cx, cy), with a 2-frame swim and a 1-pixel bob.
void drawTurtle(int16_t cx, int16_t cy) {
  bool f = (millis() / 400) % 2;              // swap swim frames every 0.4 s
  int16_t bob = (millis() / 800) % 2;
  const uint8_t* spr;
  uint8_t sz;
  switch (pet.stage) {
    case STAGE_HATCHLING: spr = f ? SPR_HATCH_1 : SPR_HATCH_0; sz = 16; break;
    case STAGE_JUVENILE:  spr = f ? SPR_JUV_1   : SPR_JUV_0;   sz = 24; break;
    default:
      if (pet.female) spr = f ? SPR_ADULT_F_1 : SPR_ADULT_F_0;
      else            spr = f ? SPR_ADULT_M_1 : SPR_ADULT_M_0;
      sz = 32;
  }
  drawSprite(cx - sz / 2, cy - sz / 2 + bob, spr, sz, sz, 1);

  const uint8_t* emote = currentEmote();
  if (emote && (millis() / 600) % 4 != 0)     // blink off briefly so it catches the eye
    drawSprite(cx + sz / 2 + 2, cy - sz / 2, emote, 8, 8, 1);
}

void drawStatusCorner() {
#if SIM_ENV
  textAt(0, 56, F("SIM"));
#else
  if (!envFresh()) textAt(0, 56, F("NO SIG"));
#endif
  if (pet.sunscreenDue && (millis() / 500) % 2) textAt(110, 56, F("SPF"));
}

void drawHome() {
  drawStatBars();
  for (int16_t x = 0; x < SCREEN_W; x += 8)   // waterline
    display.drawPixel(x + (millis() / 300) % 8, 54, SSD1306_WHITE);
  drawTurtle(64, 34);
  drawStatusCorner();
}

void drawEgg() {
  uint16_t period = map(pet.eggSeconds, 0, EGG_SECONDS, 600, 120);   // wobbles faster near hatching
  int16_t wobble = (millis() / period) % 2 ? 1 : -1;
  uint16_t left = EGG_SECONDS - pet.eggSeconds;
  // Cracks appear in the last quarter, and spread in the last twelfth, of incubation
  const uint8_t* spr = left <= EGG_SECONDS / 12 + 1 ? SPR_EGG_CRACK_1 : (left <= EGG_SECONDS / 4 ? SPR_EGG_CRACK_0 : SPR_EGG);
  drawSprite(48 + wobble, 18, spr, 16, 16, 2);   // drawn 2x: 32x32 on screen, below the header
  // Tell the player when warming the nest sensor counts (pinch it to warm it)
  if (inSexWindow())                          textAt(10, 4, F("Warm=girl Cool=boy"));
  else if (pet.eggSeconds < EGG_SECONDS / 3)  textAt(28, 4, F("Incubating..."));
  else                                        textAt(16, 4, F("Almost hatching!"));
  display.setCursor(40, 56); display.print(F("Nest ")); printTenths(env.nest10); display.print(F(" C"));
  drawStatusCorner();
}

void drawMenu() {
  drawStatBars();
  drawTurtle(64, 33);
  display.fillRect(0, 52, SCREEN_W, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(4, 54);
  printMenuName(menuIndex);
  display.setTextColor(SSD1306_WHITE);
}

const __FlashStringHelper* stageName() {
  switch (pet.stage) {
    case STAGE_EGG:       return F("egg");
    case STAGE_HATCHLING: return F("hatchling");
    case STAGE_JUVENILE:  return F("juvenile");
    default:              return F("adult");
  }
}

void drawStats() {
  display.setCursor(0, 4);  display.print(stageName()); display.print(pet.female ? F(", female") : F(", male"));
  display.setCursor(0, 20); display.print(F("Health  ")); display.print(pet.health);
  display.setCursor(0, 29); display.print(F("Happy   ")); display.print(pet.happy);
  display.setCursor(0, 38); display.print(F("Fed     ")); display.print(pet.hunger);
  display.setCursor(0, 47); display.print(F("Comfort ")); display.print(pet.comfort);
  display.setCursor(0, 56); display.print(F("Care ")); display.print(careScore());
  display.print(F("  grow ")); display.print(pet.stageSeconds); display.print('/'); display.print(STAGE_SECONDS);
}

void drawWater() {
  if (!envFresh()) { textAt(0, 24, F("No signal from the")); textAt(0, 34, F("Reef Station")); return; }
  display.setTextSize(2);
  display.setCursor(0, 0);
  switch (env.wq) {
    case WQ_CLEAN:   display.print(F("CLEAN")); break;
    case WQ_CAUTION: display.print(F("CAUTION")); break;
    case WQ_AVOID:   display.print(F("AVOID")); break;
    default:         display.print(F("NO SAMPLE")); break;
  }
  display.setTextSize(1);
  display.setCursor(0, 22); display.print(F("Cloudiness ")); display.print(env.cloud); display.print('%');
  display.setCursor(0, 32);
  if (env.sal10 >= 0) { display.print(F("Salinity ")); printTenths(env.sal10); display.print(F(" ppt")); }
  else if (env.tds > 0) { display.print(F("TDS ")); display.print(env.tds); display.print(F(" ppm")); }
  // no salt sensor on the Reef Station: no salt line
  switch (env.wq) {
    case WQ_CLEAN:   textAt(0, 48, F("Clean water. Yay!")); break;
    case WQ_CAUTION: textAt(0, 48, F("A bit cloudy...")); textAt(0, 56, F("runoff? Be careful.")); break;
    case WQ_AVOID:   textAt(0, 48, F("Murky! I feel sick.")); break;
    default:         textAt(0, 48, F("Dip a probe!")); break;
  }
}

void drawAlert() {
  display.setTextSize(1);
  switch (shownAlert) {
    case ALERT_HATCHED:
      textAt(0, 4, F("It hatched!"));
      textAt(0, 20, pet.female ? F("It's a girl!") : F("It's a boy!"));
      display.setCursor(0, 32); display.print(F("Nest avg ")); printTenths(pet.nestAvg10); display.print(F(" C"));
      textAt(0, 45, F("Warm nest = females"));
      textAt(0, 55, F("Cool nest = males"));
      break;
    case ALERT_GREW:       textAt(0, 4, F("Growing up!")); display.setCursor(0, 24); display.print(F("Now a ")); display.print(stageName()); break;
    case ALERT_HOT:        textAt(0, 4, F("Hot out there!")); textAt(0, 24, F("Find shade and")); textAt(0, 34, F("drink water.")); break;
    case ALERT_DANGER_HOT: textAt(0, 4, F("DANGER: heat!")); textAt(0, 24, F("Get out of the sun")); textAt(0, 34, F("right now.")); break;
    case ALERT_SUNSCREEN:  textAt(0, 4, F("Reapply sunscreen!")); textAt(0, 24, F("Menu > Sunscreen")); break;
    case ALERT_DIRTY:      textAt(0, 4, F("Murky water!")); textAt(0, 24, F("I feel sick...")); break;
    case ALERT_RESCUED:    textAt(0, 4, F("Rescued!")); textAt(0, 24, F("Off to turtle rehab")); break;
    case ALERT_RELEASED:   textAt(0, 4, F("Released!")); textAt(0, 24, F("Back in the Gulf")); break;
    default: break;
  }
}

void drawRescue() {
  textAt(7, 4, F("Turtle rehab center"));
  display.fillRect(57, 20, 14, 26, SSD1306_WHITE);   // rescue cross, below the header
  display.fillRect(49, 26, 30, 14, SSD1306_WHITE);
  display.setCursor(10, 54); display.print(F("Recovering: ")); display.print(pet.rescueSeconds); display.print('s');
}

void drawScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  switch (screen) {
    case SCR_EGG:       drawEgg(); break;
    case SCR_HOME:      drawHome(); break;
    case SCR_MENU:      drawMenu(); break;
    case SCR_STATS:     drawStats(); break;
    case SCR_WATER:     drawWater(); break;
    case SCR_JOURNEY:   drawJourney(); break;
    case SCR_GAME_CRAB: drawCrab(); break;
    case SCR_GAME_NET:  drawNet(); break;
    case SCR_ALERT:     drawAlert(); break;
    case SCR_MESSAGE:   if (msg1) textAt(0, 4, msg1); if (msg2) textAt(0, 24, msg2); break;
    case SCR_RESCUE:    drawRescue(); break;
  }
  display.display();
}

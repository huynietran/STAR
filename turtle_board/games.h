// Mini-games. Each game is three functions: startX(), updateX() -> true when
// finished, and drawX(). Only one runs at a time, so they share one union.
// Crab Catch and Net Escape are complete; tune them with the constants at the top of each.
// Owner: Games, art and sound
#pragma once
#include "config.h"
#include "buttons.h"
#include "pet.h"
#include "sound.h"
#include "sprites.h"
#include "screen.h"

// ---------------- Crab Catch (Feed -> hunger) ----------------
// Kemp's ridleys mainly eat swimming crabs; floating plastic gets swallowed as food.
struct Faller { int16_t x, y; uint8_t kind; bool alive; };   // kind 0 crab, 1 plastic bag

struct CrabGame {
  Faller items[5];
  int16_t px;                 // turtle x
  uint8_t crabs, bags;
  unsigned long startMs, lastSpawn, overMs;
  bool over;
};

// ---------------- Net Escape (Play -> happiness) ----------------
// Hold B to swim up, release to sink. Swim through the gaps in the fishing nets.
// Teaches: getting caught in fishing gear (e.g. shrimp trawls) is the species' main threat.
const uint8_t NET_GAP     = 26;   // height of the opening, pixels (bigger = easier)
const uint8_t NET_SPACING = 56;   // distance between nets, pixels
const uint8_t NET_SPEED   = 2;    // pixels per frame (bigger = harder)
const int16_t SWIM_X      = 16;   // turtle's fixed x position

struct Net { int16_t x; uint8_t gapY; bool passed; };

struct NetGame {
  Net nets[3];
  int16_t y;                      // turtle's top edge
  uint8_t score;
  unsigned long startMs, overMs;
  bool over, caught;
};
union GameState {
  CrabGame crab;
  NetGame net;
};
GameState game;

const int16_t PLAYER_Y = 48;

void startCrab() {
  memset(&game, 0, sizeof(game));
  game.crab.px = (SCREEN_W - 16) / 2;
  game.crab.startMs = game.crab.lastSpawn = millis();
}

bool updateCrab() {
  CrabGame& g = game.crab;
  unsigned long now = millis();
  if (g.over) return now - g.overMs > 2500;   // result screen, then done

  if (btnA.down) g.px -= 3;
  if (btnC.down) g.px += 3;
  g.px = constrain(g.px, 0, SCREEN_W - 16);

  unsigned long elapsed = now - g.startMs;
  bool fast = elapsed > 15000;
  if (now - g.lastSpawn >= (fast ? 450UL : 700UL)) {
    g.lastSpawn = now;
    for (Faller& f : g.items) {
      if (f.alive) continue;
      f.alive = true;
      f.x = random(0, SCREEN_W - 8);
      f.y = BODY_Y;                         // start below the yellow header
      f.kind = random(100) < 25 ? 1 : 0;   // 25% plastic
      break;
    }
  }

  for (Faller& f : g.items) {
    if (!f.alive) continue;
    f.y += fast ? 3 : 2;
    bool hit = f.y + 8 >= PLAYER_Y + 2 && f.y <= PLAYER_Y + 12 &&
               f.x + 8 > g.px + 2 && f.x < g.px + 14;
    if (hit) {
      f.alive = false;
      if (f.kind == 0) { g.crabs++; playMelody(MEL_EAT); }
      else             { g.bags++;  playMelody(MEL_BAD); }
    } else if (f.y > SCREEN_H) {
      f.alive = false;
    }
  }

  if (elapsed >= GAME_SECONDS * 1000UL) {
    g.over = true;
    g.overMs = now;
    addTo(pet.hunger, 5 * g.crabs);
    addTo(pet.happy, 10);                  // playing any game cheers it up
    if (g.crabs) showHeart();
    addTo(pet.health, -10 * (int)g.bags);
  }
  return false;
}

void drawCrab() {
  CrabGame& g = game.crab;
  if (g.over) {
    display.setCursor(10, 4);  display.print(F("Crabs eaten: ")); display.print(g.crabs);
    display.setCursor(10, 24); display.print(F("Plastic:     ")); display.print(g.bags);
    if (g.bags) textAt(10, 42, F("Plastic isn't food!"));
    else        textAt(10, 42, F("Great hunting!"));
    return;
  }
  display.setCursor(0, 4); display.print(F("Crabs ")); display.print(g.crabs);
  unsigned long left = (GAME_SECONDS * 1000UL - (millis() - g.startMs)) / 1000;
  display.setCursor(104, 4); display.print(left); display.print('s');
  bool f2 = (millis() / 300) % 2;   // swap animation frames every 0.3 s
  for (Faller& f : g.items) {
    if (!f.alive) continue;
    const uint8_t* spr = f.kind ? (f2 ? SPR_BAG_1 : SPR_BAG_0) : (f2 ? SPR_CRAB_1 : SPR_CRAB_0);
    drawSprite(f.x, f.y, spr, 8, 8, 1);
  }
  drawSprite(g.px, PLAYER_Y, f2 ? SPR_HATCH_1 : SPR_HATCH_0, 16, 16, 1);
}

// ---------------- Net Escape functions ----------------
void startNet() {
  memset(&game, 0, sizeof(game));
  NetGame& g = game.net;
  g.y = 34;
  for (uint8_t i = 0; i < 3; i++) {
    g.nets[i].x = SCREEN_W + i * NET_SPACING;
    g.nets[i].gapY = random(BODY_Y + 2, SCREEN_H - NET_GAP);
  }
  g.startMs = millis();
}

void endNet(bool caught) {
  NetGame& g = game.net;
  g.over = true;
  g.caught = caught;
  g.overMs = millis();
  addTo(pet.happy, 5 + 3 * g.score);
  if (g.score) showHeart();
  playMelody(caught ? MEL_BAD : MEL_GROW);
}

bool updateNet() {
  NetGame& g = game.net;
  unsigned long now = millis();
  if (g.over) return now - g.overMs > 2500;   // result screen, then done

  g.y += btnB.down ? -2 : 1;                  // swim up while B is held, else sink
  g.y = constrain(g.y, BODY_Y, SCREEN_H - 8);

  for (Net& n : g.nets) {
    n.x -= NET_SPEED;
    if (n.x < -4) {                           // off screen: recycle it on the right
      n.x += 3 * NET_SPACING;
      n.gapY = random(BODY_Y + 2, SCREEN_H - NET_GAP);
      n.passed = false;
    }
    bool sameColumn = n.x < SWIM_X + 16 && n.x + 4 > SWIM_X;
    bool inGap = g.y >= n.gapY && g.y + 8 <= n.gapY + NET_GAP;
    if (sameColumn && !inGap) { endNet(true); return false; }
    if (!n.passed && n.x + 4 < SWIM_X) { n.passed = true; g.score++; playMelody(MEL_EAT); }
  }
  if (now - g.startMs >= GAME_SECONDS * 1000UL) endNet(false);
  return false;
}

void drawNetColumn(int16_t x, int16_t y0, int16_t y1) {   // a strip of mesh from y0 to y1
  if (y1 <= y0) return;
  display.drawFastVLine(x, y0, y1 - y0, SSD1306_WHITE);
  display.drawFastVLine(x + 4, y0, y1 - y0, SSD1306_WHITE);
  for (int16_t y = y0; y < y1; y += 3) display.drawFastHLine(x, y, 5, SSD1306_WHITE);   // mesh rungs
}

void drawNet() {
  NetGame& g = game.net;
  if (g.over) {
    textAt(4, 4, g.caught ? F("Caught in a net!") : F("You made it!"));
    display.setCursor(4, 20); display.print(F("Nets passed: ")); display.print(g.score);
    textAt(4, 34, F("Nets trap turtles."));
    textAt(4, 44, F("Escape hatches"));
    textAt(4, 54, F("(TEDs) save them."));
    return;
  }
  display.setCursor(0, 4); display.print(F("Nets ")); display.print(g.score);
  unsigned long left = (GAME_SECONDS * 1000UL - (millis() - g.startMs)) / 1000;
  display.setCursor(104, 4); display.print(left); display.print('s');
  display.drawFastHLine(0, HEADER_H, SCREEN_W, SSD1306_WHITE);   // water surface, just below the header
  for (Net& n : g.nets) {
    if (n.x > SCREEN_W) continue;
    drawNetColumn(n.x, BODY_Y, n.gapY);
    drawNetColumn(n.x, n.gapY + NET_GAP, SCREEN_H);
  }
  bool f = (millis() / 300) % 2;
  drawSprite(SWIM_X, g.y, f ? SPR_SWIM_1 : SPR_SWIM_0, 16, 8, 1);
}

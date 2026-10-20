// Mini-games. Each game is three functions: startX(), updateX() -> true when
// finished, and drawX(). Only one runs at a time, so they share one union.
// Crab Catch is complete as a template; Net Escape is yours to build.
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
// TODO: side-scroller. Hold B to swim up, release to sink; nets scroll in with gaps.
// Teaches: fishing gear (e.g. shrimp trawls) is the species' main threat.
struct NetGame {
  int16_t y;
  uint8_t score;
  bool over;
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
      f.y = 8;
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
    display.setCursor(10, 12); display.print(F("Crabs eaten: ")); display.print(g.crabs);
    display.setCursor(10, 24); display.print(F("Plastic:     ")); display.print(g.bags);
    if (g.bags) textAt(10, 42, F("Plastic isn't food!"));
    else        textAt(10, 42, F("Great hunting!"));
    return;
  }
  display.setCursor(0, 0); display.print(F("Crabs ")); display.print(g.crabs);
  unsigned long left = (GAME_SECONDS * 1000UL - (millis() - g.startMs)) / 1000;
  display.setCursor(104, 0); display.print(left); display.print('s');
  bool f2 = (millis() / 300) % 2;   // swap animation frames every 0.3 s
  for (Faller& f : g.items) {
    if (!f.alive) continue;
    const uint8_t* spr = f.kind ? (f2 ? SPR_BAG_1 : SPR_BAG_0) : (f2 ? SPR_CRAB_1 : SPR_CRAB_0);
    drawSprite(f.x, f.y, spr, 8, 8, 1);
  }
  drawSprite(g.px, PLAYER_Y, f2 ? SPR_HATCH_1 : SPR_HATCH_0, 16, 16, 1);
}

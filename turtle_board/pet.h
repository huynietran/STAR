// The turtle: four stats, life stages, temperature-based sex, rescue.
// Rates are demo-speed starting points; tune so an ignored turtle visibly
// declines within about two minutes.
// Owner: Turtle Brain lead
#pragma once
#include "config.h"
#include "env.h"

enum Stage : uint8_t { STAGE_EGG, STAGE_HATCHLING, STAGE_JUVENILE, STAGE_ADULT };

enum Alert : uint8_t {
  ALERT_NONE, ALERT_HATCHED, ALERT_GREW, ALERT_HOT, ALERT_DANGER_HOT,
  ALERT_SUNSCREEN, ALERT_DIRTY, ALERT_RESCUED, ALERT_RELEASED
};

struct Pet {
  uint8_t  health, happy, hunger, comfort;  // 0-100; hunger = fullness
  Stage    stage;
  bool     female;
  bool     inRescue;
  bool     sunscreenDue;
  uint16_t eggSeconds;
  uint16_t stageSeconds;   // seconds of good care in this stage
  uint16_t rescueSeconds;
  uint16_t sunSeconds;     // seconds in the sun since the last sunscreen
  int32_t  nestSum10;      // running sum for the average nest temperature
  uint16_t nestCount;
  int16_t  nestAvg10;      // C x10, set at hatching
};

Pet pet;
Alert pendingAlert = ALERT_NONE;
uint32_t ticks = 0;
uint8_t heatCooldown = 0, dirtyCooldown = 0;   // seconds until the alert may repeat

void queueAlert(Alert a) {
  if (pendingAlert == ALERT_NONE || a == ALERT_RESCUED || a == ALERT_HATCHED) pendingAlert = a;
}

void addTo(uint8_t& v, int d) {
  int n = (int)v + d;
  v = n < 0 ? 0 : (n > 100 ? 100 : n);
}

uint8_t careScore() {
  return ((uint16_t)pet.health + pet.happy + pet.hunger + pet.comfort) / 4;
}

void newEgg() {
  pet = Pet();   // all zero
  pet.health = pet.happy = pet.hunger = pet.comfort = START_STAT;
  pet.stage = STAGE_EGG;
  pendingAlert = ALERT_NONE;
}

// Kemp's ridley: ~50% female at 30.0 C, mixed sexes up to ~32.5 C, warmer = more females.
// Chance of female (per mille) every 0.5 C from 28.0 to 32.5 C: the logistic curve
// 1 / (1 + e^-((t - 30.0) / 0.6)), precomputed so the board needs no float math.
const uint16_t FEMALE_PERMILLE[10] PROGMEM = {34, 76, 159, 303, 500, 697, 841, 924, 966, 985};

uint16_t femaleChance(int16_t t10) {
  if (t10 <= 280) return 34;
  if (t10 >= 325) return 985;
  uint8_t i = (t10 - 280) / 5;          // table step below t
  int16_t into = (t10 - 280) % 5;       // tenths past that step (0-4)
  uint16_t a = pgm_read_word(&FEMALE_PERMILLE[i]), b = pgm_read_word(&FEMALE_PERMILLE[i + 1]);
  return a + (int32_t)(b - a) * into / 5;
}

void hatch() {
  pet.nestAvg10 = pet.nestCount ? pet.nestSum10 / pet.nestCount : env.nest10;
  pet.female = random(1000) < femaleChance(pet.nestAvg10);
  pet.stage = STAGE_HATCHLING;
  pet.stageSeconds = 0;
  queueAlert(ALERT_HATCHED);
}

void growUp() {
  if (pet.stage == STAGE_EGG) { hatch(); return; }
  if (pet.stage >= STAGE_ADULT) return;
  pet.stage = (Stage)(pet.stage + 1);
  pet.stageSeconds = 0;
  queueAlert(ALERT_GREW);
}

// NWS heat-index bands: Caution 80-90 F, Extreme Caution 90-103 F, Danger 103+ F
int heatPenalty() {
  if (env.heatIndexF >= 103) return 60;
  if (env.heatIndexF >= 90)  return 35;
  if (env.heatIndexF >= 80)  return 15;
  return 0;
}

void applySunscreen() {
  pet.sunSeconds = 0;
  pet.sunscreenDue = false;
}

// Called once per second.
void petTick() {
  ticks++;
  if (heatCooldown)  heatCooldown--;
  if (dirtyCooldown) dirtyCooldown--;
  bool fresh = envFresh();

  if (pet.stage == STAGE_EGG) {
    if (fresh) { pet.nestSum10 += env.nest10; pet.nestCount++; }
    if (++pet.eggSeconds >= EGG_SECONDS) hatch();
    return;
  }

  if (pet.inRescue) {
    if (pet.rescueSeconds) pet.rescueSeconds--;
    if (pet.rescueSeconds == 0) {
      pet.inRescue = false;
      pet.health = 50;
      pet.happy = pet.hunger = pet.comfort = 60;
      pet.stageSeconds = 0;
      queueAlert(ALERT_RELEASED);
    }
    return;
  }

  // Slow decay
  if (ticks % 6 == 0) addTo(pet.hunger, -1);
  if (ticks % 8 == 0) addTo(pet.happy, -1);

  // Environment effects (paused while the link is quiet)
  if (fresh) {
    if (env.sun >= SUN_THRESHOLD && !pet.sunscreenDue && ++pet.sunSeconds >= SUNSCREEN_SECONDS) {
      pet.sunscreenDue = true;
      queueAlert(ALERT_SUNSCREEN);
    }
    int target = 100 - heatPenalty() - (pet.sunscreenDue ? 25 : 0);
    addTo(pet.comfort, constrain(target - (int)pet.comfort, -2, 2));

    if (env.heatIndexF >= 90 && heatCooldown == 0) {
      queueAlert(env.heatIndexF >= 103 ? ALERT_DANGER_HOT : ALERT_HOT);
      heatCooldown = 30;
    }

    if (env.wq == WQ_AVOID) {
      addTo(pet.health, -1);
      if (dirtyCooldown == 0) { queueAlert(ALERT_DIRTY); dirtyCooldown = 20; }
    } else if (env.wq == WQ_CAUTION && ticks % 4 == 0) {
      addTo(pet.health, -1);
    }
  }

  // Neglect hurts
  if ((pet.hunger < 20 || pet.happy < 20 || pet.comfort < 20) && ticks % 5 == 0) addTo(pet.health, -1);

  // Recovery when everything is good
  bool waterOk = !fresh || env.wq == WQ_CLEAN || env.wq == WQ_NO_SAMPLE;
  if (pet.hunger >= 60 && pet.happy >= 60 && pet.comfort >= 60 && waterOk && ticks % 5 == 0) {
    addTo(pet.health, +1);
  }

  // Growth: needs every stat kept up, so an ignored turtle never grows
  bool allGood = pet.health >= GROW_MIN_STAT && pet.happy >= GROW_MIN_STAT &&
                 pet.hunger >= GROW_MIN_STAT && pet.comfort >= GROW_MIN_STAT;
  if (allGood) pet.stageSeconds++;
  else if (careScore() < CARE_RESET) pet.stageSeconds = 0;
  if (pet.stage < STAGE_ADULT && pet.stageSeconds >= STAGE_SECONDS) growUp();

  // Rescue instead of death
  if (pet.health == 0) {
    pet.inRescue = true;
    pet.rescueSeconds = RESCUE_SECONDS;
    queueAlert(ALERT_RESCUED);
  }
}

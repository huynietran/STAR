// Host-side tests: link parsing, sex-from-temperature odds, growth and rescue timing.
// Run: sh tests/run.sh   (uses stubs/ in place of the Arduino core)
#include <stdio.h>
unsigned long fakeMillis = 1;
#include "../turtle_board/config.h"
#undef SIM_ENV
#define SIM_ENV 0
#include "../turtle_board/env.h"
#include "../turtle_board/pet.h"
int fails = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)
void tick(int n) { for (int i = 0; i < n; i++) { fakeMillis += 1000; env.lastSeen = fakeMillis; petTick(); } }
int main() {
  CHECK(tenths("31.4") == 314); CHECK(tenths("-2") == -20); CHECK(tenths("22") == 220); CHECK(tenths("0.5") == 5);
  char line[] = "tds:245,cloud:12,water:22.4,air:29.1,rh:55,sun:78,hi:88,nest:30.5,wq:2,bogus:7";
  parseLine(line);
  CHECK(env.tds == 245); CHECK(env.cloud == 12); CHECK(env.water10 == 224); CHECK(env.air10 == 291);
  CHECK(env.rh == 55); CHECK(env.sun == 78); CHECK(env.heatIndexF == 88); CHECK(env.nest10 == 305); CHECK(env.wq == 2);
  CHECK(envFresh());
  fakeMillis += 6000; CHECK(!envFresh());
  parseLine("bogus:1,nothing:2"); CHECK(!envFresh());            // unknown keys don't count as data
  parseLine("wq:0,clo"); parseLine("ud:99,hi:91"); CHECK(env.wq == 0); CHECK(env.heatIndexF == 91); CHECK(env.cloud == 12);  // torn line
  parseLine("waterwaterwater:5,rh:61"); CHECK(env.rh == 61);       // over-long key ignored, rest still parsed
  CHECK(femaleChance(300) == 500); CHECK(femaleChance(270) == 34); CHECK(femaleChance(330) == 985); CHECK(femaleChance(303) == 618);
  // sex ratio from 2000 hatchings at each temperature
  int temps[] = {270, 300, 315};
  for (int t : temps) { int f = 0; for (int i = 0; i < 2000; i++) { newEgg(); env.nest10 = t; tick(EGG_SECONDS); f += pet.female; }
    printf("nest %.1f C -> %d%% female\n", t / 10.0, f * 100 / 2000); }
  // only the middle third of incubation decides sex: warm only then -> mostly female
  { int f = 0;
    for (int i = 0; i < 1000; i++) {
      newEgg();
      for (int s = 0; s < EGG_SECONDS; s++) { env.nest10 = inSexWindow() ? 330 : 220; tick(1); }
      f += pet.female; }
    printf("warm only in middle third -> %d%% female\n", f / 10); CHECK(f > 900); }
  { int f = 0;
    for (int i = 0; i < 1000; i++) {
      newEgg();
      for (int s = 0; s < EGG_SECONDS; s++) { env.nest10 = inSexWindow() ? 220 : 330; tick(1); }
      f += pet.female; }
    printf("warm only outside middle third -> %d%% female\n", f / 10); CHECK(f < 100); }
  // ignored vs. active player, calm conditions, 10 minutes after hatching
  for (int active = 0; active < 2; active++) {
    newEgg(); env.nest10 = 300; env.wq = WQ_CLEAN; env.sun = 10; env.heatIndexF = 78; tick(EGG_SECONDS);
    int grewAt[4] = {0}; Stage last = pet.stage;
    for (int s = 1; s <= 600; s++) {
      if (active && s % 45 == 0) { addTo(pet.hunger, 40); addTo(pet.happy, 10); }   // one Crab Catch round, 8 crabs
      if (active && s % 10 == 0) addTo(pet.happy, 2);                              // petting
      tick(1);
      if (pet.stage != last) { grewAt[pet.stage] = s; last = pet.stage; }
    }
    printf("%s player: juvenile at %ds, adult at %ds (0 = never); end H%d J%d F%d C%d\n", active ? "active " : "ignored",
           grewAt[2], grewAt[3], pet.health, pet.happy, pet.hunger, pet.comfort);
  }
  // dirty water: time to rescue, then release
  newEgg(); tick(EGG_SECONDS); env.wq = WQ_AVOID; int s = 0; while (!pet.inRescue && s < 1000) { tick(1); s++; }
  printf("dirty water -> rescued after %d s\n", s); CHECK(pet.inRescue);
  env.wq = WQ_CLEAN; tick(RESCUE_SECONDS); CHECK(!pet.inRescue); CHECK(pet.health == 50);
  // link silence pauses water damage
  newEgg(); tick(EGG_SECONDS); env.wq = WQ_AVOID; uint8_t h = pet.health;
  for (int i = 0; i < 20; i++) { fakeMillis += 1000; petTick(); }   // no lastSeen updates
  printf("health during link silence: %d -> %d\n", h, pet.health);
  printf(fails ? "%d FAILED\n" : "all checks passed\n", fails);
  return fails;
}

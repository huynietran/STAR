# Test checklist

Run top to bottom before every rehearsal and on demo morning. Log each run at the
bottom: date and time, who ran it, pass or fail, and a note on anything odd.

## Turtle Board alone (`SIM_ENV 1`)

- [ ] Egg appears on power-up; top line changes Incubating... → Warm=girl Cool=boy → Almost hatching!
- [ ] Egg cracks, then hatches at 60 s with the ding and a girl/boy screen
- [ ] Buttons: A opens and cycles the menu, B selects, C goes back
- [ ] Crab Catch: crabs raise Food, a plastic bag lowers Health, round ends at 30 s
- [ ] Net Escape: hold B to rise, nets passed are counted, end screen mentions TEDs
- [ ] Hold B + C for 3 s: cycles calm, dirty water, heat + sun, warm nest
- [ ] Dirty-water scenario: sick bubble shows, Health falls
- [ ] Ignored turtle never grows; a cared-for one reaches adult
- [ ] Health hits 0: rescue screen, then released at 50 health
- [ ] Hold A + B for 3 s: skips to the next stage
- [ ] Hold A + C for 3 s: fresh egg

## Reef Station alone

- [ ] LCD cycles through all 5 pages
- [ ] Serial Monitor shows one line per second (`CAL_MODE 0`: no RAW lines)
- [ ] Probe in air: NO SAMPLE, all LEDs off
- [ ] Tap → yellow, Gulf (35 g/L salt) → green, Runoff → red
- [ ] Phone flashlight on the light sensor raises Sun %
- [ ] Pinching the TMP36 raises Nest above 30 °C within about 15 s

## Both boards linked (`SIM_ENV 0` on the Turtle Board)

- [ ] No `NO SIG`; unplugging the link wire shows it within 5 s and Health stops changing
- [ ] Runoff makes the turtle sick within a few seconds
- [ ] Pinching the nest sensor while the screen says Warm=girl Cool=boy → girl;
      not touching it → boy (try each twice)
- [ ] Flashlight held on the light sensor for 2 min → sunscreen reminder; Menu → Sunscreen clears it

## Demo readiness

- [ ] Full demo story fits in 4 minutes (use A + B to skip a stage if it runs long)
- [ ] Both boards run on power banks, no laptop, for 30+ minutes without resetting
- [ ] Reset for the next judge (A + C, rinse probe, probe in air) takes under 30 s
- [ ] Backup video recorded and saved on two laptops

## Run log

| Date and time | Who | Section | Pass / fail | Notes |
| --- | --- | --- | --- | --- |
| | | | | |

# Test checklist

Run top to bottom before every rehearsal and on demo morning. Log each run at the
bottom: date and time, who ran it, pass or fail, and a note on anything odd.

## Turtle Board alone (`SIM_ENV 1`)

- [ ] Egg appears on power-up; top line changes Incubating... (0–6 s) → Warm=girl Cool=boy (6–13 s) → Almost hatching!
- [ ] Egg cracks, then hatches at 20 s with the ding and a girl/boy screen
- [ ] Buttons: A opens and cycles the menu, B selects, C goes back
- [ ] Crab Catch: crabs raise Food, a plastic bag lowers Health, round ends at 20 s
- [ ] Net Escape: hold B to rise, nets passed are counted, end screen mentions TEDs
- [ ] Hold B + C for 3 s: cycles calm, dirty water, heat + sun, warm nest
- [ ] Dirty-water scenario: sick bubble shows, Health falls
- [ ] Ignored turtle never grows; a cared-for one reaches adult
- [ ] Health hits 0: rescue screen, then released at 50 health
- [ ] Hold A + B for 3 s: skips to the next stage
- [ ] Hold A + C for 3 s: fresh egg

## Reef Station alone

- [ ] LCD cycles through 4 pages: water verdict, air and humidity, feels-like, sun and nest
- [ ] Serial Monitor shows one line per second (`CAL_MODE 0`: no RAW lines)
- [ ] Probe resting in the Clean cup: green light, CLEAN on the LCD
- [ ] Clean tap water → green, Slightly cloudy → yellow, Runoff → red
- [ ] Phone flashlight on the light sensor raises Sun %
- [ ] Pinching the TMP36 raises Nest above 30 °C within about 15 s

## Both boards linked (`SIM_ENV 0` on the Turtle Board)

- [ ] No `NO SIG`; unplugging the link wire shows it within 5 s and Health stops changing
- [ ] Runoff makes the turtle sick within a few seconds
- [ ] Pinching the nest sensor from about 10 s before a new egg until it hatches → girl;
      not touching it → boy (try each twice)
- [ ] Flashlight held on the light sensor for 20 s → sunscreen reminder; Menu → Sunscreen clears it

## Demo readiness

- [ ] Full demo story fits in 4 minutes (use A + B to skip a stage if it runs long)
- [ ] Both boards run on power banks, no laptop, for 30+ minutes without resetting
- [ ] Reset for the next judge (A + C, rinse the probe, back in the Clean cup) takes under 30 s
- [ ] Backup video recorded and saved on two laptops

## Run log

| Date and time | Who | Section | Pass / fail | Notes |
| --- | --- | --- | --- | --- |
| | | | | |

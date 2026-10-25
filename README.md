# Ridley's Reef

A two-Arduino virtual pet that teaches beach and sea-turtle care. You raise a
Kemp's ridley, the smallest and most endangered sea turtle, from egg to adult on an
OLED screen. Its health reacts to real readings from a sensor station that checks
water cloudiness, heat, sun and nest temperature.

Hardware hackathon project, demo on Sunday, Oct 25, 2026.

## How it fits together

```
 Water cups      Air         Sun             Nest
 turbidity       DHT11       photoresistor   TMP36 (pinch to warm)
      \            |            |             /
       v           v            v            v
 +--------------------------------------------------------+
 | Reef Station (Board 2)  reef_station/                  |
 | smooths + calibrates readings, decides verdicts        |
 | LCD pages, green/yellow/red swim light, laptop graph   |
 +--------------------------------------------------------+
                  |  one text line per second, D1 -> D8, 9600 baud
                  v
 +--------------------------------------------------------+
 | Turtle Board (Board 1)  turtle_board/                  |
 | pet stats, life stages, games, alerts, journey map     |
 | OLED, 3 buttons, speaker                               |
 +--------------------------------------------------------+
```

## Repo layout

| Path | What | Owner |
| --- | --- | --- |
| `turtle_board/turtle_board.ino` | Setup and main loop | Turtle Brain |
| `turtle_board/config.h` | Pins, timings, `DEMO` and `SIM_ENV` switches | Turtle Brain |
| `turtle_board/env.h` | Link parser + fake data mode | Turtle Brain |
| `turtle_board/buttons.h` | Debounced buttons and 3-second combos | Turtle Brain |
| `turtle_board/pet.h` | Stats, hatching, growth, rescue | Turtle Brain |
| `turtle_board/ui.h` | Screens, menu, alerts | Turtle Brain |
| `turtle_board/screen.h` | OLED and drawing helpers | Turtle Brain |
| `turtle_board/games.h` | Crab Catch and Net Escape (both done) | Games, art and sound |
| `turtle_board/sprites.h` | Placeholder sprites, replace with real art | Games, art and sound |
| `turtle_board/sound.h` | Non-blocking jingles | Games, art and sound |
| `turtle_board/journey.h`, `journey_data.h` | Journey map page (Lucky's real track) | Games + Build/story |
| `reef_station/reef_station.ino` | All sensors, verdicts, LCD, LEDs, link sender | Reef Station |
| `reef_station/config.h` | Pins, calibration values, `NO_SALT_SENSOR`, `CAL_MODE` and DIY sensor switches | Reef Station |
| `docs/` | Pin map, link protocol, calibration, demo-day checklist | Everyone |
| `tools/` | Sprite and journey-map converters | Games / Build |
| `tests/` | Laptop tests for the link parser and pet logic | Turtle Brain |

## Getting started

1. Install **Arduino IDE 2**.
2. Library Manager: install **Adafruit SSD1306** (say yes to its dependencies:
   Adafruit GFX Library, Adafruit BusIO), **DHT sensor library** by Adafruit (yes to
   Adafruit Unified Sensor). LiquidCrystal, Wire and SoftwareSerial come with the IDE.
3. Board: **Arduino Uno**.
4. Open `turtle_board/turtle_board.ino` or `reef_station/reef_station.ino`. Every
   `.h` file in the folder opens as a tab.
5. Turtle Board without the Reef Station: leave `SIM_ENV 1` in `config.h`. Set it to
   `0` once the link wire is in.

## Controls (Turtle Board)

| Where | A | B | C |
| --- | --- | --- | --- |
| Home | Open menu | Pet (+2 happiness) | — |
| Menu | Next item | Select | Back |
| Crab Catch | Move left (hold) | — | Move right (hold) |
| Net Escape | — | Swim up (hold) | — |

Hold for 3 seconds: **A + C** new egg · **A + B** next life stage · **B + C** next
fake-data scenario (only with `SIM_ENV 1`).

## Team rules

- One owner per file (table above). Ask before editing someone else's file.
- Work on a branch named after you; merge to `main` only when it compiles.
- GitHub Actions compiles both sketches and runs the logic tests on every push.
  Fix a red X before anything else.
- Tag a known-good version every night: `git tag night-wed && git push --tags`.
- Photograph the breadboards whenever something works.

## Status

Must (MVP, Wednesday night)
- [x] Turtle with 4 stats and 3-button menu (starter code)
- [x] Crab Catch (starter code)
- [x] Reef Station reads, decides and sends (starter code)
- [ ] Everything wired and tested on real hardware
- [ ] Boards linked: dirty water makes the turtle sick

Demo setup (decided Saturday): no salt sensor (swim light from cloudiness only), no sand
or egg prop (judges pinch the nest sensor), enclosures shown as prototype designs.

Should (Friday, 9 pm freeze)
- [x] Egg that hatches male/female by nest temperature
- [x] Growth stages and rescue center
- [x] Sunscreen timer and "Applied" menu item
- [x] Traffic-light LEDs and laptop graph
- [x] Net Escape game
- [ ] Real sprites and sounds
- [x] Real tracked-turtle journey data (Lucky)
- [ ] Calibration values filled in

Could
- [ ] IMU tilt or ToF hand control
- [ ] Turtle trivia
- [ ] Hatchling Dash

## Memory budget (measured)

| Build | Flash (of 32,256) | Global RAM (of 2,048) |
| --- | --- | --- |
| Turtle Board, `SIM_ENV 0` | about 28.3 KB | about 720 bytes |
| Reef Station, no salt sensor | about 18 KB | about 430 bytes |

The OLED library takes another 1,024 bytes of RAM at startup that the IDE doesn't
show. **Keep the Turtle Board's "Global variables" under about 750 bytes.** If the
screen stays blank and the built-in LED blinks fast, it ran out of RAM.

Flash is the tighter limit on the Turtle Board, so: no `float` math (it costs about
2 KB), every string in `F("...")`, art in `PROGMEM`. If you still run out, edit
`Adafruit_SSD1306.h` in your libraries folder and uncomment `#define SSD1306_NO_SPLASH`,
which drops the 1.2 KB Adafruit logo.

## Art pipeline

Every sprite already exists as placeholder art in `turtle_board/sprites.h`, and the
code already uses all of them. To add real art, replace an array with yours, keeping the
**same name and same size**. No other file needs to change.

Draw in Piskel, convert with [image2cpp](https://javl.github.io/image2cpp/) (black
background, Horizontal, 1 bit per pixel), and paste. Preview the whole set without
uploading: `python3 tools/preview_sprites.py` writes `sprite_sheet.png`.
The team's Sprite Guide doc has the full list and step-by-step instructions.

## Journey data

The journey page follows **Lucky**, a real Kemp's ridley tagged in August 2026 at the
eastern tip of Massachusetts: 12 weekly positions (days 0–77) from Cape Cod through Long
Island Sound toward New York City. To swap in a different track:

1. Put the positions in a CSV (`day,lat,lon`), like `tools/sample_track.csv`.
2. Set the map bounds at the top of `tools/journey_to_progmem.py` to frame the route.
3. Run `python3 tools/journey_to_progmem.py track.csv coast.csv > turtle_board/journey_data.h`
4. Update the name, date and place labels in `drawJourney()` in `journey.h`.

## Tests

`sh tests/run.sh` (needs any C++ compiler) checks the link parser, the sex-from-
temperature odds, growth timing for an ignored vs. an active player, rescue timing,
that only the middle third of incubation decides the sex, and that a silent link never
hurts the turtle.

# Ridley's Reef

A two-Arduino virtual pet that teaches beach and sea-turtle care. You raise a
Kemp's ridley, the smallest and most endangered sea turtle, from egg to adult on an
OLED screen. Its health reacts to real readings from a sensor station that checks
water cloudiness and salt, heat, sun and nest temperature.

Hardware hackathon project, demo on Sunday, Oct 25, 2026.

## How it fits together

```
 Water cups      Air         Sun             Nest
 TDS, turbidity  DHT11       photoresistor   TMP36 in egg prop
 DS18B20
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
| `turtle_board/games.h` | Crab Catch (done), Net Escape (to build) | Games, art and sound |
| `turtle_board/sprites.h` | Placeholder sprites, replace with real art | Games, art and sound |
| `turtle_board/sound.h` | Non-blocking jingles | Games, art and sound |
| `turtle_board/journey.h`, `journey_data.h` | Journey map page (placeholder route) | Games + Build/story |
| `reef_station/reef_station.ino` | All sensors, verdicts, LCD, LEDs, link sender | Reef Station |
| `reef_station/config.h` | Pins, calibration values, DIY sensor switches | Reef Station |
| `docs/` | Pin map, link protocol, calibration, demo-day checklist | Everyone |
| `tools/` | Sprite and journey-map converters | Games / Build |
| `tests/` | Laptop tests for the link parser and pet logic | Turtle Brain |

## Getting started

1. Install **Arduino IDE 2**.
2. Library Manager: install **Adafruit SSD1306** (say yes to its dependencies:
   Adafruit GFX Library, Adafruit BusIO), **DHT sensor library** by Adafruit (yes to
   Adafruit Unified Sensor), **OneWire**, **DallasTemperature**. LiquidCrystal,
   Wire and SoftwareSerial come with the IDE.
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

Should (Friday, 9 pm freeze)
- [x] Egg that hatches male/female by nest temperature
- [x] Growth stages and rescue center
- [x] Sunscreen timer and "Applied" menu item
- [x] Traffic-light LEDs and laptop graph
- [ ] Net Escape game
- [ ] Real sprites and sounds
- [ ] Real tracked-turtle journey data
- [ ] Calibration values filled in

Could
- [ ] IMU tilt or ToF hand control
- [ ] Turtle trivia
- [ ] Hatchling Dash

## Memory budget (measured)

| Build | Flash (of 32,256) | Global RAM (of 2,048) |
| --- | --- | --- |
| Turtle Board, `SIM_ENV 0` | about 26.0 KB | about 720 bytes |
| Reef Station | about 19.6 KB | about 430 bytes |

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

`journey_data.h` holds a **placeholder** route along the coast, not a real track.
To use a real one:

1. On seaturtle.org's tracking pages, open the Padre Island National Seashore Kemp's
   ridley project and choose a turtle.
2. Put about 12 positions in a CSV (`day,lat,lon`), like `tools/sample_track.csv`.
3. Run `python3 tools/journey_to_progmem.py track.csv tools/sample_coast.csv > turtle_board/journey_data.h`
4. Set `JOURNEY_IS_PLACEHOLDER` to `false` in `journey.h`.

## Tests

`sh tests/run.sh` (needs any C++ compiler) checks the link parser, the sex-from-
temperature odds, growth timing for an ignored vs. an active player, rescue timing,
and that a silent link never hurts the turtle.

# Link protocol (Reef Station → Turtle Board)

Once a second the Reef Station prints one line of `key:value` pairs at 9600 baud.
It goes out on D1 to the Turtle Board and over USB to the laptop, where Arduino IDE's
Serial Plotter graphs it. Change this format only with both board owners agreeing.

```
tds:245,cloud:12,water:22.4,air:29.1,rh:55,sun:78,hi:88,nest:30.5,wq:0
```

| Key | Meaning | Units and range |
| --- | --- | --- |
| `tds` | Dissolved salts (TDS sensor). Not sent with `NO_SALT_SENSOR 1` (current demo), unless `SIM_SALT 1` sends a simulated 1000 or 300. | ppm, 0–1000; 1000 = at or over the sensor's limit |
| `sal` | Salinity (DIY salt probe, sent instead of `tds`) | ppt, 0 to about 40 |
| `cloud` | Cloudiness vs. clear-water baseline | %, 0–100 |
| `water` | Water temperature | °C, one decimal |
| `air` | Air temperature | °C, one decimal |
| `rh` | Relative humidity | % |
| `sun` | Light level, scaled by calibration | %, 0–100 |
| `hi` | Heat index including the sun correction | °F |
| `nest` | Nest sensor temperature (judges pinch it to warm the nest) | °C, one decimal |
| `wq` | Water verdict | 0 clean, 1 caution, 2 avoid, 3 no sample (probe dry) |

## Rules

- The Reef Station owns every verdict (`wq`, `hi`). The Turtle Board only reacts.
- The Turtle Board parses pairs as characters arrive and ignores unknown keys, so
  adding a key never breaks the other board.
- No valid pair for 5 seconds → the Turtle Board shows `NO SIG` and pauses all
  environment effects. A loose wire never hurts the turtle.
- The Reef Station sends the line one pair at a time, about 30 ms apart (`PAIR_GAP_MS`
  in `reef_station.ino`). The Turtle Board's software serial buffer holds only 63
  characters and can't empty while a frame is being drawn, so a whole line at once can
  overflow it, lose the end of the line, and flicker `NO SIG`. Keep the gaps if you add keys.
- `wq:3` (probe dry) also has no effect on the turtle. Without it, a probe sitting in
  air reads as fresh water and the turtle would slowly sicken between demos.

## Testing the link without the other board

- Reef Station alone: open Serial Monitor at 9600 and watch the lines.
- Turtle Board alone: set `SIM_ENV 1` in `turtle_board/config.h`. Hold B + C for
  3 s to cycle scenarios: calm, dirty water, heat + sun, warm nest.

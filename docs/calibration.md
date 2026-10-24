# Calibration

> **Current demo: no salt sensor** (`NO_SALT_SENSOR 1` in `reef_station/config.h`). Skip every salt and TDS step below;
> calibrate only the turbidity sensor (`CLOUD_V_CLEAR`, `CLOUD_V_MURKY`, `CLOUD_CAUTION`, `CLOUD_AVOID`),
> the light sensor and the nest offset. The swim light then works from cloudiness alone:
> clean → green, slightly cloudy → yellow, runoff → red.

Calibration is what makes the numbers believable to judges. Do a full session on
Wednesday, then redo the clear-water baseline and the light threshold in the demo
room on Sunday. Record every reading in `calibration-log.csv`.

## What to mix

| Cup | Recipe | Used for |
| --- | --- | --- |
| Clean | Distilled water | Baseline and rinsing |
| Seawater | 35 g salt in 1 L water (about ocean salinity) | Salt threshold, demo "Gulf" cup |
| Salt 5, Salt 15 | 5 g/L and 15 g/L | DIY salt probe curve only |
| Milk 1, 2, 5, 10 | 1, 2, 5, 10 mL milk in 1 L water | Repeatable cloudiness standards |
| Tap | Tap water | Demo cup |
| Runoff | Water + a spoon of soil, stirred | Demo cup (not a standard: it varies) |

## Procedure

1. Open Serial Monitor on the Reef Station at 9600.
2. Dip the probe, wait 10 seconds, then write down the values. Do each cup 3 times.
3. **Rinse in distilled water and pat dry between every cup.** Salt left on a probe
   ruins the next reading.
4. To get raw voltages, temporarily print `cloudRaw.get() * VREF / 1023.0` and
   `saltRaw.get()` in `sendLine()`.
5. Put the results into `reef_station/config.h`:

| Constant | Set it to |
| --- | --- |
| `CLOUD_V_CLEAR` | Turbidity volts in distilled water |
| `CLOUD_V_MURKY` | Turbidity volts in the Milk 10 cup |
| `CLOUD_CAUTION`, `CLOUD_AVOID` | Cloudiness % halfway between the cups you call clean, cloudy and murky |
| `TDS_DRY_MAX` | Below the tap-water reading, above the dry-probe reading |
| `TDS_FRESH_MAX` | Halfway between tap water and the seawater cup (seawater pins near 1000) |
| `SALT_CAL_RAW[4]` | DIY probe raw readings in 0, 5, 15, 35 g/L |
| `DIY_CLOUD_CLEAR_SIGNAL` | DIY cell signal in distilled water |
| `LIGHT_ROOM`, `LIGHT_SUN` | Light raw reading under room light, and with the lamp or flashlight on it |
| `NEST_OFFSET_C` | DHT11 air °C minus TMP36 °C, both at room temperature |

## Expected demo result

| Cup | Light | Why |
| --- | --- | --- |
| Tap | Yellow | Clear but fresh: at a beach, fresh water usually means runoff |
| Seawater | Green | Clear and salty, like healthy Gulf water |
| Runoff | Red | Cloudy |
| Probe in air | Off | No sample |

This is a screening signal, not a safety test. Texas's beach program decides
advisories from Enterococcus bacteria lab counts, which no cheap sensor can measure.

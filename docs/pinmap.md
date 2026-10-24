# Pin map

The single source of truth for wiring. If you move a wire, change this file and the
board's `config.h` in the same commit.

## Board 1: Turtle Board

| Pin | Connects to | Notes |
| --- | --- | --- |
| A4 (SDA), A5 (SCL) | OLED, plus IMU and ToF if used | One I2C bus. OLED usually 0x3C, MPU-6050 0x68, VL53L0X 0x29. |
| 5V, GND | OLED, IMU, amplifier power | Check the IMU breakout: some are 3.3 V only. |
| D2 | Button A (next / open menu) | Other leg to GND. `INPUT_PULLUP`, no resistor. |
| D3 | Button B (select / pet) | Same |
| D4 | Button C (back) | Same |
| D7 | Nothing | Reserved: SoftwareSerial needs a TX pin |
| D8 | Link from Reef Station D1, through 1 kΩ | SoftwareSerial receive, 9600 baud |
| D9 | 220 Ω → speaker red wire; speaker black wire → GND | `tone()`. Quiet but safe. Never wire the speaker straight to the pin (too much current). For louder sound: an amplifier board (e.g. PAM8403) or a TIP120 transistor. |
| GND | Reef Station GND | Required for the link |
| A0 | Nothing | Left floating on purpose: seeds the random generator |
| D5, D6, D10 | Optional mood RGB LED | On/off only. No `analogWrite` on D3/D11 (`tone()` uses their timer). |

## Board 2: Reef Station

| Pin | Connects to | Notes |
| --- | --- | --- |
| D12, D11, D5, D4, D3, D2 | LCD RS, E, D4, D5, D6, D7 | V0 to contrast pot wiper, RW to GND, backlight through 220 Ω |
| A0 | TDS sensor output | 0–2.3 V |
| A1 | Turbidity sensor output | Board switch on "A". Only the probe tip goes in water. |
| A2 | Photoresistor divider | Photoresistor 5 V → A2, 10 kΩ A2 → GND |
| A3 | TMP36 "nest" sensor, at the breadboard edge so judges can pinch it | Flat face toward you: left 5 V, middle signal, right GND. Pinching it warms the nest; only the middle third of the egg stage counts. |
| D7 | DHT11 data | Bare 4-pin DHT11 needs 10 kΩ to 5 V; modules have it |
| D8 | DS18B20 data | 4.7 kΩ pull-up to 5 V |
| D1 (TX) | 1 kΩ → Turtle Board D8 | Same data also goes to the laptop over USB |
| D6 | Green LED via 220 Ω | Clean |
| A4 | Yellow LED via 220 Ω | Caution |
| A5 | Red LED via 220 Ω | Avoid |
| D9, D10, D13 | Spare | Used by the DIY sensors below |

If an upload to Board 2 fails, unplug the D1 link wire and try again.

### DIY sensor wiring (only if the bought sensors are late)

| Sensor | Wiring | Switch in `config.h` |
| --- | --- | --- |
| Cloudiness cell | White LED on D13 (220 Ω); photoresistor into A1 with 10 kΩ to GND; both across a clear cup inside a cardboard sleeve | `USE_DIY_CLOUD 1` |
| Salt probe | Two stainless screws 1 cm apart. Screw 1 → D9. Screw 2 → A0, and through 1 kΩ → D10. | `USE_DIY_SALT 1` |

Running the salt probe and the TDS sensor together: move the probe's sense wire to A4,
the yellow LED to D13, and set `PIN_SALT_SENSE` to `A4`.

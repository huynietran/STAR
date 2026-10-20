#!/usr/bin/env python3
"""Turn ASCII art into a 1-bit PROGMEM bitmap for Adafruit GFX's drawBitmap().

Draw with '#' (pixel on) and '.' (pixel off), one row per line, all rows the
same width. Example file egg.txt:

    ..####..
    .#....#.
    #......#
    .#....#.
    ..####..

Run:
    python3 tools/ascii_sprite.py egg.txt SPR_EGG

and paste the output into turtle_board/sprites.h.

For PNG art (Piskel exports), use image2cpp instead:
https://javl.github.io/image2cpp/  (Draw mode: Horizontal, 1 bit per pixel)
"""
import sys


def to_progmem(rows, name):
    width = len(rows[0])
    if any(len(r) != width for r in rows):
        raise SystemExit("All rows must be the same width.")
    out = []
    for r in rows:
        padded = r + "." * ((8 - width % 8) % 8)
        for i in range(0, len(padded), 8):
            chunk = padded[i:i + 8]
            out.append(int("".join("1" if ch == "#" else "0" for ch in chunk), 2))
    lines = [f"// {width}x{len(rows)}",
             f"const uint8_t {name}[] PROGMEM = {{"]
    for i in range(0, len(out), 12):
        lines.append("  " + ", ".join(f"0x{b:02X}" for b in out[i:i + 12]) + ",")
    lines.append("};")
    return "\n".join(lines)


def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: ascii_sprite.py ART.txt SPRITE_NAME")
    with open(sys.argv[1]) as f:
        rows = [ln.rstrip("\n") for ln in f if ln.strip()]
    print(to_progmem(rows, sys.argv[2]))


if __name__ == "__main__":
    main()

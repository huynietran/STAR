#!/usr/bin/env python3
"""Render every sprite in turtle_board/sprites.h to sprite_sheet.png, so you can
check your art without uploading. Needs Pillow: pip install pillow
Run from the repo root: python3 tools/preview_sprites.py"""
import re, sys
from PIL import Image, ImageDraw, ImageFont
src = open(sys.argv[1] if len(sys.argv) > 1 else "turtle_board/sprites.h").read()
sprites = []
for m in re.finditer(r"// (\d+)x(\d+)\nconst uint8_t (\w+)\[\] PROGMEM = \{([^}]*)\}", src):
    w, h, name, body = int(m[1]), int(m[2]), m[3], m[4]
    data = [int(x, 16) for x in re.findall(r"0x[0-9A-F]{2}", body)]
    sprites.append((name, w, h, data))
P = 5; cellw, cellh = 32 * P + 40, 32 * P + 46; cols = 5
rows = (len(sprites) + cols - 1) // cols
img = Image.new("RGB", (cols * cellw, rows * cellh), (246, 246, 244))
d = ImageDraw.Draw(img)
try: font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 14)
except: font = ImageFont.load_default()
for i, (name, w, h, data) in enumerate(sprites):
    ox, oy = (i % cols) * cellw + 20, (i // cols) * cellh + 12
    d.rectangle([ox - 2, oy - 2, ox + 32 * P + 1, oy + 32 * P + 1], fill=(12, 16, 28))
    bpr = (w + 7) // 8
    x0 = ox + (32 - w) * P // 2; y0 = oy + (32 - h) * P // 2
    for r in range(h):
        for c in range(w):
            if data[r * bpr + c // 8] & (0x80 >> (c % 8)):
                d.rectangle([x0 + c * P, y0 + r * P, x0 + c * P + P - 2, y0 + r * P + P - 2], fill=(120, 220, 255))
    d.text((ox, oy + 32 * P + 8), f"{name}  {w}x{h}", fill=(30, 30, 30), font=font)
img.save("sprite_sheet.png")
print(f"Wrote sprite_sheet.png with {len(sprites)} sprites")

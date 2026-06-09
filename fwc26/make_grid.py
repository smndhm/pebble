#!/usr/bin/env python3
"""Generate a grid PNG from all fwc26 screenshots."""
from PIL import Image, ImageDraw, ImageFont
import os

HERE     = os.path.dirname(os.path.abspath(__file__))
SHOT_DIR = os.path.join(HERE, 'screenshots')
OUT      = os.path.join(SHOT_DIR, 'grid.png')

PLATFORMS    = ['basalt', 'aplite', 'chalk', 'diorite', 'emery', 'flint']
BW_PLAT      = {'aplite', 'diorite', 'flint'}
COLOR_TEAMS  = ['france', 'usa', 'canada', 'mexico']
COLOR_MODES  = ['logo', 'logo_pixel', 'sup_minutes', 'sup_hm', 'classic']
BW_MODES     = ['logo', 'sup_minutes', 'sup_hm', 'classic']

# Cell dimensions (native max: emery 200×228)
CELL_W = 200
CELL_H = 228
PAD    = 6   # padding around each image inside cell
LABEL_H = 22

# Build list of rows: (label, [(platform, team_or_None, mode_or_None), ...])
# mode_or_None is None when that mode doesn't apply to the platform (e.g. logo_pixel on B&W)
rows = []
for platform in PLATFORMS:
    is_bw = platform in BW_PLAT
    teams = [None] if is_bw else COLOR_TEAMS
    row_modes = BW_MODES if is_bw else COLOR_MODES
    for team in teams:
        label = f"{platform}" + (f" / {team}" if team else "")
        row_shots = [(platform, team, m if m in row_modes else None) for m in COLOR_MODES]
        rows.append((label, row_shots))

n_rows = len(rows)
n_cols = len(COLOR_MODES)

header_h = LABEL_H  # column headers
row_label_w = 140

total_w = row_label_w + n_cols * (CELL_W + PAD) + PAD
total_h = header_h + n_rows * (CELL_H + PAD + LABEL_H) + PAD

img = Image.new('RGB', (total_w, total_h), (24, 24, 24))
draw = ImageDraw.Draw(img)

try:
    font      = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf', 11)
    font_bold = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf', 11)
except Exception:
    font = font_bold = ImageFont.load_default()

# Column headers
for ci, mode in enumerate(COLOR_MODES):
    x = row_label_w + ci * (CELL_W + PAD) + PAD + CELL_W // 2
    draw.text((x, 4), mode, font=font_bold, fill=(200, 200, 200), anchor='mt')

# Rows
for ri, (label, shots) in enumerate(rows):
    y_top = header_h + ri * (CELL_H + PAD + LABEL_H) + PAD

    # Row label (left side)
    draw.text((4, y_top + CELL_H // 2), label, font=font, fill=(180, 180, 180), anchor='lm')

    for ci, (platform, team, mode) in enumerate(shots):
        x_cell = row_label_w + ci * (CELL_W + PAD) + PAD

        if mode is None:
            # Mode not applicable for this platform
            draw.rectangle([x_cell, y_top, x_cell + CELL_W, y_top + CELL_H], fill=(30, 30, 30))
            draw.text((x_cell + CELL_W // 2, y_top + CELL_H // 2), 'N/A', font=font, fill=(80, 80, 80), anchor='mm')
            continue

        if team:
            path = os.path.join(SHOT_DIR, platform, team, f'{mode}.png')
        else:
            path = os.path.join(SHOT_DIR, platform, f'{mode}.png')

        if not os.path.exists(path):
            # Draw a red placeholder
            draw.rectangle([x_cell, y_top, x_cell + CELL_W, y_top + CELL_H], fill=(80, 20, 20))
            draw.text((x_cell + CELL_W // 2, y_top + CELL_H // 2), '?', font=font, fill=(255, 80, 80), anchor='mm')
            continue

        shot = Image.open(path).convert('RGB')
        sw, sh = shot.size
        # Center in cell
        ox = x_cell + (CELL_W - sw) // 2
        oy = y_top  + (CELL_H - sh) // 2
        img.paste(shot, (ox, oy))

    # Row mode label below
    draw.text((row_label_w + PAD, y_top + CELL_H + 2), label, font=font, fill=(140, 140, 140))

img.save(OUT, optimize=True)
print(f'Grid saved → {OUT}  ({total_w}×{total_h})')

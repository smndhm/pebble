#!/usr/bin/env python3
"""
Generate a grid PNG from all watchface screenshots.
Usage: tools/make_grid.py <path/to/watchface/screenshots/>

Reads config.json in that directory for column ordering.
Scans the directory for images: platform/slug.png or platform/team/slug.png
"""
import sys, os, json, argparse
from PIL import Image, ImageDraw, ImageFont

PLATFORM_ORDER = ['basalt', 'aplite', 'chalk', 'diorite', 'emery', 'flint']
BW_PLATFORMS = {'aplite', 'diorite', 'flint'}

CELL_W = 200
CELL_H = 228
PAD = 6
LABEL_H = 22
ROW_LABEL_W = 150


def scan_shots(shots_dir):
    """Returns {(platform, team_or_None): {slug: path}}"""
    result = {}
    for entry in sorted(os.scandir(shots_dir), key=lambda e: e.name):
        if not entry.is_dir() or entry.name not in PLATFORM_ORDER:
            continue
        platform = entry.name
        for f in sorted(os.scandir(entry.path), key=lambda e: e.name):
            if f.is_file() and f.name.endswith('.png') and f.name != 'grid.png':
                result.setdefault((platform, None), {})[f.name[:-4]] = f.path
            elif f.is_dir():
                team = f.name
                for g in sorted(os.scandir(f.path), key=lambda e: e.name):
                    if g.is_file() and g.name.endswith('.png'):
                        result.setdefault((platform, team), {})[g.name[:-4]] = g.path
    return result


def slug_order_from_config(shots_dir):
    config_path = os.path.join(shots_dir, 'config.json')
    if not os.path.exists(config_path):
        return None
    with open(config_path) as f:
        config = json.load(f)
    slugs = []
    if 'matrix' in config:
        for mode in config['matrix'].get('modes', []):
            if mode['slug'] not in slugs:
                slugs.append(mode['slug'])
    if 'shots' in config:
        for s in config['shots']:
            if s['slug'] not in slugs:
                slugs.append(s['slug'])
    if 'bw_shots' in config:
        for s in config['bw_shots']:
            if s['slug'] not in slugs:
                slugs.append(s['slug'])
    return slugs or None


def make_grid(shots_dir):
    shots = scan_shots(shots_dir)
    if not shots:
        print('No screenshots found.')
        return

    ordered_slugs = slug_order_from_config(shots_dir)
    all_slugs = {slug for row in shots.values() for slug in row}
    if ordered_slugs:
        columns = [s for s in ordered_slugs if s in all_slugs]
        columns += sorted(all_slugs - set(columns))
    else:
        columns = sorted(all_slugs)

    def row_key(pk):
        platform, sub = pk
        pidx = PLATFORM_ORDER.index(platform) if platform in PLATFORM_ORDER else 99
        return (pidx, sub or '')

    rows = sorted(shots.keys(), key=row_key)

    n_cols, n_rows = len(columns), len(rows)
    total_w = ROW_LABEL_W + n_cols * (CELL_W + PAD) + PAD
    total_h = LABEL_H + n_rows * (CELL_H + PAD + LABEL_H) + PAD

    img = Image.new('RGB', (total_w, total_h), (24, 24, 24))
    draw = ImageDraw.Draw(img)

    try:
        font = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf', 11)
        font_bold = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf', 11)
    except Exception:
        font = font_bold = ImageFont.load_default()

    for ci, slug in enumerate(columns):
        x = ROW_LABEL_W + ci * (CELL_W + PAD) + PAD + CELL_W // 2
        draw.text((x, 4), slug, font=font_bold, fill=(200, 200, 200), anchor='mt')

    for ri, (platform, team) in enumerate(rows):
        label = f"{platform}" + (f" / {team}" if team else "")
        y_top = LABEL_H + ri * (CELL_H + PAD + LABEL_H) + PAD
        draw.text((4, y_top + CELL_H // 2), label, font=font, fill=(180, 180, 180), anchor='lm')

        row_shots = shots[(platform, team)]
        for ci, slug in enumerate(columns):
            x_cell = ROW_LABEL_W + ci * (CELL_W + PAD) + PAD
            path = row_shots.get(slug)
            if not path:
                draw.rectangle([x_cell, y_top, x_cell + CELL_W, y_top + CELL_H], fill=(30, 30, 30))
                draw.text((x_cell + CELL_W // 2, y_top + CELL_H // 2), 'N/A', font=font, fill=(80, 80, 80), anchor='mm')
                continue
            shot = Image.open(path).convert('RGB')
            sw, sh = shot.size
            img.paste(shot, (x_cell + (CELL_W - sw) // 2, y_top + (CELL_H - sh) // 2))

        draw.text((ROW_LABEL_W + PAD, y_top + CELL_H + 2), label, font=font, fill=(140, 140, 140))

    out = os.path.join(shots_dir, 'grid.png')
    img.save(out, optimize=True)
    print(f'Grid saved → {out}  ({total_w}×{total_h})')


def main():
    parser = argparse.ArgumentParser(description='Generate screenshot grid')
    parser.add_argument('screenshots_dir', help='Path to watchface screenshots/ directory')
    args = parser.parse_args()
    make_grid(os.path.abspath(args.screenshots_dir))


if __name__ == '__main__':
    main()

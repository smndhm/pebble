# tools

Shared scripts for all watchfaces in this monorepo.

```
tools/
  font/           ← glyph generation scripts (fwc26)
  screenshots/    ← emulator capture + grid compositor
```

---

## screenshots/

### `make_screenshots.py`

Captures screenshots for a watchface by driving the Pebble emulator.

**Requires:** pebble-tool Python (shebang auto-selects it)

```bash
tools/screenshots/make_screenshots.py watchfaces/fwc26/screenshots/config.json
tools/screenshots/make_screenshots.py watchfaces/lslt/screenshots/config.json
```

Reads `screenshots/config.json` in the watchface directory. Derives platforms from `package.json` and the app binary from `build/`. Outputs to `screenshots/<platform>/[team/]<slug>.png`.

**Config format — simple shots** (same set for all platforms):
```json
{ "shots": [
    { "slug": "thin", "time": [0, 0, 0], "appmessage": { "BGCOLOR": 0, "FGCOLOR": 16777215 } }
] }
```

**Config format — matrix** (color platforms) + **bw_shots** (B&W platforms):
```json
{
  "time": [20, 26, 0],
  "matrix": {
    "teams": [{ "slug": "france", "team_index": 23, "bg": 43775, "fg": 255, "fg2": 16711680, "col": 16777215 }],
    "modes": [{ "slug": "logo", "team_mode": "colors", "DISPLAY_MODE": 0, "LOGO_PIXEL": 0, "SUB_MODE": 0 }]
  },
  "bw_shots": [
    { "slug": "logo", "appmessage": { "DISPLAY_MODE": 0 } }
  ]
}
```

`team_mode: "colors"` sends `BGCOLOR`/`FGCOLOR`/`COLOR_RIGHT`/`COL_COLOR` from the team entry.
`team_mode: "index"` sends `TEAM_INDEX`.

### `make_grid.py`

Composes a grid PNG from all captured screenshots.

**Requires:** system Python + Pillow

```bash
tools/screenshots/make_grid.py watchfaces/fwc26/screenshots/
tools/screenshots/make_grid.py watchfaces/lslt/screenshots/
```

Reads `config.json` for column ordering. Outputs `screenshots/grid.png`.

### `pebble_emulator.py`

Internal module imported by `make_screenshots.py`. Provides helpers for controlling the Pebble emulator: `connect_emulator`, `set_time`, `send_appmessage`, `take_screenshot`, `shutdown_emulator`.

---

## font/

Scripts for generating `src/c/digit_paths.h` from glyph sources. Pass any watchface directory as argument.

### `svg_to_c.py`

Converts `<watchface>/resources/svg/0.svg` … `9.svg` into C path arrays.

```bash
python3 tools/font/svg_to_c.py watchfaces/fwc26
```

### `fontra_to_c.py`

Converts Fontra glyph JSON files into C path arrays.

```bash
python3 tools/font/fontra_to_c.py watchfaces/fwc26
# or with a custom glyphs directory:
python3 tools/font/fontra_to_c.py watchfaces/fwc26 --fontra ~/fonts/myfont/glyphs
```

Default Fontra path: `~/fonts/fwc26/Untitled.fontra/glyphs/`.

Both scripts produce identical output — use whichever source is available.

### `svg_to_pdc.py`

Converts SVGs to Pebble PDC format. Used by `watchfaces/claude-fails/` only — not needed for fwc26.

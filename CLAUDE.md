# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository Overview

A monorepo of Pebble/Rebble watchface projects, each a standalone Pebble SDK 3 app. All target the full platform set: aplite, basalt, chalk, diorite, emery, flint.

```
watchfaces/      ← one directory per watchface
  fwc26/         ← World Cup 2026 (active)
  lslt/          ← minimalist variable-font animation
  claude-fails/  ← archived: earlier WC2026 PDC approach
tools/
  font/          ← glyph generation (fwc26)
  screenshots/   ← emulator capture + grid compositor
```

| Watchface | Description |
|---|---|
| `watchfaces/fwc26/` | World Cup 2026 watchface using a custom FWC26 typeface (active development) |
| `watchfaces/lslt/` | Minimalist watchface that animates a variable font — digit weight grows over the minute/hour |
| `watchfaces/claude-fails/` | Earlier WC2026 attempt using pre-baked PDC resources instead of drawn paths |

## Build & Run

From inside a watchface directory:

```bash
cd watchfaces/fwc26
pebble build                              # build all platforms
pebble install --emulator basalt          # install on emulator (basalt = most common)
pebble install --emulator chalk           # round screen (Pebble Time Round)
pebble install --emulator emery           # large color screen (Pebble 2)
pebble logs --emulator basalt             # stream app logs
```

There are no test suites. Validation is by running on the emulator.

## Screenshots

Each watchface has a `screenshots/config.json` that defines shots (time + AppMessage config per platform). B&W vs color platform split is automatic from `package.json` `targetPlatforms`.

```bash
# Capture all screenshots for a watchface
tools/screenshots/make_screenshots.py watchfaces/fwc26/screenshots/config.json

# Generate the grid overview image
tools/screenshots/make_grid.py watchfaces/fwc26/screenshots/

# Same for lslt
tools/screenshots/make_screenshots.py watchfaces/lslt/screenshots/config.json
tools/screenshots/make_grid.py watchfaces/lslt/screenshots/
```

`make_screenshots.py` must be run with the pebble-tool Python (shebang handles this). `make_grid.py` uses system Python + Pillow.

## Architecture

### Pebble app structure

Every watchface follows the same pattern:
- `init()` → `app_event_loop()` → `deinit()` in `main()`
- Drawing happens in `layer_set_update_proc` callbacks — `layer_mark_dirty()` triggers a redraw
- Phone settings are sent via AppMessage, parsed in `inbox_received_callback`, and saved with `persist_write_*`
- Clay (`src/pkjs/index.js`) defines the phone-side settings UI; `messageKeys` in `package.json` declare the communication keys

### fwc26 — custom typeface rendering

Digits are stored as polygon point arrays in `src/c/digit_paths.h` (generated — do not edit directly). The coordinate space is `x ∈ [0,1000], y ∈ [0,750]`; `draw_digit()` scales them at draw time to any `GRect` via integer arithmetic:
```c
pt.x * rect.w / 1000 + origin.x
pt.y * rect.h / 750  + origin.y
```

Two display modes toggled by tap or settings:
- **Clock mode**: four digits H1 H2 M1 M2 in two vertically-stacked, overlapping pairs. The overlap column is painted by a separate `s_overlap_layers[]` on top, blending to `TRANSCOLOR` in that zone (color platforms only).
- **Logo mode**: just the minute digits (M1 M2), stacked vertically at a larger size.

Platform-specific digit sizes are set via `#ifdef PBL_PLATFORM_EMERY / PBL_ROUND / else` at the top of `fwc26.c`.

#### Regenerating digit_paths.h

Two scripts produce the same output format — use whichever source you have:

```bash
# From Fontra glyph editor (requires ~/fonts/fwc26/Untitled.fontra/glyphs/)
python3 tools/font/fontra_to_c.py watchfaces/fwc26

# From SVG files in resources/svg/
python3 tools/font/svg_to_c.py watchfaces/fwc26
```

`tools/font/svg_to_pdc.py` converts `resources/svg/` → `resources/digits/*.pdc` (used by `claude-fails`, not `fwc26`).

### lslt — variable font animation

Glyphs are axis-aligned rectangles (up to `MAX_CONTOURS=6` contours of 4 points each), not outline paths. Two data arrays per digit (`*_regular`, `*_bold`) are interpolated using fixed-point arithmetic (`FP_SHIFT=8`).

The interpolation factor encodes time progression:
- **Hour digits**: weight = minutes/59 (grows bolder across the hour, resets on hour change)
- **Minute digits**: weight = seconds/59 (grows bolder across the minute, resets on minute change)

Glyph results are cached in `CachedGlyph` structs and only recalculated when the digit or interpolation value changes, reducing per-second CPU work.

### claude-fails — PDC resource approach

Digits are pre-converted SVG→PDC files embedded as raw resources. The wscript converts `resources/svg/*.svg` → `resources/digits/*.pdc` at build time using `tools/font/svg_to_pdc.py`. Separate PDC sets exist for std (aplite/basalt/diorite/flint), chalk (round), and emery platforms.

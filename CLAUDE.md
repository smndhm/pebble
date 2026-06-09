# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository Overview

A monorepo of Pebble/Rebble watchface projects, each a standalone Pebble SDK 3 app. All target the full platform set: aplite, basalt, chalk, diorite, emery, flint.

| Project | Description |
|---|---|
| `fwc26/` | FIFA World Cup 2026 watchface using a custom FWC26 typeface (active development) |
| `lslt/` | Minimalist watchface that animates a variable font — digit weight grows over the minute/hour |
| `claude-fails/` | Earlier WC2026 attempt using pre-baked PDC resources instead of drawn paths |

## Build & Run

From inside a project directory:

```bash
pebble build                              # build all platforms
pebble install --emulator basalt          # install on emulator (basalt = most common)
pebble install --emulator chalk           # round screen (Pebble Time Round)
pebble install --emulator emery           # large color screen (Pebble 2)
pebble logs --emulator basalt             # stream app logs
```

There are no test suites. Validation is by running on the emulator.

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
cd fwc26 && python3 fontra_to_c.py

# From SVG files in resources/svg/
cd fwc26 && python3 svg_to_c.py
```

`fwc26/wscript` also calls `svg2pdc.py` at build time to convert `resources/svg/` → `resources/digits/*.pdc` (used by `claude-fails`, not `fwc26`).

### lslt — variable font animation

Glyphs are axis-aligned rectangles (up to `MAX_CONTOURS=6` contours of 4 points each), not outline paths. Two data arrays per digit (`*_regular`, `*_bold`) are interpolated using fixed-point arithmetic (`FP_SHIFT=8`).

The interpolation factor encodes time progression:
- **Hour digits**: weight = minutes/59 (grows bolder across the hour, resets on hour change)
- **Minute digits**: weight = seconds/59 (grows bolder across the minute, resets on minute change)

Glyph results are cached in `CachedGlyph` structs and only recalculated when the digit or interpolation value changes, reducing per-second CPU work.

### claude-fails — PDC resource approach

Digits are pre-converted SVG→PDC files embedded as raw resources. The wscript converts `resources/svg/*.svg` → `resources/digits/*.pdc` at build time using `svg2pdc.py`. Separate PDC sets exist for std (aplite/basalt/diorite/flint), chalk (round), and emery platforms.

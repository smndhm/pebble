# fwc26 — FIFA World Cup 2026 Watchface

**Date:** 2026-05-25
**Platforms:** aplite, basalt, chalk, diorite, emery, flint

---

## Overview

A static digital watchface using a custom typeface designed for FIFA World Cup 2026. Two display modes: a full clock (HH MM) and a "logo mode" showing only the minutes as two large centered digits, evoking the WC26 logo. Mode is configurable via Clay settings; a tap gesture can temporarily switch modes with a color-change transition.

---

## Build Pipeline

- **Source assets:** `resources/svg/0.svg` … `resources/svg/9.svg` (custom fwc26 typeface, viewBox 1000×750)
- **Auto-conversion:** `wscript` runs `svg2pdc.py` before each build, generating `resources/digits/{std,chalk,emery}_{0..9}.pdc` (30 files total)
- **Resources:** `package.json` declares all 30 PDCs as `"type": "raw"` entries
- **C loading:** `gdraw_command_image_create_with_resource()` at init, stored in a `GDrawCommandImage *s_digits[10]` array, freed on deinit

Platform sizes (matching `gen_pdc.py` / `claude-fails`):

| Platform | Height | Approx width (1000:750 ratio) |
|---|---|---|
| std (aplite/basalt/diorite/flint) | 68 px | 91 px |
| chalk | 82 px | 109 px |
| emery | 95 px | 127 px |

---

## Display Modes

### Mode Logo (default)
- Shows the 2 minute digits (MM) only
- Digits are scaled to fill the screen as large as possible, respecting the 1000:750 aspect ratio
- Centered horizontally and vertically

### Mode Horloge
- Shows 4 digits in a 2×2 grid: hours top row, minutes bottom row
- Layout follows `lslt` spacing: small gap between the two columns and rows
- Digits use the same PDC assets, scaled to platform sizes

---

## Tap Transition

When tap-to-switch is enabled (Clay setting):

1. User taps the watch face
2. Digits immediately change to the **transition color**
3. Display switches to the alternate mode (Logo ↔ Horloge)
4. After ~3 seconds, digits return to the normal foreground color and the display reverts to the default mode
5. Implemented with `accel_tap_service_subscribe` + `app_timer` (3000 ms)

---

## Clay Settings

| Setting | Type | Default | Description |
|---|---|---|---|
| Mode par défaut | Toggle | Logo | Logo mode or Horloge mode |
| Tap activé | Toggle | On | Enable tap-to-switch gesture |
| Couleur foreground | Color | Rouge (#C8102E → `0b11110000`) | Digit color |
| Couleur background | Color | Noir (`0b11000000`) | Background color |
| Couleur transition | Color | Or (`0b11111100`) | Color shown during tap switch |

Colors persist via `persist_write` / `persist_read`. On aplite/diorite/flint (1-bit), color pickers are hidden; only white on black is used.

---

## Official WC 2026 Colors (Pebble approximation)

| Name | Hex | Pebble GColor8 |
|---|---|---|
| Rouge | #C8102E | `0b11110000` |
| Bleu marine | #003DA5 | `0b11000011` |
| Or | #FFD700 | `0b11111100` |
| Blanc | #FFFFFF | `0b11111111` |
| Noir | #000000 | `0b11000000` |

---

## File Structure

```
fwc26/
├── package.json
├── wscript
├── svg2pdc.py              (copied/symlinked from dev tools)
├── src/
│   └── c/
│       └── fwc26.c
├── resources/
│   ├── svg/
│   │   ├── 0.svg … 9.svg
│   └── digits/             (auto-generated, gitignored)
│       ├── std_0.pdc … std_9.pdc
│       ├── chalk_0.pdc … chalk_9.pdc
│       └── emery_0.pdc … emery_9.pdc
└── src/js/
    └── clay-config.js
```

---

## Out of Scope

- Animations (no interpolation between digit weights)
- Date display
- Seconds
- World Cup match info or schedule

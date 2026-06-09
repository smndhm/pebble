# Pebble Watchfaces

A monorepo of [Rebble](https://rebble.io) / Pebble SDK 3 watchfaces.

```
watchfaces/
  fwc26/    ← World Cup 2026
  lslt/     ← minimalist variable-font animation
tools/
  font/       ← glyph generation scripts
  screenshots/ ← emulator capture + grid compositor
```

---

## fwc26 — World Cup 2026

A watchface inspired by the FWC2026 typeface. Digits are drawn as filled polygons scaled at runtime. Three display modes (logo, overlap, classic) with per-team color themes.

| basalt | chalk |
|--------|-------|
| ![](watchfaces/fwc26/screenshots/basalt/france/logo.png) | ![](watchfaces/fwc26/screenshots/chalk/france/logo.png) |
| ![](watchfaces/fwc26/screenshots/basalt/france/sup_hm.png) | ![](watchfaces/fwc26/screenshots/chalk/france/sup_hm.png) |
| ![](watchfaces/fwc26/screenshots/basalt/france/classic.png) | ![](watchfaces/fwc26/screenshots/chalk/france/classic.png) |

[→ Full grid](watchfaces/fwc26/screenshots/grid.png) · [→ README](watchfaces/fwc26/README.md)

---

## lslt — variable font animation

Digits interpolate between a thin and bold weight as time progresses — hour digits grow bolder over the hour, minute digits over the minute.

| thin | mid | bold |
|------|-----|------|
| ![](watchfaces/lslt/screenshots/basalt/thin.png) | ![](watchfaces/lslt/screenshots/basalt/mid.png) | ![](watchfaces/lslt/screenshots/basalt/bold.png) |

[→ README](watchfaces/lslt/README.md)

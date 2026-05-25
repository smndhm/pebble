# fwc26 Watchface Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Create the fwc26 Pebble watchface — a static clock using the custom FIFA WC 2026 typeface, with logo mode (MM only) and clock mode (HH MM), configurable via Clay with a tap-triggered color-change transition.

**Architecture:** Single C file (`fwc26.c`) loads 10 PDC images at init (generated from SVGs at build time by `wscript` calling `svg2pdc.py`). Two layout modes share the same PDC assets. Clay settings and persist handle theming and mode preferences. Tap gesture toggles mode temporarily with a color change.

**Tech Stack:** Pebble SDK 3 / waf, C, Python 3 (svg2pdc.py), Rebble Clay JS

---

## File Map

| File | Action | Responsibility |
|---|---|---|
| `fwc26/package.json` | Create | Project config, resource declarations (30 PDCs), Clay message keys |
| `fwc26/wscript` | Create | Build script — runs svg2pdc.py before Pebble SDK build |
| `fwc26/svg2pdc.py` | Create (copy+edit) | SVG→PDC converter with fwc26-specific platform sizes |
| `fwc26/.gitignore` | Create | Ignore `resources/digits/` (auto-generated) |
| `fwc26/resources/svg/2.svg` | Create (copy) | Only digit SVG available; others are placeholders |
| `fwc26/resources/svg/{0,1,3..9}.svg` | Create (placeholder) | Copies of 2.svg to allow a working build |
| `fwc26/src/c/fwc26.c` | Create | Full watchface: layout, draw, tick, Clay inbox, tap |
| `fwc26/src/pkjs/index.js` | Create | Clay config: 5 settings |

**Digit dimensions** (from `svg2pdc.py PLATFORMS`, viewBox 1000×750):

| Platform | DIGIT_H | DIGIT_W | 2-digit row | Screen W |
|---|---|---|---|---|
| std (aplite/basalt/diorite/flint) | 48 px | 64 px | 136 px | 144 px |
| chalk | 60 px | 80 px | 168 px | 180 px |
| emery | 68 px | 91 px | 190 px | 200 px |

---

## Task 1: Scaffold project structure

**Files:**
- Create: `fwc26/` directory tree
- Create: `fwc26/.gitignore`
- Create: `fwc26/package.json`

- [ ] **Step 1: Create directory tree**

```bash
mkdir -p /home/simonduhem/dev/pebble/fwc26/src/c
mkdir -p /home/simonduhem/dev/pebble/fwc26/src/pkjs
mkdir -p /home/simonduhem/dev/pebble/fwc26/resources/svg
mkdir -p /home/simonduhem/dev/pebble/fwc26/resources/digits
```

- [ ] **Step 2: Create `.gitignore`**

Create `/home/simonduhem/dev/pebble/fwc26/.gitignore`:
```
build/
resources/digits/
node_modules/
```

- [ ] **Step 3: Create `package.json`**

Create `/home/simonduhem/dev/pebble/fwc26/package.json`:
```json
{
  "name": "fwc26",
  "author": "smndhm",
  "description": "FIFA World Cup 2026 watchface using the custom fwc26 typeface.",
  "version": "0.1.0",
  "keywords": ["pebble", "watchface", "fifa", "world-cup", "wc2026", "rebble"],
  "private": true,
  "pebble": {
    "displayName": "FWC26",
    "uuid": "c2edb3a7-a11c-4e91-8c24-c4b87c7649de",
    "sdkVersion": "3",
    "enableMultiJS": true,
    "targetPlatforms": ["aplite", "basalt", "chalk", "diorite", "emery", "flint"],
    "watchapp": { "watchface": true },
    "resources": {
      "media": [
        { "type": "raw", "name": "DIGIT_STD_0",   "file": "digits/std_0.pdc"   },
        { "type": "raw", "name": "DIGIT_STD_1",   "file": "digits/std_1.pdc"   },
        { "type": "raw", "name": "DIGIT_STD_2",   "file": "digits/std_2.pdc"   },
        { "type": "raw", "name": "DIGIT_STD_3",   "file": "digits/std_3.pdc"   },
        { "type": "raw", "name": "DIGIT_STD_4",   "file": "digits/std_4.pdc"   },
        { "type": "raw", "name": "DIGIT_STD_5",   "file": "digits/std_5.pdc"   },
        { "type": "raw", "name": "DIGIT_STD_6",   "file": "digits/std_6.pdc"   },
        { "type": "raw", "name": "DIGIT_STD_7",   "file": "digits/std_7.pdc"   },
        { "type": "raw", "name": "DIGIT_STD_8",   "file": "digits/std_8.pdc"   },
        { "type": "raw", "name": "DIGIT_STD_9",   "file": "digits/std_9.pdc"   },
        { "type": "raw", "name": "DIGIT_CHALK_0", "file": "digits/chalk_0.pdc" },
        { "type": "raw", "name": "DIGIT_CHALK_1", "file": "digits/chalk_1.pdc" },
        { "type": "raw", "name": "DIGIT_CHALK_2", "file": "digits/chalk_2.pdc" },
        { "type": "raw", "name": "DIGIT_CHALK_3", "file": "digits/chalk_3.pdc" },
        { "type": "raw", "name": "DIGIT_CHALK_4", "file": "digits/chalk_4.pdc" },
        { "type": "raw", "name": "DIGIT_CHALK_5", "file": "digits/chalk_5.pdc" },
        { "type": "raw", "name": "DIGIT_CHALK_6", "file": "digits/chalk_6.pdc" },
        { "type": "raw", "name": "DIGIT_CHALK_7", "file": "digits/chalk_7.pdc" },
        { "type": "raw", "name": "DIGIT_CHALK_8", "file": "digits/chalk_8.pdc" },
        { "type": "raw", "name": "DIGIT_CHALK_9", "file": "digits/chalk_9.pdc" },
        { "type": "raw", "name": "DIGIT_EMERY_0", "file": "digits/emery_0.pdc" },
        { "type": "raw", "name": "DIGIT_EMERY_1", "file": "digits/emery_1.pdc" },
        { "type": "raw", "name": "DIGIT_EMERY_2", "file": "digits/emery_2.pdc" },
        { "type": "raw", "name": "DIGIT_EMERY_3", "file": "digits/emery_3.pdc" },
        { "type": "raw", "name": "DIGIT_EMERY_4", "file": "digits/emery_4.pdc" },
        { "type": "raw", "name": "DIGIT_EMERY_5", "file": "digits/emery_5.pdc" },
        { "type": "raw", "name": "DIGIT_EMERY_6", "file": "digits/emery_6.pdc" },
        { "type": "raw", "name": "DIGIT_EMERY_7", "file": "digits/emery_7.pdc" },
        { "type": "raw", "name": "DIGIT_EMERY_8", "file": "digits/emery_8.pdc" },
        { "type": "raw", "name": "DIGIT_EMERY_9", "file": "digits/emery_9.pdc" }
      ]
    },
    "messageKeys": ["BGCOLOR", "FGCOLOR", "TRANSCOLOR", "MODE_LOGO", "TAP_ENABLED"]
  },
  "dependencies": {
    "@rebble/clay": "^1.0.8"
  }
}
```

- [ ] **Step 4: Commit**

```bash
cd /home/simonduhem/dev/pebble
git add fwc26/.gitignore fwc26/package.json
git commit -m "feat(fwc26): scaffold project with package.json"
```

---

## Task 2: SVG assets and build pipeline

**Files:**
- Create: `fwc26/svg2pdc.py`
- Create: `fwc26/wscript`
- Create: `fwc26/resources/svg/*.svg` (1 real + 9 placeholders)

- [ ] **Step 1: Copy real SVG**

```bash
cp /home/simonduhem/fonts/fwc26/2.svg /home/simonduhem/dev/pebble/fwc26/resources/svg/2.svg
```

- [ ] **Step 2: Create placeholder SVGs for missing digits**

```bash
for d in 0 1 3 4 5 6 7 8 9; do
  cp /home/simonduhem/dev/pebble/fwc26/resources/svg/2.svg \
     /home/simonduhem/dev/pebble/fwc26/resources/svg/${d}.svg
done
```

- [ ] **Step 3: Create `svg2pdc.py` with fwc26 platform sizes**

Copy from `/home/simonduhem/dev/pebble/claude-fails/svg2pdc.py` then change ONLY the PLATFORMS constant.

Create `/home/simonduhem/dev/pebble/fwc26/svg2pdc.py` — same content as `claude-fails/svg2pdc.py` except replace:

```python
PLATFORMS = [
    ("std",   68),   # aplite/basalt/diorite/flint  144×168
    ("chalk", 82),   # chalk 180×180
    ("emery", 95),   # emery 200×228
]

OUT = os.path.join(os.path.dirname(__file__), "resources/digits")
```

with:

```python
PLATFORMS = [
    ("std",   48),   # aplite/basalt/diorite/flint  → 64×48 px per digit
    ("chalk", 60),   # chalk                        → 80×60 px per digit
    ("emery", 68),   # emery                        → 91×68 px per digit
]

OUT = os.path.join(os.path.dirname(__file__), "resources/digits")
```

- [ ] **Step 4: Verify svg2pdc.py generates correct PDCs**

```bash
cd /home/simonduhem/dev/pebble/fwc26
python3 svg2pdc.py resources/svg/2.svg resources/digits/
```

Expected output (3 lines):
```
[std] 64×48px  1 cmd(s): ...  → resources/digits/std_2.pdc (...)
[chalk] 80×60px  1 cmd(s): ...  → resources/digits/chalk_2.pdc (...)
[emery] 91×68px  1 cmd(s): ...  → resources/digits/emery_2.pdc (...)
```

- [ ] **Step 5: Create `wscript`**

Create `/home/simonduhem/dev/pebble/fwc26/wscript`:
```python
import os
import os.path
import sys
import glob
import subprocess

top = '.'
out = 'build'


def options(ctx):
    ctx.load('pebble_sdk')


def configure(ctx):
    ctx.load('pebble_sdk')


def build(ctx):
    # Generate PDCs from SVGs before the Pebble SDK build
    project_root = ctx.path.abspath()
    svg_dir  = os.path.join(project_root, 'resources', 'svg')
    out_dir  = os.path.join(project_root, 'resources', 'digits')
    svg2pdc  = os.path.join(project_root, 'svg2pdc.py')

    if os.path.isdir(svg_dir) and os.path.isfile(svg2pdc):
        os.makedirs(out_dir, exist_ok=True)
        for svg in sorted(glob.glob(os.path.join(svg_dir, '*.svg'))):
            subprocess.check_call([sys.executable, svg2pdc, svg, out_dir])

    ctx.load('pebble_sdk')

    build_worker = os.path.exists('worker_src')
    binaries = []
    cached_env = ctx.env

    for platform in ctx.env.TARGET_PLATFORMS:
        ctx.env = ctx.all_envs[platform]
        ctx.set_group(ctx.env.PLATFORM_NAME)
        app_elf = '{}/pebble-app.elf'.format(ctx.env.BUILD_DIR)
        ctx.pbl_build(
            source=ctx.path.ant_glob('src/c/**/*.c'),
            target=app_elf,
            bin_type='app'
        )
        binaries.append({'platform': platform, 'app_elf': app_elf})

    ctx.env = cached_env
    ctx.set_group('bundle')
    ctx.pbl_bundle(
        binaries=binaries,
        js=ctx.path.ant_glob(['src/pkjs/**/*.js', 'src/pkjs/**/*.json']),
        js_entry_file='src/pkjs/index.js'
    )
```

- [ ] **Step 6: Commit**

```bash
cd /home/simonduhem/dev/pebble
git add fwc26/svg2pdc.py fwc26/wscript fwc26/resources/svg/
git commit -m "feat(fwc26): add SVG assets and wscript SVG→PDC pipeline"
```

---

## Task 3: Base watchface — static clock mode

**Files:**
- Create: `fwc26/src/c/fwc26.c`

- [ ] **Step 1: Create `fwc26.c` with clock mode only**

Create `/home/simonduhem/dev/pebble/fwc26/src/c/fwc26.c`:
```c
#include <pebble.h>

// Digit pixel dimensions — sized to fit 2 per row, ratio 1000:750
#ifdef PBL_PLATFORM_EMERY
  #define DIGIT_W  91
  #define DIGIT_H  68
  #define DIGIT_RES(n) RESOURCE_ID_DIGIT_EMERY_##n
#elif defined(PBL_ROUND)
  #define DIGIT_W  80
  #define DIGIT_H  60
  #define DIGIT_RES(n) RESOURCE_ID_DIGIT_CHALK_##n
#else
  #define DIGIT_W  64
  #define DIGIT_H  48
  #define DIGIT_RES(n) RESOURCE_ID_DIGIT_STD_##n
#endif

#define GAP 8

// Persist keys
#define PERSIST_BGCOLOR      1
#define PERSIST_FGCOLOR      2
#define PERSIST_TRANSCOLOR   3
#define PERSIST_MODE_LOGO    4
#define PERSIST_TAP_ENABLED  5

// Message key fallbacks (generated header overrides these at build time)
#ifndef MESSAGE_KEY_BGCOLOR
#define MESSAGE_KEY_BGCOLOR     10000
#define MESSAGE_KEY_FGCOLOR     10001
#define MESSAGE_KEY_TRANSCOLOR  10002
#define MESSAGE_KEY_MODE_LOGO   10003
#define MESSAGE_KEY_TAP_ENABLED 10004
#endif

// Default colors (WC 2026 palette)
#define DEFAULT_BGCOLOR    ((GColor){.argb = 0b11000000}) // opaque black
#define DEFAULT_FGCOLOR    ((GColor){.argb = 0b11110000}) // WC red
#define DEFAULT_TRANSCOLOR ((GColor){.argb = 0b11111100}) // WC gold

static const uint32_t kDigitRes[10] = {
  DIGIT_RES(0), DIGIT_RES(1), DIGIT_RES(2), DIGIT_RES(3), DIGIT_RES(4),
  DIGIT_RES(5), DIGIT_RES(6), DIGIT_RES(7), DIGIT_RES(8), DIGIT_RES(9),
};

static Window            *s_window;
static Layer             *s_layer;
static struct tm          s_time;
static GDrawCommandImage *s_digits[10];
static GRect              s_clock_rects[4]; // H1 H2 M1 M2
static GRect              s_logo_rects[2];  // M1 M2

static GColor s_bgcolor;
static GColor s_fgcolor;
static GColor s_transcolor;
static bool   s_mode_logo;
static bool   s_tap_enabled;
static bool   s_tap_active;
static AppTimer *s_tap_timer;

static void recalculate_layout(GRect bounds) {
  int sw = bounds.size.w;
  int sh = bounds.size.h;

  // Clock mode: 2×2 grid centered
  int cx = (sw - (2 * DIGIT_W + GAP)) / 2;
  int cy = (sh - (2 * DIGIT_H + GAP)) / 2;
  s_clock_rects[0] = GRect(cx,              cy,              DIGIT_W, DIGIT_H);
  s_clock_rects[1] = GRect(cx + DIGIT_W + GAP, cy,           DIGIT_W, DIGIT_H);
  s_clock_rects[2] = GRect(cx,              cy + DIGIT_H + GAP, DIGIT_W, DIGIT_H);
  s_clock_rects[3] = GRect(cx + DIGIT_W + GAP, cy + DIGIT_H + GAP, DIGIT_W, DIGIT_H);

  // Logo mode: 2 digits, single row, centered
  int lx = (sw - (2 * DIGIT_W + GAP)) / 2;
  int ly = (sh - DIGIT_H) / 2;
  s_logo_rects[0] = GRect(lx,              ly, DIGIT_W, DIGIT_H);
  s_logo_rects[1] = GRect(lx + DIGIT_W + GAP, ly, DIGIT_W, DIGIT_H);
}

static void draw_digit(GContext *ctx, int digit, GRect r, GColor fg) {
  GDrawCommandImage *img  = s_digits[digit];
  GDrawCommandList  *cmds = gdraw_command_image_get_command_list(img);
  uint32_t n = gdraw_command_list_get_num_commands(cmds);
  for (uint32_t j = 0; j < n; j++) {
    GDrawCommand *cmd = gdraw_command_list_get_command(cmds, j);
    gdraw_command_set_hidden(cmd, false);
    gdraw_command_set_fill_color(cmd, fg);
    gdraw_command_set_stroke_width(cmd, 0);
  }
  gdraw_command_image_draw(ctx, img, r.origin);
}

static void layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, s_bgcolor);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  int h = s_time.tm_hour;
  if (!clock_is_24h_style()) {
    h = h % 12;
    if (h == 0) h = 12;
  }
  int m = s_time.tm_min;

  bool show_logo = s_mode_logo ^ s_tap_active;

#ifdef PBL_COLOR
  GColor fg = s_tap_active ? s_transcolor : s_fgcolor;
#else
  GColor fg = GColorWhite;
#endif

  if (show_logo) {
    draw_digit(ctx, m / 10, s_logo_rects[0], fg);
    draw_digit(ctx, m % 10, s_logo_rects[1], fg);
  } else {
    int d[4] = { h / 10, h % 10, m / 10, m % 10 };
    for (int i = 0; i < 4; i++) {
      draw_digit(ctx, d[i], s_clock_rects[i], fg);
    }
  }
}

static void tap_timer_callback(void *data) {
  s_tap_timer  = NULL;
  s_tap_active = false;
  layer_mark_dirty(s_layer);
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  if (!s_tap_enabled) return;
  if (s_tap_active) {
    if (s_tap_timer) { app_timer_cancel(s_tap_timer); s_tap_timer = NULL; }
    s_tap_active = false;
  } else {
    s_tap_active = true;
    s_tap_timer  = app_timer_register(3000, tap_timer_callback, NULL);
  }
  layer_mark_dirty(s_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  s_time = *tick_time;
  layer_mark_dirty(s_layer);
}

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
  Tuple *t = dict_read_first(iter);
  while (t) {
    if (t->key == MESSAGE_KEY_BGCOLOR) {
      s_bgcolor = GColorFromHEX((uint32_t)t->value->int32);
      persist_write_int(PERSIST_BGCOLOR, (int)t->value->int32);
      if (s_window) window_set_background_color(s_window, s_bgcolor);
    } else if (t->key == MESSAGE_KEY_FGCOLOR) {
      s_fgcolor = GColorFromHEX((uint32_t)t->value->int32);
      persist_write_int(PERSIST_FGCOLOR, (int)t->value->int32);
    } else if (t->key == MESSAGE_KEY_TRANSCOLOR) {
      s_transcolor = GColorFromHEX((uint32_t)t->value->int32);
      persist_write_int(PERSIST_TRANSCOLOR, (int)t->value->int32);
    } else if (t->key == MESSAGE_KEY_MODE_LOGO) {
      s_mode_logo = (bool)t->value->int32;
      persist_write_bool(PERSIST_MODE_LOGO, s_mode_logo);
    } else if (t->key == MESSAGE_KEY_TAP_ENABLED) {
      s_tap_enabled = (bool)t->value->int32;
      persist_write_bool(PERSIST_TAP_ENABLED, s_tap_enabled);
    }
    t = dict_read_next(iter);
  }
  if (s_layer) layer_mark_dirty(s_layer);
}

static void window_load(Window *window) {
  for (int i = 0; i < 10; i++) {
    s_digits[i] = gdraw_command_image_create_with_resource(kDigitRes[i]);
  }
  Layer *root   = window_get_root_layer(window);
  GRect  bounds = layer_get_bounds(root);
  recalculate_layout(bounds);
  s_layer = layer_create(bounds);
  layer_set_update_proc(s_layer, layer_update_proc);
  layer_add_child(root, s_layer);
  window_set_background_color(window, s_bgcolor);
  time_t now = time(NULL);
  s_time = *localtime(&now);
}

static void window_unload(Window *window) {
  for (int i = 0; i < 10; i++) {
    gdraw_command_image_destroy(s_digits[i]);
    s_digits[i] = NULL;
  }
  if (s_tap_timer) { app_timer_cancel(s_tap_timer); s_tap_timer = NULL; }
  layer_destroy(s_layer);
}

static void init(void) {
  s_bgcolor     = persist_exists(PERSIST_BGCOLOR)     ? GColorFromHEX(persist_read_int(PERSIST_BGCOLOR))  : DEFAULT_BGCOLOR;
  s_fgcolor     = persist_exists(PERSIST_FGCOLOR)     ? GColorFromHEX(persist_read_int(PERSIST_FGCOLOR))  : DEFAULT_FGCOLOR;
  s_transcolor  = persist_exists(PERSIST_TRANSCOLOR)  ? GColorFromHEX(persist_read_int(PERSIST_TRANSCOLOR)) : DEFAULT_TRANSCOLOR;
  s_mode_logo   = persist_exists(PERSIST_MODE_LOGO)   ? persist_read_bool(PERSIST_MODE_LOGO)               : true;
  s_tap_enabled = persist_exists(PERSIST_TAP_ENABLED) ? persist_read_bool(PERSIST_TAP_ENABLED)             : true;
  s_tap_active  = false;
  s_tap_timer   = NULL;

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
    .load   = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  accel_tap_service_subscribe(tap_handler);
  app_message_open(128, 0);
  app_message_register_inbox_received(inbox_received_callback);
}

static void deinit(void) {
  accel_tap_service_unsubscribe();
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
```

- [ ] **Step 2: Commit**

```bash
cd /home/simonduhem/dev/pebble
git add fwc26/src/c/fwc26.c
git commit -m "feat(fwc26): add main watchface C source"
```

---

## Task 4: Clay JS settings

**Files:**
- Create: `fwc26/src/pkjs/index.js`

- [ ] **Step 1: Install Clay dependency**

```bash
cd /home/simonduhem/dev/pebble/fwc26
npm install
```

Expected: creates `node_modules/` with `@rebble/clay`.

- [ ] **Step 2: Create `src/pkjs/index.js`**

Create `/home/simonduhem/dev/pebble/fwc26/src/pkjs/index.js`:
```javascript
const Clay = require('@rebble/clay');

new Clay([
  {
    type: 'heading',
    defaultValue: 'FWC26',
  },
  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Display',
      },
      {
        type: 'toggle',
        messageKey: 'MODE_LOGO',
        label: 'Mode Logo (MM seulement)',
        defaultValue: true,
      },
      {
        type: 'toggle',
        messageKey: 'TAP_ENABLED',
        label: 'Tap pour basculer',
        defaultValue: true,
      },
      {
        type: 'submit',
        defaultValue: 'Save',
      },
    ],
  },
  {
    type: 'section',
    capabilities: ['COLOR'],
    items: [
      {
        type: 'heading',
        defaultValue: 'Colors',
      },
      {
        type: 'color',
        messageKey: 'FGCOLOR',
        label: 'Chiffres',
        allowGray: false,
        defaultValue: 'C81428',
      },
      {
        type: 'color',
        messageKey: 'BGCOLOR',
        label: 'Fond',
        allowGray: false,
        defaultValue: '000000',
      },
      {
        type: 'color',
        messageKey: 'TRANSCOLOR',
        label: 'Transition (tap)',
        allowGray: false,
        defaultValue: 'FFFF00',
      },
      {
        type: 'submit',
        defaultValue: 'Save',
      },
    ],
  },
]);
```

- [ ] **Step 3: Commit**

```bash
cd /home/simonduhem/dev/pebble
git add fwc26/src/pkjs/index.js fwc26/package-lock.json
git commit -m "feat(fwc26): add Clay settings UI"
```

---

## Task 5: Build and test on emulator

**Files:** None new — verifies all previous tasks.

- [ ] **Step 1: Build**

```bash
cd /home/simonduhem/dev/pebble/fwc26
pebble build
```

Expected: build succeeds, `build/fwc26.pbw` is created. The wscript will auto-generate all 30 PDC files before compiling. If it fails with a missing PDC, check that Step 2 of Task 2 created all 10 placeholder SVGs.

- [ ] **Step 2: Test on basalt emulator (clock mode)**

```bash
pebble install --emulator basalt
```

Expected: watchface launches, shows 4 digits (HH MM) in 2×2 grid, white on black (basalt has color support, so red digits on black).

- [ ] **Step 3: Test logo mode via emulator settings**

In the Pebble app or using `pebble emu-app-config`, open Clay settings. Toggle "Mode Logo" on. Expected: only 2 digits (MM) centered on screen.

- [ ] **Step 4: Test tap gesture**

```bash
pebble emu-tap --emulator basalt
```

Expected: digits turn gold for 3 seconds, display switches to the alternate mode, then reverts.

- [ ] **Step 5: Test on chalk (round screen)**

```bash
pebble install --emulator chalk
```

Expected: watchface displays correctly on 180×180 round screen with `DIGIT_W=80, DIGIT_H=60`.

- [ ] **Step 6: Test on emery**

```bash
pebble install --emulator emery
```

Expected: correct on 200×228 screen with `DIGIT_W=91, DIGIT_H=68`.

- [ ] **Step 7: Test on aplite (1-bit, no color)**

```bash
pebble install --emulator aplite
```

Expected: digits display in white, color section of Clay is hidden.

- [ ] **Step 8: Commit if all tests pass**

```bash
cd /home/simonduhem/dev/pebble
git add fwc26/
git commit -m "feat(fwc26): first working build — all platforms"
```

---

## Notes for future work

- **Remaining SVGs (0, 1, 3–9):** Currently placeholders (copies of `2.svg`). As each digit is designed in Fontra and exported to `/home/simonduhem/fonts/fwc26/`, copy it to `fwc26/resources/svg/<n>.svg` and rebuild — the wscript picks it up automatically.
- **svg2pdc.py winding:** The "2" SVG has `fill="black"` which maps to Pebble `0xC0`. `draw_digit()` overrides fill color at draw time via `gdraw_command_set_fill_color`, so the PDC fill color is irrelevant at runtime — the SVG fill color doesn't matter for this watchface.
- **Clock mode 24h:** The watchface respects `clock_is_24h_style()` — in 12h mode, midnight/noon show as 12, not 0.

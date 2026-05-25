#include <pebble.h>

// PDCs are generated at logo size; clock mode scales coordinates at draw time.
#ifdef PBL_PLATFORM_EMERY
  #define DIGIT_W       91
  #define DIGIT_H       68
  #define LOGO_DIGIT_W  141
  #define LOGO_DIGIT_H  106
  #define DIGIT_RES(n)  RESOURCE_ID_DIGIT_EMERY_##n
#elif defined(PBL_ROUND)
  #define DIGIT_W       80
  #define DIGIT_H       60
  #define LOGO_DIGIT_W  109
  #define LOGO_DIGIT_H  82
  #define DIGIT_RES(n)  RESOURCE_ID_DIGIT_CHALK_##n
#else
  #define DIGIT_W       64
  #define DIGIT_H       48
  #define LOGO_DIGIT_W  101
  #define LOGO_DIGIT_H  76
  #define DIGIT_RES(n)  RESOURCE_ID_DIGIT_STD_##n
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
static GRect              s_logo_rects[2];  // M1 M2 (stacked vertically)

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
  s_clock_rects[0] = GRect(cx,                   cy,                   DIGIT_W, DIGIT_H);
  s_clock_rects[1] = GRect(cx + DIGIT_W + GAP,   cy,                   DIGIT_W, DIGIT_H);
  s_clock_rects[2] = GRect(cx,                   cy + DIGIT_H + GAP,   DIGIT_W, DIGIT_H);
  s_clock_rects[3] = GRect(cx + DIGIT_W + GAP,   cy + DIGIT_H + GAP,   DIGIT_W, DIGIT_H);

  // Logo mode: 2 digits stacked vertically, centered, larger size
  int lx = (sw - LOGO_DIGIT_W) / 2;
  int ly = (sh - (2 * LOGO_DIGIT_H + GAP)) / 2;
  s_logo_rects[0] = GRect(lx, ly,                       LOGO_DIGIT_W, LOGO_DIGIT_H);
  s_logo_rects[1] = GRect(lx, ly + LOGO_DIGIT_H + GAP,  LOGO_DIGIT_W, LOGO_DIGIT_H);
}

static void _color_digit(GDrawCommandImage *img, GColor fg) {
  GDrawCommandList *cmds = gdraw_command_image_get_command_list(img);
  uint32_t n = gdraw_command_list_get_num_commands(cmds);
  for (uint32_t j = 0; j < n; j++) {
    GDrawCommand *cmd = gdraw_command_list_get_command(cmds, j);
    gdraw_command_set_hidden(cmd, false);
    gdraw_command_set_fill_color(cmd, fg);
    gdraw_command_set_stroke_width(cmd, 0);
  }
}

// Draw at native (logo) size.
static void draw_digit(GContext *ctx, int digit, GRect r, GColor fg) {
  GDrawCommandImage *img = s_digits[digit];
  _color_digit(img, fg);
  gdraw_command_image_draw(ctx, img, r.origin);
}

// Draw scaled down to clock-mode size by temporarily mutating point coordinates.
// Saves originals on the stack and restores exactly — avoids integer drift on each tap.
#define SCALE_MAX_PTS 64
static void draw_digit_at_scale(GContext *ctx, int digit, GRect r, GColor fg) {
  GDrawCommandImage *img  = s_digits[digit];
  _color_digit(img, fg);

  GDrawCommandList *cmds = gdraw_command_image_get_command_list(img);
  uint32_t nc = gdraw_command_list_get_num_commands(cmds);

  GPoint saved[SCALE_MAX_PTS];
  uint16_t total = 0;

  for (uint32_t j = 0; j < nc; j++) {
    GDrawCommand *cmd = gdraw_command_list_get_command(cmds, j);
    uint16_t np = gdraw_command_get_num_points(cmd);
    for (uint16_t k = 0; k < np && total < SCALE_MAX_PTS; k++, total++) {
      saved[total] = gdraw_command_get_point(cmd, k);
      GPoint p = saved[total];
      p.x = (int16_t)(p.x * DIGIT_H / LOGO_DIGIT_H);
      p.y = (int16_t)(p.y * DIGIT_H / LOGO_DIGIT_H);
      gdraw_command_set_point(cmd, k, p);
    }
  }

  gdraw_command_image_draw(ctx, img, r.origin);

  // Restore from saved — exact, no drift
  total = 0;
  for (uint32_t j = 0; j < nc; j++) {
    GDrawCommand *cmd = gdraw_command_list_get_command(cmds, j);
    uint16_t np = gdraw_command_get_num_points(cmd);
    for (uint16_t k = 0; k < np && total < SCALE_MAX_PTS; k++, total++) {
      gdraw_command_set_point(cmd, k, saved[total]);
    }
  }
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
      draw_digit_at_scale(ctx, d[i], s_clock_rects[i], fg);
    }
  }
}

static void tap_timer_callback(void *data) {
  s_tap_timer  = NULL;
  s_tap_active = false;
  layer_mark_dirty(s_layer);
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
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
      if (s_tap_enabled) {
        accel_tap_service_subscribe(tap_handler);
      } else {
        accel_tap_service_unsubscribe();
        if (s_tap_timer) { app_timer_cancel(s_tap_timer); s_tap_timer = NULL; }
        s_tap_active = false;
      }
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
  s_bgcolor     = persist_exists(PERSIST_BGCOLOR)     ? GColorFromHEX(persist_read_int(PERSIST_BGCOLOR))    : DEFAULT_BGCOLOR;
  s_fgcolor     = persist_exists(PERSIST_FGCOLOR)     ? GColorFromHEX(persist_read_int(PERSIST_FGCOLOR))    : DEFAULT_FGCOLOR;
  s_transcolor  = persist_exists(PERSIST_TRANSCOLOR)  ? GColorFromHEX(persist_read_int(PERSIST_TRANSCOLOR)) : DEFAULT_TRANSCOLOR;
  s_mode_logo   = persist_exists(PERSIST_MODE_LOGO)   ? persist_read_bool(PERSIST_MODE_LOGO)                : true;
  s_tap_enabled = persist_exists(PERSIST_TAP_ENABLED) ? persist_read_bool(PERSIST_TAP_ENABLED)              : true;
  s_tap_active  = false;
  s_tap_timer   = NULL;

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
    .load   = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  if (s_tap_enabled) accel_tap_service_subscribe(tap_handler);
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

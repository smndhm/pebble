#include <pebble.h>
#include "digit_paths.h"
#include "themes.h"

// Display modes
typedef enum {
  DISPLAY_LOGO       = 0,  // M1 M2 stacked, no overlap
  DISPLAY_LOGO_PIXEL = 1,  // M1 M2 stacked, pixel grid in digit colors
  DISPLAY_MINUTES    = 2,  // M1 M2 side-by-side, large overlap
  DISPLAY_HM         = 3,  // H1H2 + M1M2, both pairs overlapping
  DISPLAY_CLASSIC    = 4,  // H1 H2 / M1 M2, 2×2 grid, no overlap
  DISPLAY_COUNT      = 5,
} DisplayMode;

// Persist keys
#define PERSIST_VERSION       0   // bump PERSIST_FORMAT_VER to wipe stale data
#define PERSIST_BGCOLOR       1
#define PERSIST_FGCOLOR       2
#define PERSIST_COLOR_RIGHT   3
#define PERSIST_DISPLAY_MODE  4
#define PERSIST_COL_COLOR     6
#define PERSIST_TEAM_INDEX    8

#define PERSIST_FORMAT_VER    9   // increment to force a reset on next boot

#ifndef MESSAGE_KEY_BGCOLOR
#define MESSAGE_KEY_BGCOLOR        10000
#define MESSAGE_KEY_FGCOLOR        10001
#define MESSAGE_KEY_COLOR_RIGHT    10002
#define MESSAGE_KEY_DISPLAY_MODE   10003
#define MESSAGE_KEY_TEAM_INDEX     10004
#define MESSAGE_KEY_COL_COLOR      10005
#define MESSAGE_KEY_REQUEST_CONFIG 10006
#define MESSAGE_KEY_LOGO_PIXEL     10007
#define MESSAGE_KEY_SUB_MODE       10008
#endif

#define DEFAULT_BGCOLOR     ((GColor){.argb = 0b11000000})  // black
#define DEFAULT_FGCOLOR     ((GColor){.argb = 0b11111111})  // white
#define DEFAULT_COLOR_RIGHT ((GColor){.argb = 0b11111111})  // white
#define DEFAULT_COL_COLOR   ((GColor){.argb = 0b11000000})  // black

static Window     *s_window;
static Layer      *s_layer;
static struct tm   s_time;
static GRect       s_hm_rects[4];   // DISPLAY_HM: H1 H2 M1 M2
static GRect       s_logo_rects[2]; // DISPLAY_LOGO: M1 M2
static GRect       s_min_rects[2];  // DISPLAY_MINUTES: M1 M2

static GColor      s_bgcolor;
static GColor      s_fgcolor;
static GColor      s_color_right;
static GColor      s_col_color;
static DisplayMode s_display_mode;
static int         s_team_index;

static GRect  s_classic_rects[4];  // DISPLAY_CLASSIC: H1 H2 M1 M2

static GPoint s_pts[DIGIT_PATH_MAX_PTS];
static GPoint s_pts2[DIGIT_PATH_MAX_PTS];

static uint16_t scale_digit_pts(int digit, GRect rect, GPoint *pts_out) {
  const DigitPath *dp = &kDigitPaths[digit];
  uint16_t n = dp->n;
  int32_t rw = rect.size.w, rh = rect.size.h;
  int16_t ox = rect.origin.x, oy = rect.origin.y;
  for (uint16_t i = 0; i < n; i++) {
    pts_out[i].x = (int16_t)(dp->pts[i].x * rw / 1000 + ox);
    pts_out[i].y = (int16_t)(dp->pts[i].y * rh / 750  + oy);
  }
  return n;
}

static void draw_digit(GContext *ctx, int digit, GRect rect, GColor color) {
  uint16_t n = scale_digit_pts(digit, rect, s_pts);
  GPathInfo pi = { n, s_pts };
  GPath *path = gpath_create(&pi);
  graphics_context_set_fill_color(ctx, color);
  gpath_draw_filled(ctx, path);
  gpath_destroy(path);
}

static bool point_in_polygon(int16_t px, int16_t py, const GPoint *pts, uint16_t n) {
  int inside = 0;
  for (uint16_t i = 0, j = n - 1; i < n; j = i++) {
    int32_t xi = pts[i].x, yi = pts[i].y;
    int32_t xj = pts[j].x, yj = pts[j].y;
    if ((yi > py) != (yj > py)) {
      int32_t dy = yj - yi;
      if (dy > 0 ? (px - xi) * dy < (xj - xi) * (py - yi)
                 : (px - xi) * dy > (xj - xi) * (py - yi)) {
        inside ^= 1;
      }
    }
  }
  return inside;
}

#ifdef PBL_COLOR
// Redraws pixels in the intersection of two digit polygons with the given color
static void draw_collision(GContext *ctx, int da, GRect ra, int db, GRect rb, GColor color) {
  int x0 = rb.origin.x;
  int x1 = ra.origin.x + ra.size.w;
  int y0 = ra.origin.y;
  int y1 = y0 + ra.size.h;
  if (x0 >= x1) return;

  uint16_t na = scale_digit_pts(da, ra, s_pts);
  uint16_t nb = scale_digit_pts(db, rb, s_pts2);

  graphics_context_set_stroke_color(ctx, color);
  for (int y = y0; y < y1; y++) {
    for (int x = x0; x < x1; x++) {
      if (point_in_polygon(x, y, s_pts, na) && point_in_polygon(x, y, s_pts2, nb)) {
        graphics_draw_pixel(ctx, GPoint(x, y));
      }
    }
  }
}
#else
// B&W: checkerboard dither in the digit intersection (simulates gray overlap zone)
static void draw_collision_dither(GContext *ctx, int da, GRect ra, int db, GRect rb) {
  int x0 = rb.origin.x;
  int x1 = ra.origin.x + ra.size.w;
  int y0 = ra.origin.y;
  int y1 = y0 + ra.size.h;
  if (x0 >= x1) return;

  uint16_t na = scale_digit_pts(da, ra, s_pts);
  uint16_t nb = scale_digit_pts(db, rb, s_pts2);

  graphics_context_set_stroke_color(ctx, GColorBlack);
  for (int y = y0; y < y1; y++) {
    for (int x = x0 + ((x0 + y) & 1); x < x1; x += 2) {
      if (point_in_polygon(x, y, s_pts, na) && point_in_polygon(x, y, s_pts2, nb)) {
        graphics_draw_pixel(ctx, GPoint(x, y));
      }
    }
  }
}
#endif

#ifdef PBL_COLOR
// Pixel grid in digit colors: 3-tone palette derived from fg (original, lighter, darker)
static void draw_digit_pixel(GContext *ctx, int digit, GRect rect, GColor fg, GColor bg) {
  uint16_t n = scale_digit_pts(digit, rect, s_pts);
  int x0 = rect.origin.x, y0 = rect.origin.y;
  int w = rect.size.w,    h = rect.size.h;
  int x1 = x0 + w,        y1 = y0 + h;

  uint8_t r = fg.r, g = fg.g, b = fg.b;
  GColor palette[3] = {
    fg,
    {.argb = (uint8_t)(0xC0 | ((uint8_t)(r<3?r+1:3)<<4) | ((uint8_t)(g<3?g+1:3)<<2) | (uint8_t)(b<3?b+1:3))},
    {.argb = (uint8_t)(0xC0 | ((uint8_t)(r>0?r-1:0)<<4) | ((uint8_t)(g>0?g-1:0)<<2) | (uint8_t)(b>0?b-1:0))},
  };

  int minute = s_time.tm_min;
  for (int y = y0; y < y1; y++) {
    int row = (y - y0) * 3 / h;
    for (int x = x0; x < x1; x++) {
      if (point_in_polygon(x, y, s_pts, n)) {
        int col = (x - x0) * 4 / w;
        unsigned int hv = (unsigned int)(row * 1013 + col * 509 + digit * 251 + minute * 127);
        hv ^= hv >> 5; hv ^= hv << 3;
        GColor pc = palette[hv % 3];
        if (gcolor_equal(pc, bg)) pc = palette[(hv + 1) % 3];
        graphics_context_set_stroke_color(ctx, pc);
        graphics_draw_pixel(ctx, GPoint(x, y));
      }
    }
  }
}
#endif // PBL_COLOR

static void recalculate_layout(GRect bounds) {
  int bw = bounds.size.w;
  int bh = bounds.size.h;
  int ox, oy, sw, sh, gap;

#ifdef PBL_ROUND
  // Circular safe zone: 8% inset keeps content within the round display.
  // Gap is kept small and independent of the inset.
  int inset = bw * 8 / 100;
  ox = inset; oy = inset;
  sw = bw - 2 * inset; sh = bh - 2 * inset;
  gap = 4;
#else
  gap = (bh > 200) ? 6 : 4;  // emery→6, all others→4
  ox = gap; oy = gap;
  sw = bw - 2 * gap; sh = bh - 2 * gap;
#endif

  // DISPLAY_LOGO: M1 M2 stacked vertically, gap ≈ 5.71% of digit height (FWC26 logo ratio)
  int logo_dh = (sh - gap) / 2;
  int logo_dw = logo_dh * 4 / 3;
  if (logo_dw > sw) { logo_dw = sw; logo_dh = logo_dw * 3 / 4; }
  int lx = (sw - logo_dw) / 2;
  int ly = (sh - (2 * logo_dh + gap)) / 2;
  s_logo_rects[0] = GRect(ox + lx, oy + ly,                 logo_dw, logo_dh);
  s_logo_rects[1] = GRect(ox + lx, oy + ly + logo_dh + gap, logo_dw, logo_dh);

  // DISPLAY_MINUTES: Panini diagonal, 50% h-overlap
  int mdw   = sw * 2 / 3;
  int mdh   = mdw * 3 / 4;
  int mhov  = mdw / 2;
  int mvoff = mdh / 2;
  int my    = (sh - (mdh + mvoff)) / 2;
  s_min_rects[0] = GRect(ox,               oy + my,          mdw, mdh);
  s_min_rects[1] = GRect(ox + mdw - mhov,  oy + my + mvoff,  mdw, mdh);

  // DISPLAY_HM: two overlapping pairs
  // Round: use /6 instead of /5 so the layout fits within the circular boundary
#ifdef PBL_ROUND
  int hm_dh   = (sh - gap) * 2 / 6 + gap;
#else
  int hm_dh   = (sh - gap) * 2 / 5;
#endif
  int hm_dw   = hm_dh * 4 / 3;
  if (hm_dw * 8 / 5 > sw) { hm_dw = sw * 5 / 8; hm_dh = hm_dw * 3 / 4; }
  int hm_hov  = hm_dw * 2 / 5;
  int hm_voff = hm_dh / 4;
  int hm_ph   = hm_dh + hm_voff;
  int hm_pw   = hm_dw * 2 - hm_hov;
  int hm_x0   = (sw - hm_pw) / 2;
  int hm_y0   = (sh - (2 * hm_ph + gap)) / 2;
  int hm_y1   = hm_y0 + hm_ph + gap;
  s_hm_rects[0] = GRect(ox + hm_x0,                   oy + hm_y0,            hm_dw, hm_dh);
  s_hm_rects[1] = GRect(ox + hm_x0 + hm_dw - hm_hov,  oy + hm_y0 + hm_voff, hm_dw, hm_dh);
  s_hm_rects[2] = GRect(ox + hm_x0,                   oy + hm_y1,            hm_dw, hm_dh);
  s_hm_rects[3] = GRect(ox + hm_x0 + hm_dw - hm_hov,  oy + hm_y1 + hm_voff, hm_dw, hm_dh);

  // DISPLAY_CLASSIC: 2×2 grid — inter-digit gap = gap (same h and v), gap margin on each side
  int cl_igap = gap;
  int cl_dw   = (sw - cl_igap - 2 * gap) / 2;  // leaves gap margin on each side
  int cl_dh   = cl_dw * 3 / 4;
  int cl_x0   = (sw - (2 * cl_dw + cl_igap)) / 2;
  int cl_x1   = cl_x0 + cl_dw + cl_igap;
  int cl_y0   = (sh - (2 * cl_dh + cl_igap)) / 2;
  int cl_y1   = cl_y0 + cl_dh + cl_igap;
  s_classic_rects[0] = GRect(ox + cl_x0, oy + cl_y0, cl_dw, cl_dh);
  s_classic_rects[1] = GRect(ox + cl_x1, oy + cl_y0, cl_dw, cl_dh);
  s_classic_rects[2] = GRect(ox + cl_x0, oy + cl_y1, cl_dw, cl_dh);
  s_classic_rects[3] = GRect(ox + cl_x1, oy + cl_y1, cl_dw, cl_dh);
}

#ifdef PBL_COLOR
static GColor readable_on(GColor bg) {
  int lum = ((bg.argb >> 4) & 0x3) + ((bg.argb >> 2) & 0x3) + (bg.argb & 0x3);
  return (lum >= 4) ? GColorBlack : GColorWhite;
}
#endif

static GColor display_bg(void) {
#ifdef PBL_COLOR
  if (gcolor_equal(s_bgcolor, s_fgcolor) || gcolor_equal(s_bgcolor, s_color_right)) {
    return readable_on(s_bgcolor);
  }
#endif
  return s_bgcolor;
}

static void layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  int h = s_time.tm_hour;
  if (!clock_is_24h_style()) { h %= 12; if (!h) h = 12; }
  int m = s_time.tm_min;

  GColor disp_bg = display_bg();
#ifdef PBL_COLOR
  GColor fg    = s_fgcolor;
  GColor right = s_color_right;
#else
  GColor fg    = GColorWhite;
  GColor right = GColorWhite;
#endif

  graphics_context_set_fill_color(ctx, disp_bg);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  switch (s_display_mode) {
    case DISPLAY_LOGO:
      draw_digit(ctx, m / 10, s_logo_rects[0], fg);
      draw_digit(ctx, m % 10, s_logo_rects[1], right);
      break;

    case DISPLAY_LOGO_PIXEL:
#ifdef PBL_COLOR
      draw_digit_pixel(ctx, m / 10, s_logo_rects[0], fg,    disp_bg);
      draw_digit_pixel(ctx, m % 10, s_logo_rects[1], right, disp_bg);
#endif
      break;

    case DISPLAY_MINUTES:
      draw_digit(ctx, m / 10, s_min_rects[0], fg);
      draw_digit(ctx, m % 10, s_min_rects[1], right);
#ifdef PBL_COLOR
      draw_collision(ctx, m/10, s_min_rects[0], m%10, s_min_rects[1], s_col_color);
#else
      draw_collision_dither(ctx, m/10, s_min_rects[0], m%10, s_min_rects[1]);
#endif
      break;

    case DISPLAY_HM: {
      int d[4] = { h/10, h%10, m/10, m%10 };
      draw_digit(ctx, d[0], s_hm_rects[0], fg);
      draw_digit(ctx, d[1], s_hm_rects[1], right);
      draw_digit(ctx, d[2], s_hm_rects[2], fg);
      draw_digit(ctx, d[3], s_hm_rects[3], right);
#ifdef PBL_COLOR
      draw_collision(ctx, d[0], s_hm_rects[0], d[1], s_hm_rects[1], s_col_color);
      draw_collision(ctx, d[2], s_hm_rects[2], d[3], s_hm_rects[3], s_col_color);
#else
      draw_collision_dither(ctx, d[0], s_hm_rects[0], d[1], s_hm_rects[1]);
      draw_collision_dither(ctx, d[2], s_hm_rects[2], d[3], s_hm_rects[3]);
#endif
      break;
    }

    case DISPLAY_CLASSIC: {
      int d[4] = { h/10, h%10, m/10, m%10 };
      draw_digit(ctx, d[0], s_classic_rects[0], fg);
      draw_digit(ctx, d[1], s_classic_rects[1], fg);
      draw_digit(ctx, d[2], s_classic_rects[2], right);
      draw_digit(ctx, d[3], s_classic_rects[3], right);
      break;
    }

    default:
      break;
  }
}

static void mark_all_dirty(void) {
  if (s_layer) layer_mark_dirty(s_layer);
}


#ifdef PBL_COLOR
// Robust persist read: handles both old 24-bit format and current 8-bit argb format.
// Valid Pebble opaque colors are 0xC0-0xFF; values below are invalid (old format or corrupt).
static GColor load_color(int key, GColor fallback) {
  if (!persist_exists(key)) return fallback;
  int v = persist_read_int(key);
  if (v > 0xFF) return GColorFromHEX((uint32_t)v);  // old 24-bit format
  if ((uint8_t)v >= 0xC0) return (GColor){.argb = (uint8_t)v};  // current 8-bit format
  return fallback;  // invalid/transparent — ignore
}
#endif

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  s_time = *tick_time;
  mark_all_dirty();
}

// Clay sends SELECT values as CSTRING ("0", "48" …) instead of TUPLE_INT.
static int tuple_int(Tuple *t) {
  return (t->type == TUPLE_CSTRING) ? atoi(t->value->cstring) : (int)t->value->int32;
}

// Clay sends color values as CSTRING hex ("FF0000") — parse accordingly.
static GColor tuple_color(Tuple *t) {
  uint32_t v = (t->type == TUPLE_CSTRING)
    ? (uint32_t)strtol(t->value->cstring, NULL, 16)
    : (uint32_t)t->value->int32;
  return GColorFromHEX(v);
}

static void send_config_to_js(void) {
  DictionaryIterator *iter;
  if (app_message_outbox_begin(&iter) != APP_MSG_OK) return;
  // Decompose internal mode into UI values: ui_mode 0=Logo 1=Superposition 2=Classique
  int32_t ui_mode  = (s_display_mode <= DISPLAY_LOGO_PIXEL) ? 0 :
                     (s_display_mode <= DISPLAY_HM)          ? 1 : 2;
  int32_t logo_pix = (s_display_mode == DISPLAY_LOGO_PIXEL) ? 1 : 0;
  int32_t sub_m    = (s_display_mode == DISPLAY_HM)         ? 1 : 0;
  int32_t bg    = s_bgcolor.argb;
  int32_t fg    = s_fgcolor.argb;
  int32_t right = s_color_right.argb;
  int32_t col   = s_col_color.argb;
  int32_t team  = s_team_index;
  dict_write_int(iter, MESSAGE_KEY_DISPLAY_MODE, &ui_mode,  4, true);
  dict_write_int(iter, MESSAGE_KEY_LOGO_PIXEL,   &logo_pix, 4, true);
  dict_write_int(iter, MESSAGE_KEY_SUB_MODE,     &sub_m,    4, true);
  dict_write_int(iter, MESSAGE_KEY_BGCOLOR,       &bg,       4, true);
  dict_write_int(iter, MESSAGE_KEY_FGCOLOR,       &fg,       4, true);
  dict_write_int(iter, MESSAGE_KEY_COLOR_RIGHT,   &right,    4, true);
  dict_write_int(iter, MESSAGE_KEY_COL_COLOR,     &col,      4, true);
  dict_write_int(iter, MESSAGE_KEY_TEAM_INDEX,    &team,     4, true);
  app_message_outbox_send();
}

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
  int    team_idx = -1;
  int    ui_mode  = -1;
  int    logo_pix = -1;
  int    sub_m    = -1;
  GColor new_bg   = {0}, new_fg = {0}, new_right = {0}, new_col = {0};
  bool   has_bg = false, has_fg = false, has_right = false, has_col = false;

  Tuple *t = dict_read_first(iter);
  while (t) {
    if      (t->key == MESSAGE_KEY_REQUEST_CONFIG) { send_config_to_js(); return; }
    else if (t->key == MESSAGE_KEY_DISPLAY_MODE)   ui_mode  = tuple_int(t);
    else if (t->key == MESSAGE_KEY_LOGO_PIXEL)     logo_pix = tuple_int(t);
    else if (t->key == MESSAGE_KEY_SUB_MODE)       sub_m    = tuple_int(t);
    else if (t->key == MESSAGE_KEY_TEAM_INDEX)     team_idx = tuple_int(t);
    else if (t->key == MESSAGE_KEY_BGCOLOR)      { new_bg    = tuple_color(t); has_bg    = true; }
    else if (t->key == MESSAGE_KEY_FGCOLOR)      { new_fg    = tuple_color(t); has_fg    = true; }
    else if (t->key == MESSAGE_KEY_COLOR_RIGHT)  { new_right = tuple_color(t); has_right = true; }
    else if (t->key == MESSAGE_KEY_COL_COLOR)    { new_col   = tuple_color(t); has_col   = true; }
    t = dict_read_next(iter);
  }

  // Compose internal display mode from UI values
  if (ui_mode >= 0) {
    if (ui_mode == 0) {
#ifndef PBL_COLOR
      logo_pix = 0;  // no pixel mode on B&W
#endif
      bool pixel = (logo_pix >= 0) ? (logo_pix > 0) : (s_display_mode == DISPLAY_LOGO_PIXEL);
      s_display_mode = pixel ? DISPLAY_LOGO_PIXEL : DISPLAY_LOGO;
    } else if (ui_mode == 1) {
      bool hm = (sub_m >= 0) ? (sub_m > 0) : (s_display_mode == DISPLAY_HM);
      s_display_mode = hm ? DISPLAY_HM : DISPLAY_MINUTES;
    } else {
      s_display_mode = DISPLAY_CLASSIC;
    }
    persist_write_int(PERSIST_DISPLAY_MODE, s_display_mode);
  }

  // Persist team selection
  if (team_idx >= 0) {
    s_team_index = team_idx;
    persist_write_int(PERSIST_TEAM_INDEX, team_idx);
  }

  // Team preset: superposition modes only
  bool is_sup = (s_display_mode == DISPLAY_MINUTES || s_display_mode == DISPLAY_HM);
  if (team_idx >= 0 && team_idx < TEAM_COUNT && is_sup) {
    s_fgcolor     = (GColor){.argb = kTeams[team_idx].fg};
    s_color_right = (GColor){.argb = kTeams[team_idx].fg2};
    s_col_color   = (GColor){.argb = kTeams[team_idx].col};
    persist_write_int(PERSIST_FGCOLOR,     s_fgcolor.argb);
    persist_write_int(PERSIST_COLOR_RIGHT, s_color_right.argb);
    persist_write_int(PERSIST_COL_COLOR,   s_col_color.argb);
    if (!has_bg) {
      s_bgcolor = (GColor){.argb = kTeams[team_idx].bg};
      persist_write_int(PERSIST_BGCOLOR, s_bgcolor.argb);
    }
  } else {
    if (has_fg)    { s_fgcolor     = new_fg;    persist_write_int(PERSIST_FGCOLOR,     s_fgcolor.argb); }
    if (has_right) { s_color_right = new_right; persist_write_int(PERSIST_COLOR_RIGHT, s_color_right.argb); }
    if (has_col)   { s_col_color   = new_col;   persist_write_int(PERSIST_COL_COLOR,   s_col_color.argb); }
  }
  if (has_bg) { s_bgcolor = new_bg; persist_write_int(PERSIST_BGCOLOR, s_bgcolor.argb); }

  if (s_window) window_set_background_color(s_window, display_bg());
  mark_all_dirty();
}

static void window_load(Window *window) {
  Layer *root   = window_get_root_layer(window);
  GRect  bounds = layer_get_bounds(root);

  s_layer = layer_create(bounds);
  layer_set_update_proc(s_layer, layer_update_proc);
  layer_add_child(root, s_layer);


  recalculate_layout(bounds);
  window_set_background_color(window, s_bgcolor);
  time_t now = time(NULL);
  s_time = *localtime(&now);
}

static void window_unload(Window *window) {
  layer_destroy(s_layer);
}

static void init(void) {
  // Wipe stale persist data if format version changed
  if (!persist_exists(PERSIST_VERSION) ||
      persist_read_int(PERSIST_VERSION) != PERSIST_FORMAT_VER) {
    persist_delete(PERSIST_BGCOLOR);
    persist_delete(PERSIST_FGCOLOR);
    persist_delete(PERSIST_COLOR_RIGHT);
    persist_delete(PERSIST_COL_COLOR);
    persist_delete(7);  // PERSIST_PIXEL_BGCOLOR (legacy)
    persist_delete(PERSIST_TEAM_INDEX);
    persist_delete(PERSIST_DISPLAY_MODE);
    persist_write_int(PERSIST_VERSION, PERSIST_FORMAT_VER);
  }

#ifdef PBL_COLOR
  s_bgcolor     = load_color(PERSIST_BGCOLOR,     DEFAULT_BGCOLOR);
  s_fgcolor     = load_color(PERSIST_FGCOLOR,     DEFAULT_FGCOLOR);
  s_color_right = load_color(PERSIST_COLOR_RIGHT, DEFAULT_COLOR_RIGHT);
  s_col_color   = load_color(PERSIST_COL_COLOR,   DEFAULT_COL_COLOR);
#else
  s_bgcolor     = GColorBlack;
  s_fgcolor     = GColorWhite;
  s_color_right = GColorWhite;
  s_col_color   = GColorBlack;
#endif
  int stored_mode = persist_exists(PERSIST_DISPLAY_MODE) ? persist_read_int(PERSIST_DISPLAY_MODE) : 0;
  s_display_mode  = (stored_mode >= 0 && stored_mode < DISPLAY_COUNT) ? (DisplayMode)stored_mode : DISPLAY_LOGO;
#ifndef PBL_COLOR
  if (s_display_mode == DISPLAY_LOGO_PIXEL) s_display_mode = DISPLAY_LOGO;
#endif
  s_team_index    = persist_exists(PERSIST_TEAM_INDEX) ? persist_read_int(PERSIST_TEAM_INDEX) : 48;

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
    .load   = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  app_message_register_inbox_received(inbox_received_callback);
  app_message_open(256, 256);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

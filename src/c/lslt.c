#include <pebble.h>
#include "digits.h"
#include "forced_time.h"

#define MIN2(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

typedef enum { ALIGN_LEFT = -1, ALIGN_CENTER = 0, ALIGN_RIGHT = 1 } Alignment;

// Glyph data structure
typedef struct {
  const Point (*regular)[POINTS_PER_CONTOUR];
  const Point (*bold)[POINTS_PER_CONTOUR];
  uint8_t num_contours;
} GlyphData;

// Cached glyph structure
typedef struct {
  Point glyph[MAX_CONTOURS][POINTS_PER_CONTOUR];
  uint8_t num_contours;
  int16_t gmin_x, gmin_y, gmax_x, gmax_y;
  bool valid;
} CachedGlyph;

// Global state
static Window *s_window;
static Layer *s_custom_layer;
static struct tm s_last_time;
static int16_t s_screen_width, s_screen_height;
static int16_t s_square_size, s_stroke_width, s_digit_size;
static int16_t s_cached_glyph_target_height = -1;
static GRect s_digit_rects[4];

// Glyph caches
static CachedGlyph s_hour_glyphs[2], s_min_glyphs[2];
static int8_t s_last_hour_digits[2] = {-1, -1};
static int8_t s_last_min_digits[2] = {-1, -1};
static float s_last_hour_interp = -1.0f, s_last_min_interp = -1.0f;

// Glyph lookup table (in ROM)
static const GlyphData GLYPH_TABLE[10] = {
  {zero_regular, zero_bold, ZERO_NUM_CONTOURS},
  {one_regular, one_bold, ONE_NUM_CONTOURS},
  {two_regular, two_bold, TWO_NUM_CONTOURS},
  {three_regular, three_bold, THREE_NUM_CONTOURS},
  {four_regular, four_bold, FOUR_NUM_CONTOURS},
  {five_regular, five_bold, FIVE_NUM_CONTOURS},
  {six_regular, six_bold, SIX_NUM_CONTOURS},
  {seven_regular, seven_bold, SEVEN_NUM_CONTOURS},
  {eight_regular, eight_bold, EIGHT_NUM_CONTOURS},
  {nine_regular, nine_bold, NINE_NUM_CONTOURS}
};

// Helper functions
static inline float compute_interpolation(int value, int max_value) {
  if (value < 0) return 0.0f;
  float interp = (float)value / (float)max_value;
  return CLAMP(interp, 0.0f, 1.0f);
}

static inline int8_t validate_digit(int digit) {
  return (digit >= 0 && digit <= 9) ? digit : 0;
}

// Pre-calculate glyph and cache it
static void precalculate_glyph(CachedGlyph *cached, const GlyphData *glyph_data, 
                               float interp, int16_t glyph_height)
{
  interpolate_glyph(glyph_data->regular, glyph_data->bold, cached->glyph, 
                    glyph_data->num_contours, interp, glyph_height, true);
  glyph_bounding_box((const Point (*)[POINTS_PER_CONTOUR])cached->glyph, glyph_data->num_contours, 
                     &cached->gmin_x, &cached->gmin_y, &cached->gmax_x, &cached->gmax_y);
  cached->num_contours = glyph_data->num_contours;
  cached->valid = true;
}

// Draw cached glyph with minimal copying
static void draw_cached_digit(GContext *ctx, const CachedGlyph *cached, 
                               GRect rect, Alignment alignment)
{
  if (!cached->valid) return;

  int16_t gw = cached->gmax_x - cached->gmin_x + 1;
  int16_t gh = cached->gmax_y - cached->gmin_y + 1;
  
  // Calculate position
  int16_t rect_cx = rect.origin.x + rect.size.w / 2;
  int16_t rect_cy = rect.origin.y + rect.size.h / 2;
  int16_t target_x = rect_cx - (gw / 2);
  
  if (alignment == ALIGN_LEFT) target_x = rect.origin.x;
  else if (alignment == ALIGN_RIGHT) target_x = rect.origin.x + rect.size.w - gw;
  
  int16_t target_y = rect_cy - (gh / 2);
  int16_t tx = target_x - cached->gmin_x;
  int16_t ty = target_y - cached->gmin_y;
  
  // Draw directly with translation - no buffer copy
  for (int c = 0; c < cached->num_contours; c++) {
    int16_t min_x = cached->glyph[c][0].x + tx;
    int16_t max_x = min_x;
    int16_t min_y = cached->glyph[c][0].y + ty;
    int16_t max_y = min_y;
    
    for (int p = 1; p < POINTS_PER_CONTOUR; p++) {
      int16_t px = cached->glyph[c][p].x + tx;
      int16_t py = cached->glyph[c][p].y + ty;
      if (px < min_x) min_x = px;
      if (px > max_x) max_x = px;
      if (py < min_y) min_y = py;
      if (py > max_y) max_y = py;
    }
    
    graphics_fill_rect(ctx, GRect(min_x, min_y, max_x - min_x + 1, max_y - min_y + 1), 
                       0, GCornerNone);
  }
}

// Main update function
static void custom_layer_update_proc(Layer *layer, GContext *ctx)
{
  // Initialize glyph height on first run
  if (s_cached_glyph_target_height < 0) {
    s_cached_glyph_target_height = (s_screen_width - DIGIT_EDGE_PADDING_H) / 2;
    if (s_cached_glyph_target_height <= 0) s_cached_glyph_target_height = BASE_GLYPH_SIZE;
  }

  // Extract digits
  int8_t hour_digits[2] = {
    validate_digit(s_last_time.tm_hour / 10),
    validate_digit(s_last_time.tm_hour % 10)
  };
  int8_t min_digits[2] = {
    validate_digit(s_last_time.tm_min / 10),
    validate_digit(s_last_time.tm_min % 10)
  };

  // Compute interpolations
  float hour_interp = compute_interpolation(s_last_time.tm_min, 59);
  float min_interp = compute_interpolation(s_last_time.tm_sec, 59);

  // Update hour cache if needed (only ~1x per minute)
  if (hour_digits[0] != s_last_hour_digits[0] || 
      hour_digits[1] != s_last_hour_digits[1] ||
      hour_interp != s_last_hour_interp) {
    for (int i = 0; i < 2; i++) {
      precalculate_glyph(&s_hour_glyphs[i], &GLYPH_TABLE[hour_digits[i]], 
                         hour_interp, s_cached_glyph_target_height);
    }
    s_last_hour_digits[0] = hour_digits[0];
    s_last_hour_digits[1] = hour_digits[1];
    s_last_hour_interp = hour_interp;
  }

  // Update minute cache if needed (60x per minute)
  if (min_digits[0] != s_last_min_digits[0] || 
      min_digits[1] != s_last_min_digits[1] ||
      min_interp != s_last_min_interp) {
    for (int i = 0; i < 2; i++) {
      precalculate_glyph(&s_min_glyphs[i], &GLYPH_TABLE[min_digits[i]], 
                         min_interp, s_cached_glyph_target_height);
    }
    s_last_min_digits[0] = min_digits[0];
    s_last_min_digits[1] = min_digits[1];
    s_last_min_interp = min_interp;
  }

  // Draw (always required)
  graphics_context_set_fill_color(ctx, GColorWhite);
  static const Alignment alignments[4] = {ALIGN_RIGHT, ALIGN_LEFT, ALIGN_RIGHT, ALIGN_LEFT};
  
  draw_cached_digit(ctx, &s_hour_glyphs[0], s_digit_rects[0], alignments[0]);
  draw_cached_digit(ctx, &s_hour_glyphs[1], s_digit_rects[1], alignments[1]);
  draw_cached_digit(ctx, &s_min_glyphs[0], s_digit_rects[2], alignments[2]);
  draw_cached_digit(ctx, &s_min_glyphs[1], s_digit_rects[3], alignments[3]);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // If a forced time is set in src/c/forced_time.c, ignore system ticks
  if (forced_time_is_set()) return;
  s_last_time = *tick_time;
  layer_mark_dirty(s_custom_layer);
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);
  
  // Use layer_get_unobstructed_bounds() instead of layer_get_bounds()
  // This handles Timeline Quick View and other system overlays automatically
  GRect bounds = layer_get_unobstructed_bounds(root);
  s_screen_width = bounds.size.w;
  s_screen_height = bounds.size.h;

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  if (t) s_last_time = *t;

  // If a forced time is provided in src/c/forced_time.c, override the real time
  if (forced_time_is_set()) {
    s_last_time = forced_time;
  }

  // Calculate base layout dimensions
  const int16_t needed = BASE_GLYPH_SIZE * 2 + BASE_GLYPH_SPACING;
  s_square_size = MIN2(s_screen_width, s_screen_height);

  #ifdef PBL_ROUND
  // Use grect_center_point() helper for accurate centering
  // Pre-compute circle constraints for round displays
  // Maximum inscribed square in circle: side = diameter / sqrt(2)
  // With padding: inner_radius = (screen_half - padding)
  int16_t screen_half = s_square_size / 2;
  float R_inner = (float)screen_half - (float)DIGIT_EDGE_PADDING_H;
  if (R_inner < 4.0f) R_inner = 4.0f;
  
  // Maximum square that fits in circle: side = R_inner * sqrt(2)
  // Using 1.41421356f as sqrt(2) approximation
  float max_square_f = R_inner * 1.41421356f;
  int16_t max_square = (int16_t)max_square_f;
  if (max_square < 8) max_square = 8;
  if (max_square % 2 != 0) max_square--;  // Keep even
  
  // Use smaller of available space or circle constraint
  s_square_size = MIN2(s_square_size, max_square);
  #endif

  // Calculate stroke width (center divider)
  const float scale = (float)s_square_size / (float)needed;
  s_stroke_width = (int16_t)(BASE_GLYPH_SPACING * scale);
  if (s_square_size % 2 == 0 && s_stroke_width % 2 != 0) s_stroke_width++;

  // Calculate initial digit size
  int16_t candidate = (s_square_size - s_stroke_width) / 2;
  if (candidate < 4) candidate = 4;
  if (candidate % 2 != 0) candidate--;

  #ifdef PBL_ROUND
  // Additional constraint for round: ensure corners of digit squares stay in circle
  // Each digit square corner is at distance sqrt(2) * (digit_half + stroke_half) from center
  // Require: sqrt(2) * (digit_size/2 + stroke_width/2) <= R_inner
  int16_t half_stroke = s_stroke_width / 2;
  float max_half_diagonal = R_inner / 1.41421356f;  // R_inner / sqrt(2)
  int16_t max_digit_from_circle = (int16_t)(max_half_diagonal - (float)half_stroke) * 2;
  if (max_digit_from_circle < 4) max_digit_from_circle = 4;
  if (max_digit_from_circle % 2 != 0) max_digit_from_circle--;
  
  s_digit_size = MIN2(candidate, max_digit_from_circle);
  #else
  s_digit_size = candidate;
  #endif

  // Use bounds for positioning (already accounts for unobstructed area)
  int16_t hs = s_stroke_width / 2;
  int16_t cx = s_screen_width / 2;
  int16_t cy = s_screen_height / 2;
  
  s_digit_rects[0] = GRect(cx - s_digit_size - hs, cy - s_digit_size - hs, s_digit_size, s_digit_size);
  s_digit_rects[1] = GRect(cx + hs, cy - s_digit_size - hs, s_digit_size, s_digit_size);
  s_digit_rects[2] = GRect(cx - s_digit_size - hs, cy + hs, s_digit_size, s_digit_size);
  s_digit_rects[3] = GRect(cx + hs, cy + hs, s_digit_size, s_digit_size);

  // Update cached glyph height to match actual digit size
  s_cached_glyph_target_height = s_digit_size;

  // Create layer with full window bounds (not unobstructed)
  // This ensures we fill the entire window behind any overlays
  GRect full_bounds = layer_get_bounds(root);
  s_custom_layer = layer_create(full_bounds);
  layer_set_update_proc(s_custom_layer, custom_layer_update_proc);
  layer_add_child(root, s_custom_layer);
}

// Handler for when unobstructed area changes (Timeline Quick View)
static void unobstructed_will_change(GRect final_unobstructed_screen_area, void *context) {
  // Recalculate layout when obstruction is about to appear/disappear
  s_screen_width = final_unobstructed_screen_area.size.w;
  s_screen_height = final_unobstructed_screen_area.size.h;
  
  // Force recalculation of layout on next draw
  s_cached_glyph_target_height = -1;
  
  // Mark layer dirty to trigger redraw with new layout
  if (s_custom_layer) {
    layer_mark_dirty(s_custom_layer);
  }
}

static void unobstructed_did_change(void *context) {
  // Layout has been updated, ensure one final redraw
  if (s_custom_layer) {
    layer_mark_dirty(s_custom_layer);
  }
}

static void window_unload(Window *window) {
  layer_destroy(s_custom_layer);
}

static void init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){.load = window_load, .unload = window_unload});
  window_stack_push(s_window, true);
  
  // Subscribe to unobstructed area changes (Timeline Quick View support)
  UnobstructedAreaHandlers handlers = {
    .will_change = unobstructed_will_change,
    .did_change = unobstructed_did_change
  };
  unobstructed_area_service_subscribe(handlers, NULL);
  
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit(void) {
  unobstructed_area_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
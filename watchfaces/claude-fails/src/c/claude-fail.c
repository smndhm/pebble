#include <pebble.h>

// PDC digit dimensions — must match gen_pdc.py PLATFORMS values
#ifdef PBL_PLATFORM_EMERY
  #define CAP_H   95
  #define DIGIT_W 95
  #define DIGIT_RES(n) RESOURCE_ID_DIGIT_EMERY_##n
#elif defined(PBL_ROUND)
  #define CAP_H   82
  #define DIGIT_W 82
  #define DIGIT_RES(n) RESOURCE_ID_DIGIT_CHALK_##n
#else
  #define CAP_H   68
  #define DIGIT_W 68
  #define DIGIT_RES(n) RESOURCE_ID_DIGIT_STD_##n
#endif

static const uint32_t kDigitRes[10] = {
    DIGIT_RES(0), DIGIT_RES(1), DIGIT_RES(2), DIGIT_RES(3), DIGIT_RES(4),
    DIGIT_RES(5), DIGIT_RES(6), DIGIT_RES(7), DIGIT_RES(8), DIGIT_RES(9),
};

#ifdef PBL_COLOR
static const GColor WC_PALETTE[4] = {
    {.argb = 0b11110000},  // coral red
    {.argb = 0b11000011},  // royal blue
    {.argb = 0b11111100},  // yellow
    {.argb = 0b11001010},  // teal
};
#endif

static Window              *s_window;
static Layer               *s_layer;
static struct tm            s_time;
static GDrawCommandImage   *s_digits[10];
static GRect                s_digit_rects[4]; // [H1, H2, M1, M2]
static int                  s_digit_px; // actual rendered size (may differ from DIGIT_W)

static void scale_digit_points(int target) {
    for (int i = 0; i < 10; i++) {
        GDrawCommandList *cmds = gdraw_command_image_get_command_list(s_digits[i]);
        uint32_t nc = gdraw_command_list_get_num_commands(cmds);
        for (uint32_t j = 0; j < nc; j++) {
            GDrawCommand *cmd = gdraw_command_list_get_command(cmds, j);
            uint16_t np = gdraw_command_get_num_points(cmd);
            for (uint16_t k = 0; k < np; k++) {
                GPoint p = gdraw_command_get_point(cmd, k);
                p.x = (int16_t)(p.x * target / DIGIT_W);
                p.y = (int16_t)(p.y * target / CAP_H);
                gdraw_command_set_point(cmd, k, p);
            }
        }
        gdraw_command_image_set_bounds_size(s_digits[i], GSize(target, target));
    }
}

static void recalculate_layout(GRect bounds) {
    int sw = bounds.size.w;
    int sh = bounds.size.h;
    int dw, gap_h, gap_v;
#ifdef PBL_ROUND
    // Scale grid so its corners fit inside the circular screen.
    // Inscribed-square side = sw * sqrt(2)/2 ≈ sw * 7071/10000
    int side = ((sw * 7071) / 10000) & ~1;
    gap_h = gap_v = side / 20;
    dw = (side - gap_h) / 2;
#else
    gap_h = sw / 20; gap_v = sh / 24;
    // Ensure at least 4px margin on each side (use the tighter axis)
    int a = (sw - 8 - gap_h) / 2, b = (sh - 8 - gap_v) / 2;
    int max_d = (a < b) ? a : b;
    dw = (DIGIT_W < max_d) ? DIGIT_W : max_d;
#endif
    s_digit_px = dw;
    int total_w = 2 * dw + gap_h;
    int total_h = 2 * dw + gap_v;
    int x0 = (sw - total_w) / 2;
    int x1 = x0 + dw + gap_h;
    int y0 = (sh - total_h) / 2;
    int y1 = y0 + dw + gap_v;
    s_digit_rects[0] = GRect(x0, y0, dw, dw);
    s_digit_rects[1] = GRect(x1, y0, dw, dw);
    s_digit_rects[2] = GRect(x0, y1, dw, dw);
    s_digit_rects[3] = GRect(x1, y1, dw, dw);
}

static void draw_digit(GContext *ctx, int digit, GRect r, GColor fg) {
    GDrawCommandImage *img = s_digits[digit];
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

    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    int h = s_time.tm_hour;
    if (!clock_is_24h_style()) {
        h = h % 12;
        if (h == 0) h = 12;
    }
    int digits[4] = {
        h              / 10,
        h              % 10,
        s_time.tm_min  / 10,
        s_time.tm_min  % 10,
    };

    for (int i = 0; i < 4; i++) {
#ifdef PBL_COLOR
        GColor fg = WC_PALETTE[i];
#else
        GColor fg = GColorWhite;
#endif
        draw_digit(ctx, digits[i], s_digit_rects[i], fg);
    }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    s_time = *tick_time;
    layer_mark_dirty(s_layer);
}

static void window_load(Window *window) {
    for (int i = 0; i < 10; i++) {
        s_digits[i] = gdraw_command_image_create_with_resource(kDigitRes[i]);
    }

    Layer *root = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(root);
    recalculate_layout(bounds);
    if (s_digit_px != DIGIT_W) {
        scale_digit_points(s_digit_px);
    }

    s_layer = layer_create(bounds);
    layer_set_update_proc(s_layer, layer_update_proc);
    layer_add_child(root, s_layer);

    time_t now = time(NULL);
    s_time = *localtime(&now);
}

static void window_unload(Window *window) {
    for (int i = 0; i < 10; i++) {
        gdraw_command_image_destroy(s_digits[i]);
        s_digits[i] = NULL;
    }
    layer_destroy(s_layer);
}

static void init(void) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){
        .load   = window_load,
        .unload = window_unload,
    });
    window_stack_push(s_window, true);

    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
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

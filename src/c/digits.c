#include "digits.h"

// All glyph data stored in ROM (const) to save precious RAM
const Point zero_regular[ZERO_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{14, 44}, {10, 44}, {10, 0}, {14, 0}},
    {{2, 44}, {2, 0}, {6, 0}, {6, 44}},
    {{2, 44}, {2, 40}, {14, 40}, {14, 44}},
    {{2, 0}, {14, 0}, {14, 4}, {2, 4}}
};

const Point zero_bold[ZERO_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{46, 44}, {42, 44}, {42, 0}, {46, 0}},
    {{2, 44}, {2, 0}, {38, 0}, {38, 44}},
    {{2, 44}, {2, 32}, {46, 32}, {46, 44}},
    {{2, 0}, {46, 0}, {46, 12}, {2, 12}}
};

const Point one_regular[ONE_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{10, 44}, {6, 44}, {6, 0}, {10, 0}},
    {{10, 44}, {2, 44}, {2, 40}, {10, 40}}
};

const Point one_bold[ONE_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{46, 44}, {6, 44}, {6, 0}, {46, 0}},
    {{46, 44}, {2, 44}, {2, 32}, {46, 32}}
};

const Point two_regular[TWO_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{14, 44}, {10, 44}, {10, 8}, {14, 8}},
    {{14, 44}, {2, 44}, {2, 40}, {14, 40}},
    {{14, 8}, {14, 12}, {2, 12}, {2, 8}},
    {{2, 12}, {2, 0}, {6, 0}, {6, 12}},
    {{2, 0}, {14, 0}, {14, 4}, {2, 4}},
    {{2, 44}, {2, 16}, {6, 16}, {6, 44}}
};

const Point two_bold[TWO_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{46, 44}, {34, 44}, {34, 8}, {46, 8}},
    {{46, 44}, {2, 44}, {2, 40}, {46, 40}},
    {{46, 8}, {46, 12}, {2, 12}, {2, 8}},
    {{2, 12}, {2, 0}, {30, 0}, {30, 12}},
    {{2, 0}, {46, 0}, {46, 4}, {2, 4}},
    {{2, 44}, {2, 16}, {30, 16}, {30, 44}}
};

const Point three_regular[THREE_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{14, 44}, {10, 44}, {10, 0}, {14, 0}},
    {{14, 44}, {2, 44}, {2, 40}, {14, 40}},
    {{14, 0}, {14, 4}, {2, 4}, {2, 0}},
    {{14, 20}, {2, 20}, {2, 16}, {14, 16}},
    {{2, 0}, {6, 0}, {6, 12}, {2, 12}},
    {{2, 44}, {2, 24}, {6, 24}, {6, 44}}
};

const Point three_bold[THREE_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{46, 44}, {34, 44}, {34, 0}, {46, 0}},
    {{46, 44}, {2, 44}, {2, 40}, {46, 40}},
    {{46, 0}, {46, 4}, {2, 4}, {2, 0}},
    {{46, 20}, {2, 20}, {2, 16}, {46, 16}},
    {{2, 0}, {30, 0}, {30, 12}, {2, 12}},
    {{2, 44}, {2, 24}, {30, 24}, {30, 44}}
};

const Point four_regular[FOUR_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{14, 44}, {10, 44}, {10, 0}, {14, 0}},
    {{14, 8}, {14, 12}, {2, 12}, {2, 8}},
    {{2, 8}, {6, 8}, {6, 44}, {2, 44}}
};

const Point four_bold[FOUR_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{46, 44}, {34, 44}, {34, 0}, {46, 0}},
    {{46, 8}, {46, 12}, {2, 12}, {2, 8}},
    {{2, 8}, {30, 8}, {30, 44}, {2, 44}}
};

const Point five_regular[FIVE_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{14, 44}, {2, 44}, {2, 40}, {14, 40}},
    {{2, 44}, {2, 32}, {6, 32}, {6, 44}},
    {{2, 32}, {14, 32}, {14, 36}, {2, 36}},
    {{14, 36}, {10, 36}, {10, 0}, {14, 0}},
    {{14, 0}, {14, 4}, {2, 4}, {2, 0}},
    {{2, 0}, {6, 0}, {6, 28}, {2, 28}}
};

const Point five_bold[FIVE_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{46, 44}, {2, 44}, {2, 40}, {46, 40}},
    {{2, 44}, {2, 32}, {30, 32}, {30, 44}},
    {{2, 32}, {46, 32}, {46, 36}, {2, 36}},
    {{46, 36}, {34, 36}, {34, 0}, {46, 0}},
    {{46, 0}, {46, 4}, {2, 4}, {2, 0}},
    {{2, 0}, {30, 0}, {30, 28}, {2, 28}}
};

const Point six_regular[SIX_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{14, 44}, {2, 44}, {2, 40}, {14, 40}},
    {{2, 32}, {14, 32}, {14, 36}, {2, 36}},
    {{14, 36}, {10, 36}, {10, 0}, {14, 0}},
    {{14, 0}, {14, 4}, {2, 4}, {2, 0}},
    {{2, 0}, {6, 0}, {6, 44}, {2, 44}}
};

const Point six_bold[SIX_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{46, 44}, {2, 44}, {2, 40}, {46, 40}},
    {{2, 32}, {46, 32}, {46, 36}, {2, 36}},
    {{46, 36}, {34, 36}, {34, 0}, {46, 0}},
    {{46, 0}, {46, 4}, {2, 4}, {2, 0}},
    {{2, 0}, {30, 0}, {30, 44}, {2, 44}}
};

const Point seven_regular[SEVEN_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{14, 44}, {10, 44}, {10, 0}, {14, 0}},
    {{14, 44}, {2, 44}, {2, 40}, {14, 40}},
    {{14, 12}, {2, 12}, {2, 8}, {14, 8}},
    {{6, 44}, {2, 44}, {2, 16}, {6, 16}}
};

const Point seven_bold[SEVEN_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{46, 44}, {34, 44}, {34, 0}, {46, 0}},
    {{46, 44}, {2, 44}, {2, 40}, {46, 40}},
    {{46, 12}, {2, 12}, {2, 8}, {46, 8}},
    {{30, 44}, {2, 44}, {2, 16}, {30, 16}}
};

const Point eight_regular[EIGHT_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{14, 44}, {10, 44}, {10, 0}, {14, 0}},
    {{6, 44}, {2, 44}, {2, 0}, {6, 0}},
    {{2, 44}, {2, 40}, {14, 40}, {14, 44}},
    {{2, 4}, {2, 0}, {14, 0}, {14, 4}},
    {{2, 20}, {2, 16}, {14, 16}, {14, 20}}
};

const Point eight_bold[EIGHT_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{46, 44}, {34, 44}, {34, 0}, {46, 0}},
    {{30, 44}, {2, 44}, {2, 0}, {30, 0}},
    {{2, 44}, {2, 40}, {46, 40}, {46, 44}},
    {{2, 4}, {2, 0}, {46, 0}, {46, 4}},
    {{2, 20}, {2, 16}, {46, 16}, {46, 20}}
};

const Point nine_regular[NINE_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{2, 0}, {14, 0}, {14, 4}, {2, 4}},
    {{14, 12}, {2, 12}, {2, 8}, {14, 8}},
    {{2, 8}, {6, 8}, {6, 44}, {2, 44}},
    {{2, 44}, {2, 40}, {14, 40}, {14, 44}},
    {{14, 44}, {10, 44}, {10, 0}, {14, 0}}
};

const Point nine_bold[NINE_NUM_CONTOURS][POINTS_PER_CONTOUR] = {
    {{2, 0}, {46, 0}, {46, 4}, {2, 4}},
    {{46, 12}, {2, 12}, {2, 8}, {46, 8}},
    {{2, 8}, {30, 8}, {30, 44}, {2, 44}},
    {{2, 44}, {2, 40}, {46, 40}, {46, 44}},
    {{46, 44}, {34, 44}, {34, 0}, {46, 0}}
};

// Core glyph functions implementation
void interpolate_glyph(const Point regular[][POINTS_PER_CONTOUR], 
                       const Point bold[][POINTS_PER_CONTOUR], 
                       Point out[][POINTS_PER_CONTOUR], 
                       int num_contours, float percent, 
                       int glyph_height, bool invert_y)
{
  // Find bounding box of both variants
  int16_t orig_min_x = regular[0][0].x;
  int16_t orig_max_x = regular[0][0].x;
  int16_t orig_min_y = regular[0][0].y;
  int16_t orig_max_y = regular[0][0].y;
  
  for (int c = 0; c < num_contours; c++) {
    for (int p = 0; p < POINTS_PER_CONTOUR; p++) {
      int16_t rx = regular[c][p].x, ry = regular[c][p].y;
      int16_t bx = bold[c][p].x, by = bold[c][p].y;
      if (rx < orig_min_x) orig_min_x = rx;
      if (bx < orig_min_x) orig_min_x = bx;
      if (rx > orig_max_x) orig_max_x = rx;
      if (bx > orig_max_x) orig_max_x = bx;
      if (ry < orig_min_y) orig_min_y = ry;
      if (by < orig_min_y) orig_min_y = by;
      if (ry > orig_max_y) orig_max_y = ry;
      if (by > orig_max_y) orig_max_y = by;
    }
  }
  
  int16_t orig_height = orig_max_y - orig_min_y + 1;
  if (orig_height <= 0) orig_height = 1;
  float scale = (float)glyph_height / (float)orig_height;

  // Interpolate and scale
  for (int c = 0; c < num_contours; c++) {
    for (int p = 0; p < POINTS_PER_CONTOUR; p++) {
      float interp_x = regular[c][p].x + (bold[c][p].x - regular[c][p].x) * percent;
      float interp_y = regular[c][p].y + (bold[c][p].y - regular[c][p].y) * percent;
      int16_t scaled_x = (int16_t)((interp_x - orig_min_x) * scale + 0.5f);
      int16_t scaled_y = (int16_t)((interp_y - orig_min_y) * scale + 0.5f);
      out[c][p].x = scaled_x;
      out[c][p].y = invert_y ? (glyph_height - scaled_y) : scaled_y;
    }
  }
}

void draw_glyph(GContext *ctx, const Point glyph[][POINTS_PER_CONTOUR], int num_contours)
{
  for (int c = 0; c < num_contours; c++) {
    int16_t min_x = glyph[c][0].x, max_x = glyph[c][0].x;
    int16_t min_y = glyph[c][0].y, max_y = glyph[c][0].y;
    
    for (int p = 1; p < POINTS_PER_CONTOUR; p++) {
      if (glyph[c][p].x < min_x) min_x = glyph[c][p].x;
      if (glyph[c][p].x > max_x) max_x = glyph[c][p].x;
      if (glyph[c][p].y < min_y) min_y = glyph[c][p].y;
      if (glyph[c][p].y > max_y) max_y = glyph[c][p].y;
    }
    
    graphics_fill_rect(ctx, GRect(min_x, min_y, max_x - min_x + 1, max_y - min_y + 1), 
                       0, GCornerNone);
  }
}

void glyph_bounding_box(const Point glyph[][POINTS_PER_CONTOUR], int num_contours, 
                        int16_t *min_x, int16_t *min_y, int16_t *max_x, int16_t *max_y)
{
  *min_x = *max_x = glyph[0][0].x;
  *min_y = *max_y = glyph[0][0].y;
  
  for (int c = 0; c < num_contours; c++) {
    for (int p = 0; p < POINTS_PER_CONTOUR; p++) {
      int16_t x = glyph[c][p].x, y = glyph[c][p].y;
      if (x < *min_x) *min_x = x;
      if (x > *max_x) *max_x = x;
      if (y < *min_y) *min_y = y;
      if (y > *max_y) *max_y = y;
    }
  }
}
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

void interpolate_glyph(const Point regular[][POINTS_PER_CONTOUR],
                       const Point bold[][POINTS_PER_CONTOUR],
                       Point out[][POINTS_PER_CONTOUR],
                       int num_contours, int32_t percent_fp,
                       int glyph_height, bool invert_y,
                       int16_t *out_min_x, int16_t *out_min_y,
                       int16_t *out_max_x, int16_t *out_max_y)
{
  // Precomputed scale: glyph_height / GLYPH_BBOX_HEIGHT in fixed-point
  int32_t scale_fp = ((int32_t)glyph_height << FP_SHIFT) / GLYPH_BBOX_HEIGHT;

  // Track output bounding box
  int16_t bmin_x = INT16_MAX, bmin_y = INT16_MAX;
  int16_t bmax_x = INT16_MIN, bmax_y = INT16_MIN;

  // Single pass: interpolate, scale, and compute bounding box
  for (int c = 0; c < num_contours; c++) {
    for (int p = 0; p < POINTS_PER_CONTOUR; p++) {
      // Fixed-point interpolation: reg + (bold - reg) * percent
      int32_t interp_x = ((int32_t)regular[c][p].x << FP_SHIFT)
                        + (bold[c][p].x - regular[c][p].x) * percent_fp;
      int32_t interp_y = ((int32_t)regular[c][p].y << FP_SHIFT)
                        + (bold[c][p].y - regular[c][p].y) * percent_fp;

      // Subtract bbox origin, scale, and round (all in fixed-point)
      int16_t sx = (int16_t)(((interp_x - (GLYPH_BBOX_MIN_X << FP_SHIFT)) * scale_fp
                              + (1 << (2 * FP_SHIFT - 1))) >> (2 * FP_SHIFT));
      int16_t sy = (int16_t)(((interp_y - (GLYPH_BBOX_MIN_Y << FP_SHIFT)) * scale_fp
                              + (1 << (2 * FP_SHIFT - 1))) >> (2 * FP_SHIFT));

      out[c][p].x = sx;
      out[c][p].y = invert_y ? (glyph_height - sy) : sy;

      // Update bounding box
      if (out[c][p].x < bmin_x) bmin_x = out[c][p].x;
      if (out[c][p].x > bmax_x) bmax_x = out[c][p].x;
      if (out[c][p].y < bmin_y) bmin_y = out[c][p].y;
      if (out[c][p].y > bmax_y) bmax_y = out[c][p].y;
    }
  }

  *out_min_x = bmin_x;
  *out_min_y = bmin_y;
  *out_max_x = bmax_x;
  *out_max_y = bmax_y;
}

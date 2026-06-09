#pragma once
#include <pebble.h>

typedef struct {
    int16_t x;
    int16_t y;
} Point;

// All contours are axis-aligned rectangles defined by 4 corner points
#define POINTS_PER_CONTOUR 4
#define MAX_CONTOURS 6

// Fixed-point arithmetic (8 fractional bits)
#define FP_SHIFT 8
#define FP_ONE (1 << FP_SHIFT)

// Precomputed union bounding box of all regular+bold glyph variants
#define GLYPH_BBOX_MIN_X 2
#define GLYPH_BBOX_MIN_Y 0
#define GLYPH_BBOX_HEIGHT 45  // (44 - 0 + 1)

// Number of contours per digit
#define ZERO_NUM_CONTOURS  4
#define ONE_NUM_CONTOURS   2
#define TWO_NUM_CONTOURS   6
#define THREE_NUM_CONTOURS 6
#define FOUR_NUM_CONTOURS  3
#define FIVE_NUM_CONTOURS  6
#define SIX_NUM_CONTOURS   5
#define SEVEN_NUM_CONTOURS 4
#define EIGHT_NUM_CONTOURS 5
#define NINE_NUM_CONTOURS  5

// Glyph data declarations
extern const Point zero_regular[ZERO_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point zero_bold[ZERO_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point one_regular[ONE_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point one_bold[ONE_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point two_regular[TWO_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point two_bold[TWO_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point three_regular[THREE_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point three_bold[THREE_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point four_regular[FOUR_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point four_bold[FOUR_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point five_regular[FIVE_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point five_bold[FIVE_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point six_regular[SIX_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point six_bold[SIX_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point seven_regular[SEVEN_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point seven_bold[SEVEN_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point eight_regular[EIGHT_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point eight_bold[EIGHT_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point nine_regular[NINE_NUM_CONTOURS][POINTS_PER_CONTOUR];
extern const Point nine_bold[NINE_NUM_CONTOURS][POINTS_PER_CONTOUR];

// Layout configuration
#define DIGIT_EDGE_PADDING_H 4
#define DIGIT_EDGE_PADDING_V 8
#define BASE_GLYPH_SIZE 44
#define BASE_GLYPH_SPACING 4

// Interpolate between regular and bold glyphs using fixed-point math.
// percent_fp: interpolation amount in 8.8 fixed-point (0 = regular, FP_ONE = bold)
// Also computes the output bounding box in a single pass.
void interpolate_glyph(const Point regular[][POINTS_PER_CONTOUR],
                       const Point bold[][POINTS_PER_CONTOUR],
                       Point out[][POINTS_PER_CONTOUR],
                       int num_contours, int32_t percent_fp,
                       int glyph_height, bool invert_y,
                       int16_t *out_min_x, int16_t *out_min_y,
                       int16_t *out_max_x, int16_t *out_max_y);

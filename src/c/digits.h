#pragma once
#include <pebble.h>

// Point structure for glyph coordinates
typedef struct {
    int16_t x;  // Use int16_t instead of int to save memory
    int16_t y;
} Point;

// All glyphs use cubic Bézier curves with 4 control points
#define POINTS_PER_CONTOUR 4
#define MAX_CONTOURS 6  // Maximum across all digits (2 and 3 have 6)

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

// Glyph data - all in ROM to save RAM
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

// Core functions
void interpolate_glyph(const Point regular[][POINTS_PER_CONTOUR], 
                       const Point bold[][POINTS_PER_CONTOUR], 
                       Point out[][POINTS_PER_CONTOUR], 
                       int num_contours, float percent, 
                       int glyph_height, bool invert_y);

void draw_glyph(GContext *ctx, const Point glyph[][POINTS_PER_CONTOUR], int num_contours);

void glyph_bounding_box(const Point glyph[][POINTS_PER_CONTOUR], int num_contours, 
                        int16_t *min_x, int16_t *min_y, int16_t *max_x, int16_t *max_y);

// Simple place to hardcode a forced time for screenshots.
// Edit the values below to choose the time displayed by the app.

#include "forced_time.h"

// Default: hour = -1 means no forced time.
struct tm forced_time = {
  .tm_sec = 0,
  .tm_min = 0,
  .tm_hour = -1,
};

bool forced_time_is_set(void) {
  return forced_time.tm_hour >= 0;
}


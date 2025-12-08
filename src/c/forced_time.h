// Auto-generated simple forced time container
#pragma once

#include <pebble.h>
#include <stdbool.h>

// If non-zero, this struct contains the forced time to display.
extern struct tm forced_time;

// Returns true if a forced time is set (hour/min non-negative).
bool forced_time_is_set(void);

#include <pebble.h>
#include "current_time.h"

void current_time_update(struct tm *tick_time) {
  current_time.hours = tick_time->tm_hour;
  current_time.hours -= (current_time.hours > 12) ? 12 : 0;
  current_time.minutes = tick_time->tm_min;
}
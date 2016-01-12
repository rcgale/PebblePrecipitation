#include <pebble.h>
#include "geometry.h"

GPoint get_point_on_clock(GPoint center, float percent_around, float length) {
  return (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * percent_around) * length / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * percent_around) * length / TRIG_MAX_RATIO) + center.y,
  };
}
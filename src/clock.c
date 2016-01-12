#include <pebble.h>
#include <clock.h>
#include <geometry.h>
#include <appearance.h>
#include <configuration.h>

typedef struct {
  int hours;
  int minutes;
} Time;
static Layer *s_canvas_layer;

static GPoint s_center;
static Time s_last_time;

static int s_radius = FINAL_RADIUS;

static void draw_hand(GContext *ctx, GPoint hand_point, GColor8 color, int stroke_width) {
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, stroke_width);
  graphics_draw_line(ctx, s_center, hand_point);
}

static void draw_hands(Layer *layer, GContext *ctx) {
  graphics_context_set_antialiased(ctx, ANTIALIASING);
  Time mode_time = s_last_time;
  
  // Plot hands
  float minute_percent = mode_time.minutes / 60.0;
  float minute_hand_length = s_radius - HAND_MARGIN;
  GPoint minute_hand = get_point_on_clock(s_center, minute_percent, minute_hand_length);
  float hour_percent = (mode_time.hours + mode_time.minutes / 60.0) / 12.0;
  float hour_hand_length = s_radius - 3 * HAND_MARGIN;
  GPoint hour_hand = get_point_on_clock(s_center, hour_percent, hour_hand_length);

  draw_hand(ctx, minute_hand, settings.is_minutely ? COLOR_ACTIVE_HAND_OUTLINE : COLOR_INACTIVE_HAND_OUTLINE, MINUTE_HAND_OUTLINE_STROKE_WIDTH);
  draw_hand(ctx, minute_hand, settings.is_minutely ? COLOR_ACTIVE_HAND : COLOR_INACTIVE_HAND, MINUTE_HAND_STROKE_WIDTH);
  draw_hand(ctx, hour_hand, settings.is_minutely ? COLOR_INACTIVE_HAND_OUTLINE : COLOR_ACTIVE_HAND_OUTLINE, HOUR_HAND_OUTLINE_STROKE_WIDTH);
  draw_hand(ctx, hour_hand, settings.is_minutely ? COLOR_INACTIVE_HAND : COLOR_ACTIVE_HAND, HOUR_HAND_STROKE_WIDTH);
}

Layer* clock_create(GRect window_bounds) {  
  s_center = grect_center_point(&window_bounds);
  s_canvas_layer = layer_create(window_bounds);
  layer_set_update_proc(s_canvas_layer, draw_hands);
  return s_canvas_layer;
}

void clock_destroy() {
  layer_destroy(s_canvas_layer);
}

void clock_update(struct tm *tick_time) {
   // Store time
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
  s_last_time.minutes = tick_time->tm_min;

  // Redraw
  if(s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  } 
}

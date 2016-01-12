#include <pebble.h>
#include <forecast.h>
#include <appearance.h>
#include <geometry.h>
#include <math.h>
#include <forecast_icons.h>
#include <configuration.h>

#define POINTS_MINUTELY 60
#define POINTS_HOURLY 12
#define UPDATE_INTERVAL 1
#define JS_KEY_IS_MINUTELY 99

static Layer *s_canvas_layer;
static GPoint s_center;
static GPath *s_my_path_ptr = NULL;
static float s_graph_rotate = -1;
static int s_num_graph_points;
static float s_probabilities [POINTS_MINUTELY + 1] = { 0 };

static GColor* colors = NULL;

static GColor get_color(float percent) {
  int rounded = (int)round(percent * 7);
  switch (rounded) {
    case 0: return COLOR_FORECAST_00;
    case 1: return COLOR_FORECAST_10;
    case 2: return COLOR_FORECAST_20;
    case 3: return COLOR_FORECAST_30;
    case 4: return COLOR_FORECAST_40;
    case 5: return COLOR_FORECAST_50;
    case 6: return COLOR_FORECAST_60;
    case 7: return COLOR_FORECAST_70;
    default: return GColorBlack;
  }
}

static void draw_wedge(GContext *ctx, float angle_1, float angle_2, GColor color) {
  GPoint point_0 = (GPoint){0, 0};
  GPoint point_1 = get_point_on_clock(point_0, angle_1, FINAL_RADIUS * 2);
  GPoint point_2 = get_point_on_clock(point_0, angle_2, FINAL_RADIUS * 2);
  GPathInfo path_info = {
    .num_points = 3,
    .points = (GPoint[]) { point_0, point_1, point_2 }
  };
  if (s_my_path_ptr) {
    gpath_destroy(s_my_path_ptr);
  }
  s_my_path_ptr = gpath_create(&path_info);
  graphics_context_set_fill_color(ctx, color);
  gpath_move_to(s_my_path_ptr, s_center);
  gpath_rotate_to(s_my_path_ptr, TRIG_MAX_ANGLE * s_graph_rotate);
  gpath_draw_filled(ctx, s_my_path_ptr);
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 2);
  gpath_draw_outline(ctx, s_my_path_ptr);
}

static void draw_icons(GContext *ctx) {
  
}

static void draw_forecast(Layer *layer, GContext *ctx) {
  GPoint graph_points[s_num_graph_points + 1];
  float angle_percent;
  float length;
  for (int i = 0; i < s_num_graph_points; i++) {
    float angle_1 = (float)i / s_num_graph_points;
    float angle_2 = (float)(i + 1) / s_num_graph_points;
    GColor color = get_color(s_probabilities[i]);
    draw_wedge(ctx, angle_1, angle_2, color);
  }
  draw_icons(ctx);
/*
  for (int i = 0; i < s_num_graph_points; i++) {
    angle_percent = (float)i / s_num_graph_points;
    length = s_probabilities[i] * FINAL_RADIUS;
    graph_points[i] = get_point_on_clock((GPoint){0, 0}, angle_percent, length);
  }
  graph_points[s_num_graph_points] = get_point_on_clock((GPoint){0, 0}, 1.0, length);
  // Fill the path:
  GPathInfo path_info = {
    .num_points = s_num_graph_points + 1,
    .points = graph_points
  };
  s_my_path_ptr = gpath_create(&path_info);
  gpath_rotate_to(s_my_path_ptr, TRIG_MAX_ANGLE * s_graph_rotate);

  gpath_move_to(s_my_path_ptr, s_center);
  graphics_context_set_fill_color(ctx, COLOR_FORECAST_FILL);
  gpath_draw_filled(ctx, s_my_path_ptr);
  graphics_context_set_stroke_color(ctx, COLOR_FORECAST_STROKE);
  graphics_context_set_stroke_width(ctx, 1);
  gpath_draw_outline(ctx, s_my_path_ptr); */
}

static void get_weather() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, JS_KEY_IS_MINUTELY, settings.is_minutely);
  app_message_outbox_send();  
}

static void clear_to_minute(int m) {
  if (m == 0) {
    return;
  }
  for (int i = 0; i < m; i++) {
    s_probabilities[m - 1] = s_probabilities[s_num_graph_points - 1];    
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *minute;
  for (int i = 0; i < s_num_graph_points; i++) {
    minute = dict_find(iterator, i);   
    s_probabilities[i] = (int)minute->value->int16 / 100.0;
  }
  if(s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void set_graph_rotate(struct tm *tick_time) {
  if (s_num_graph_points == POINTS_MINUTELY) {
    s_graph_rotate = 1.0 * (tick_time->tm_min - tick_time->tm_min % 5) / s_num_graph_points;
  }
  else {
    s_graph_rotate = 1.0 * (tick_time->tm_hour % 12 + tick_time->tm_min / 60.0) / s_num_graph_points;
  }
}

Layer* forecast_create(GRect window_bounds) {
  s_num_graph_points = settings.is_minutely ? POINTS_MINUTELY : POINTS_HOURLY;
  s_center = grect_center_point(&window_bounds);
  s_canvas_layer = layer_create(window_bounds);
  //layer_add_child(s_canvas_layer, textlayer_create());
  layer_set_update_proc(s_canvas_layer, draw_forecast);  
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(1024, 1024); // TODO: Determine the correct values.

  static Layer* icons_layer;
  icons_layer = forecast_icons_create(window_bounds);
  layer_add_child(s_canvas_layer, icons_layer);

  return s_canvas_layer;
}

void forecast_destroy() {
  forecast_icons_destroy();
  layer_destroy(s_canvas_layer);
}

void forecast_update(struct tm *tick_time) {
  //forecast_icons_update(tick_time);
  if(s_graph_rotate < 0 || tick_time->tm_min % UPDATE_INTERVAL == 0) {
    get_weather();
  }
  else {
    clear_to_minute(tick_time->tm_min % 5);
  }
  set_graph_rotate(tick_time);
  // Redraw
  if(s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}
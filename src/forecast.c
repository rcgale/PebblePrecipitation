#include <pebble.h>
#include <forecast.h>
#include <appearance.h>
#include <geometry.h>
#include <math.h>
#include <forecast_icons.h>
#include <configuration.h>
#include <current_time.h>

#define NUM_WEDGES 60
#define POINTS_MINUTELY 60
#define POINTS_HOURLY 12
#define UPDATE_INTERVAL 1
#define JS_KEY_IS_MINUTELY 99
#define JS_KEY_API_KEY 98

static bool needs_refresh = true;
static Layer* s_canvas_layer;
static GPoint s_center;
static GPoint s_wedges[NUM_WEDGES][3];
static GColor s_minutely [POINTS_MINUTELY + 1];
static GContext *s_context;

static GColor* colors = NULL;

static void print_gpath(GPath* path) {
  // debugsies
    APP_LOG(APP_LOG_LEVEL_INFO, "(%d, %d), (%d, %d), (%d, %d)",
          path->points[0].x, path->points[0].y,
          path->points[1].x, path->points[1].y,
          path->points[2].x, path->points[2].y);
    //APP_LOG(APP_LOG_LEVEL_INFO, "rotation: %f, offset (%d, %d)", s_wedges[i]->rotation, s_wedges[i]->offset.x, s_wedges[i]->offset.y),  
}

static GPath* get_wedge(int i) {
  GPathInfo path_info = (GPathInfo){
    .num_points = 3,
    .points = s_wedges[i]
  };
  GPath* path = gpath_create(&path_info);
  gpath_move_to(path, s_center);
  return path;
}

static void fill_wedge(GContext *ctx, GPath *wedge, GColor color) {
  graphics_context_set_fill_color(ctx, color);
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 2);
  gpath_draw_filled(ctx, wedge);  
  gpath_draw_outline(ctx, wedge);
}

static GPath *s_my_path_ptr = NULL;
static void draw_wedge(GContext *ctx, int i, GColor color) {
  if (s_my_path_ptr) {
    gpath_destroy(s_my_path_ptr);
  }
  s_my_path_ptr = get_wedge(i);
  //print_gpath(s_my_path_ptr);
  fill_wedge(ctx, s_my_path_ptr, color);
}

static void init_wedges() {
  for (int i = 0; i < NUM_WEDGES; i++) {
    float angle_1 = 1.0 * i / NUM_WEDGES;
    float angle_2 = 1.0 * (i + 1) / NUM_WEDGES;
    s_wedges[i][0] = (GPoint) { 0, 0 };
    s_wedges[i][1] = get_point_on_clock(s_wedges[i][0], angle_1, FACE_RADIUS);
    s_wedges[i][2] = get_point_on_clock(s_wedges[i][0], angle_2, FACE_RADIUS);
  }
}

static void draw_icons(GContext *ctx) {
  
}

static void draw_forecast(Layer *layer, GContext *ctx) {
  s_context = ctx;
  for (int i = 0; i < NUM_WEDGES; i++) {
    //print_gpath(s_wedges[i]);
    GColor color = s_minutely[i];
    //APP_LOG(APP_LOG_LEVEL_INFO, "data_index: %d", i);
    //APP_LOG(APP_LOG_LEVEL_INFO, "p: %d", (int)round(s_probabilities[i] * 100.0));
    draw_wedge(ctx, i, color);
  }
  draw_icons(ctx);
}

static void get_weather() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, JS_KEY_IS_MINUTELY, settings.is_minutely);
  dict_write_cstring(iter, JS_KEY_API_KEY, settings.api_key);
  app_message_outbox_send();  
}

void forecast_process_callback(DictionaryIterator *iterator, void *context) {
  Tuple *minute;
  for (int i = 0; i < NUM_WEDGES; i++) {
    minute = dict_find(iterator, i);   
    //APP_LOG(APP_LOG_LEVEL_INFO, "value at %d: %d", i, (int)minute->value->int32);
    s_minutely[i] = GColorFromHEX((int)minute->value->int32);
    if (s_context) {
      draw_wedge(s_context, i, s_minutely[i]);      
    }
  }
  if (s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);    
  }
}

Layer* forecast_create(GRect window_bounds) {
  s_center = grect_center_point(&window_bounds);
  init_wedges(s_center);
  s_canvas_layer = layer_create(window_bounds);
  //layer_add_child(s_canvas_layer, textlayer_create());
  layer_set_update_proc(s_canvas_layer, draw_forecast);  

  static Layer* icons_layer;
  icons_layer = forecast_icons_create(window_bounds);
  layer_add_child(s_canvas_layer, icons_layer);

  return s_canvas_layer;
}

void forecast_destroy() {
  forecast_icons_destroy();
  layer_destroy(s_canvas_layer);
}

void forecast_update() {
  if(needs_refresh || current_time.minutes % UPDATE_INTERVAL == 0) {
    needs_refresh = false;
    get_weather();
  }
}

void forecast_queue_refresh() {
  needs_refresh = true;
  forecast_update();
}
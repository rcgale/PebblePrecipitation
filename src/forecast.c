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
static GPoint s_points[NUM_WEDGES][3];
static GPath* s_paths[NUM_WEDGES];
static GColor s_minutely [POINTS_MINUTELY + 1];
static GContext *s_context;

static GColor* colors;

static GPath* get_wedge(int i) {
  if (!s_paths[i]) {
    GPathInfo path_info = (GPathInfo){
      .num_points = 3,
      .points = s_points[i]
    };
    s_paths[i] = gpath_create(&path_info);
    gpath_move_to(s_paths[i], s_center);    
  }
  return s_paths[i];
}

static void fill_wedge(GContext *ctx, GPath *wedge, GColor color) {
  graphics_context_set_fill_color(ctx, color);
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 2);
  gpath_draw_filled(ctx, wedge);  
  gpath_draw_outline(ctx, wedge);
}

static void init_points() {
  for (int i = 0; i < NUM_WEDGES; i++) {
    float angle_1 = 1.0 * i / NUM_WEDGES;
    float angle_2 = 1.0 * (i + 1) / NUM_WEDGES;
    s_points[i][0] = (GPoint) { 0, 0 };
    s_points[i][1] = get_point_on_clock(s_points[i][0], angle_1, FACE_RADIUS);
    s_points[i][2] = get_point_on_clock(s_points[i][0], angle_2, FACE_RADIUS);
  }
}

static void draw_icons(GContext *ctx) {
  
}

static void draw_forecast(Layer *layer, GContext *ctx) {
  s_context = ctx;
  for (int i = 0; i < NUM_WEDGES; i++) {
    GColor color = s_minutely[i];
    fill_wedge(ctx, get_wedge(i), color);
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
      fill_wedge(s_context, get_wedge(i), s_minutely[i]);      
    }
  }
  if (s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);    
  }
}

Layer* forecast_create(GRect window_bounds) {
  s_center = grect_center_point(&window_bounds);
  init_points(s_center);
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
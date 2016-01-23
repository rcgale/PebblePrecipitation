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
static struct GPath* s_wedges[NUM_WEDGES];
static float s_probabilities [POINTS_MINUTELY + 1] = { 0 };
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


static GColor get_color(float percent) {
  if (percent == 0) {
    return COLOR_FORECAST_00;
  }
  int rounded = (int)round(percent * 7);
  switch (rounded) {
    case 0: return COLOR_FORECAST_10;
    case 1: return COLOR_FORECAST_20;
    case 2: return COLOR_FORECAST_30;
    case 3: return COLOR_FORECAST_40;
    case 4: return COLOR_FORECAST_50;
    case 5: return COLOR_FORECAST_60;
    case 6: return COLOR_FORECAST_70;
    default: return GColorBlack;
  }
}

static GPath* get_wedge(float angle_1, float angle_2) {
  GPoint point_0 = (GPoint) { 0, 0 };
  GPoint point_1 = get_point_on_clock(point_0, angle_1, FINAL_RADIUS * 2);
  GPoint point_2 = get_point_on_clock(point_0, angle_2, FINAL_RADIUS * 2);
  GPathInfo path_info = (GPathInfo){
    .num_points = 3,
    .points = (GPoint[]) { point_0, point_1, point_2 }
  };
  return gpath_create(&path_info);  
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
    float angle_1 = 1.0 * i / NUM_WEDGES;
    float angle_2 = 1.0 * (i + 1) / NUM_WEDGES;
    s_my_path_ptr = get_wedge(angle_1, angle_2);
    gpath_move_to(s_my_path_ptr, s_center);
  fill_wedge(ctx, s_my_path_ptr, color);
}


static void init_wedges(GPoint center) {
  //return;
  float angle_1, angle_2;
  for (int i = 0; i < NUM_WEDGES; i++) {
    angle_1 = 1.0 * i / NUM_WEDGES;
    angle_2 = 1.0 * (i + 1) / NUM_WEDGES;
    s_wedges[i] = get_wedge(angle_1, angle_2);
    //gpath_move_to(s_wedges[i], s_center);
    print_gpath(s_wedges[i]);
  } 
}

static void destroy_wedges() {
  for (int i = 0; i < NUM_WEDGES; i++) {
    gpath_destroy(s_wedges[i]);
  }
}

static void draw_icons(GContext *ctx) {
  
}

static void draw_forecast(Layer *layer, GContext *ctx) {
  s_context = ctx;
  //init_wedges(s_center);
  for (int i = 0; i < NUM_WEDGES; i++) {
    //print_gpath(s_wedges[i]);
    GColor color = get_color(s_probabilities[i]);
    //APP_LOG(APP_LOG_LEVEL_INFO, "data_index: %d", i);
    //APP_LOG(APP_LOG_LEVEL_INFO, "p: %d", (int)round(s_probabilities[i] * 100.0));
    //fill_wedge(ctx, s_wedges[i], color);
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
    APP_LOG(APP_LOG_LEVEL_INFO, "value at %d: %d", i, (int)minute->value->int16);
    s_probabilities[i] = (int)minute->value->int16 / 100.0;
    if (s_context) {
      GColor color = get_color(s_probabilities[i]);
      draw_wedge(s_context, i, color);      
    }
  }
}

Layer* forecast_create(GRect window_bounds) {
  s_center = grect_center_point(&window_bounds);
  //init_wedges(s_center);
  s_canvas_layer = layer_create(window_bounds);
  //layer_add_child(s_canvas_layer, textlayer_create());
  layer_set_update_proc(s_canvas_layer, draw_forecast);  

  static Layer* icons_layer;
  icons_layer = forecast_icons_create(window_bounds);
  layer_add_child(s_canvas_layer, icons_layer);

  return s_canvas_layer;
}

void forecast_destroy() {
  destroy_wedges();
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
#include <pebble.h>
#include <clock.h>
#include <forecast.h>
#include <configuration.h>
#include <current_time.h>
#include <callback.h>

#define ANIMATION_DURATION 500
#define ANIMATION_DELAY    600

static Window *s_main_window;

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  current_time_update(tick_time);
  forecast_update();
  clock_update(); 
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  Layer *s_forecast_layer = forecast_create(window_bounds);
  layer_add_child(window_layer, s_forecast_layer);

  Layer *s_canvas_layer = clock_create(window_bounds);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  forecast_destroy();
  clock_destroy();
}

static void init() {
  callback_create();
  configuration_load();
  srand(time(NULL));

  time_t t = time(NULL);
  struct tm *time_now = localtime(&t);
  tick_handler(time_now, MINUTE_UNIT);

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
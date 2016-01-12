#include <pebble.h>
#include "forecast_icons.h"

#define WEATHER_ICON_NIGHT "\U0001F603"
#define WEATHER_ICON_SNOWY_FOR_LATER "A"
#define WEATHER_ICON_SNOWY ""

/*

text_layer_set_text(s_layer, "Smiley face: \U0001F603");
.icon-night:before { content: "\f100"; }
.icon-sunny:before { content: "\f101"; }
.icon-frosty:before { content: "\f102"; }
.icon-windysnow:before { content: "\f103"; }
.icon-showers:before { content: "\f104"; }
.icon-basecloud:before { content: "\f105"; }
.icon-cloud:before { content: "\f106"; }
.icon-rainy:before { content: "\f107"; }
.icon-mist:before { content: "\f108"; }
.icon-windysnowcloud:before { content: "\f109"; }
.icon-drizzle:before { content: "\f10a"; }
.icon-snowy:before { content: "\f10b"; }
.icon-sleet:before { content: "\f10c"; }
.icon-moon:before { content: "\f10d"; }
.icon-windyrain:before { content: "\f10e"; }
.icon-hail:before { content: "\f10f"; }
.icon-sunset:before { content: "\f110"; }
.icon-windyraincloud:before { content: "\f111"; }
.icon-sunrise:before { content: "\f112"; }
.icon-sun:before { content: "\f113"; }
.icon-thunder:before { content: "\f114"; }
.icon-windy:before { content: "\f115"; }
*/

static Layer *s_canvas_layer;
static GPoint s_center;
static GFont s_weather_font;

static TextLayer *s_text_layer_north;

Layer* forecast_icons_create(GRect window_bounds) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Well hello!");
  s_text_layer_north = text_layer_create(
      GRect(0, 0, window_bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_text_layer_north, GColorClear);
  text_layer_set_text_color(s_text_layer_north, GColorBlack);
  text_layer_set_text(s_text_layer_north, WEATHER_ICON_SNOWY);
  text_layer_set_text_alignment(s_text_layer_north, GTextAlignmentCenter);

  // Create GFont
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHER_32));

  // Apply to TextLayer
  text_layer_set_font(s_text_layer_north, s_weather_font);

  // Add it as a child layer to the Window's root layer
  return text_layer_get_layer(s_text_layer_north);
}

void forecast_icons_destroy() {
  text_layer_destroy(s_text_layer_north);
  layer_destroy(s_canvas_layer);
  fonts_unload_custom_font(s_weather_font);
}

void forecast_icons_update(struct tm *tick_time) {
  text_layer_set_text(s_text_layer_north, WEATHER_ICON_SNOWY);
}
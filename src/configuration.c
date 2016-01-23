#include <pebble.h>
#include <configuration.h>
#include <forecast.h>

#define KEY_API_KEY 0
#define KEY_IS_MINUTELY 1

#define DEFAULT_IS_MINUTELY true

static bool get_val_bool(int key, bool def) {
  if (persist_exists(key)) {
    return persist_read_bool(key); 
  }
  return def;
}

static char* get_api_key() {
  static char buffer[33];
  if (persist_exists(KEY_API_KEY)) {
    persist_read_string(KEY_API_KEY, buffer, sizeof(buffer));
    return buffer;
  }
  return "";
}

Settings settings;

void configuration_load() {
  settings = (Settings){
    .api_key = get_api_key(),
    .is_minutely = get_val_bool(KEY_IS_MINUTELY, DEFAULT_IS_MINUTELY)
  };
}

void configuration_set_is_minutely(bool value) {
  persist_write_bool(KEY_IS_MINUTELY, value);
  settings.is_minutely = value;
  forecast_queue_refresh();
}

void configuration_set_api_key(char* value) {
  persist_write_string(KEY_API_KEY, value);
  settings.is_minutely = value;
}

void configuration_process_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Config message received!");
  bool is_minutely = 0 == strcmp("true", dict_find(iterator, KEY_IS_MINUTELY)->value->cstring);
  configuration_set_is_minutely(is_minutely);
}

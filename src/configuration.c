#include <pebble.h>
#include "configuration.h"

#define KEY_IS_MINUTELY 0

static bool get_val_bool(int key, bool def) {
  if (persist_exists(key)) {
    return persist_read_bool(key); 
  }
  return def;
}

Settings settings;

void configuration_load() {
  settings = (Settings){
    .is_minutely = get_val_bool(KEY_IS_MINUTELY, false)
  };
}

void configuration_set_is_minutely(bool value) {
  persist_write_bool(KEY_IS_MINUTELY, value);
  settings.is_minutely = value;
}
#pragma once
typedef struct {
  char* api_key;
  bool is_minutely;
} Settings;

Settings settings;

void configuration_load();
void configuration_set_is_minutely(bool value);
void configuration_set_api_key(char* value);
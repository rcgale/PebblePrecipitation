#pragma once
typedef struct {
  bool is_minutely;
} Settings;

Settings settings;

void configuration_load();
void configuration_set_is_minutely(bool value);
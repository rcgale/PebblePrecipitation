#pragma once

typedef struct {
  int hours;
  int minutes;
} Time;

Time current_time;

void current_time_update(struct tm *tick_time);
 
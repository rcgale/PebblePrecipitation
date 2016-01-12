#pragma once

void clock_update(struct tm *tick_time);
Layer* clock_create(GRect window_bounds);
void clock_destroy();
#pragma once

void forecast_update(struct tm *tick_time);
Layer* forecast_create(GRect window_bounds);
void forecast_destroy();
#pragma once

void forecast_icons_update(struct tm *tick_time);
Layer* forecast_icons_create(GRect window_bounds);
void forecast_icons_destroy();
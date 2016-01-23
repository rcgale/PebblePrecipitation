#pragma once

void forecast_update();
Layer* forecast_create(GRect window_bounds);
void forecast_destroy();
void forecast_process_callback(DictionaryIterator *iterator, void *context);
void forecast_queue_refresh();
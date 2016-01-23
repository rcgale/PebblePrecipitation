#include <pebble.h>
#include <callback.h>
#include <configuration.h>
#include <forecast.h>

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *which_callback = dict_find(iterator, CALLBACK_ID_KEY);
  switch((int)which_callback->value->int16) {
    case CBID_CONFIG:
      configuration_process_callback(iterator, context);
      break;
    case CBID_FORECAST:
      forecast_process_callback(iterator, context);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Received callback with no callback id!");
      break;
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

void callback_create() {
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(1024, 1024); // TODO: Determine the correct values.
}

#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_day_label, *s_num_label;
static TextLayer *s_battery_layer;
static TextLayer *s_stardate_layer;
static TextLayer *s_weather_layer;

// create buffer for date
static char s_day_buffer[21];  
  
// create buffer for stardate
static char s_stardate_buffer[21];  

// create buffer for weather
static char s_weather_buffer[21];  

static void check_weather() {
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);

  // Send the message!
  app_message_outbox_send();
}

static void battery_handler(BatteryChargeState new_state) {
  // Write to buffer and display
  static char s_battery_buffer[32];
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "Battery: %d%%", new_state.charge_percent);
  text_layer_set_text(s_battery_layer, s_battery_buffer);
}

static void vibrate_time(struct tm *tick_time) {
  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Get the minutes & seconds
  strftime(buffer, sizeof("00:00"), "%M:%S", tick_time);
  // if top of the hour 2 buzzes
  if (strcmp(buffer, "00:00")==0) { 
    uint32_t segments[] = { 400, 200, 400 };
    VibePattern pat = {
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments),
    };
    vibes_enqueue_custom_pattern(pat);  
    check_weather();
  }
  
  // quarter hour short buzz
  if (strcmp(buffer, "15:00")==0) {
    vibes_short_pulse();
    check_weather();
  }
  
  // bottom of the hour 
  if (strcmp(buffer, "30:00")==0) {
    vibes_long_pulse();
    check_weather();
  }
  
  // quarter hour short buzz
  if (strcmp(buffer, "45:00")==0) {
    vibes_short_pulse();
    check_weather();
  }
    
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  // Use 24 hour format
  strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  
  strftime(s_day_buffer, sizeof(s_day_buffer), "%a %d %b", tick_time);
  text_layer_set_text(s_day_label, s_day_buffer);

  strftime(s_stardate_buffer, sizeof(s_stardate_buffer), "StarDate: %G.%j", tick_time);
  text_layer_set_text(s_stardate_layer, s_stardate_buffer);

  // vibrates on the 15s
  vibrate_time(tick_time);
  check_weather();

}

static void main_window_load(Window *window) {
   Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  s_day_label = text_layer_create(GRect(0, 100, 144, 24));
  text_layer_set_text(s_day_label, s_day_buffer);
  text_layer_set_background_color(s_day_label, GColorWhite);
  text_layer_set_text_color(s_day_label, GColorBlack);
  text_layer_set_font(s_day_label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_day_label, GTextAlignmentCenter);

  s_stardate_layer = text_layer_create(GRect(0, 125, 144, 24));
  text_layer_set_text(s_stardate_layer, s_day_buffer);
  text_layer_set_background_color(s_stardate_layer, GColorWhite);
  text_layer_set_text_color(s_stardate_layer, GColorBlack);
  text_layer_set_font(s_stardate_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_stardate_layer, GTextAlignmentCenter);

  s_weather_layer = text_layer_create(GRect(0, 25, 144, 24));
  text_layer_set_text(s_weather_layer, s_weather_buffer);
  text_layer_set_background_color(s_weather_layer, GColorWhite);
  text_layer_set_text_color(s_weather_layer, GColorBlack);
  text_layer_set_font(s_weather_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_label));  // Create output TextLayer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_stardate_layer));  // Create output TextLayer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));  // Create output TextLayer
  
  s_battery_layer = text_layer_create(GRect(0, 0, window_bounds.size.w,20));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_label));  // Create output TextLayer
  
  // Get the current battery level
  battery_handler(battery_state_service_peek());
  
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_day_label);
  text_layer_destroy(s_num_label);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_stardate_layer);
  text_layer_destroy(s_weather_layer);

}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);
  snprintf(temperature_buffer, sizeof(temperature_buffer), "%dF", 0);
  snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", "Loading...");
  APP_LOG(APP_LOG_LEVEL_INFO, "%d", (int)t->key);

  // For all items
  while(t != NULL) {
    // Which key was received?
    APP_LOG(APP_LOG_LEVEL_INFO, "%d", (int)t->key);
    
    switch(t->key) {
    case KEY_TEMPERATURE:
      APP_LOG(APP_LOG_LEVEL_INFO, "Temp %d", (int)t->value->int32 * 9/5 + 32);
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dF", (int)t->value->int32 * 9/5 + 32);
      break;
    case KEY_CONDITIONS:
      APP_LOG(APP_LOG_LEVEL_INFO, "%s", t->value->cstring);
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  
  // Assemble full string and display
  snprintf(s_weather_buffer, sizeof(s_weather_buffer), "%s, %s", temperature_buffer, conditions_buffer);
  text_layer_set_text(s_weather_layer, s_weather_buffer);
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

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Subscribe to the Battery State Service
  battery_state_service_subscribe(battery_handler);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
}

static void deinit() {
    // Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

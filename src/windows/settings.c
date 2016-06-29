#include "settings.h"

static const int MAX_TIME_SELECTION_TEXT_LEN = 10;

static const int STARTING_INTERVAL = 15;
static const int MIN_INTERVAL = 5;
static const int MAX_INTERVAL = 60;
static const int INTERVAL_CHANGE = 5;
static const int INTERVAL_SELECTION_BUTTON_REPEATING_DELAY = 200; //milliseconds

static TextLayer *text_layer;
static TextLayer *time_selection_layer;
static char *time_selection_text;
static Window *flight_window;

static char *text_for_minutes() {
  snprintf(time_selection_text, MAX_TIME_SELECTION_TEXT_LEN, "%i mins", get_interval());
  return time_selection_text;
}

static void update_interval_text() {
  text_layer_set_text(time_selection_layer, text_for_minutes());
}

static void selection_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  window_stack_push(flight_window, true);
}

static void selection_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  int current_interval = get_interval();
  if (current_interval < MAX_INTERVAL) {
    set_interval(current_interval + INTERVAL_CHANGE);
    update_interval_text();
  }
}

static void selection_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  int current_interval = get_interval();
  if (current_interval > MIN_INTERVAL) {
    set_interval(current_interval - INTERVAL_CHANGE);
    update_interval_text();
  }
}

static void selection_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, selection_select_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, INTERVAL_SELECTION_BUTTON_REPEATING_DELAY, selection_up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, INTERVAL_SELECTION_BUTTON_REPEATING_DELAY, selection_down_click_handler);
}

static void selection_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Choose an interval");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  
  time_selection_layer = text_layer_create((GRect) { .origin = { 0, 90 }, .size = {bounds.size.w, 20 }});
  text_layer_set_text(time_selection_layer, text_for_minutes());
  text_layer_set_text_alignment(time_selection_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(time_selection_layer));
}

static void selection_window_unload(Window *window) {
  text_layer_destroy(text_layer);
  text_layer_destroy(time_selection_layer);
}

void selection_init_vars(Window *_flight_window, PersistBaseDataV1 *config) {
  time_selection_text = (char *) calloc(MAX_TIME_SELECTION_TEXT_LEN, sizeof(char));
  flight_window = _flight_window;
  set_interval(config->interval);
}

void selection_deinit_vars() {
  free(time_selection_text);
}

void selection_init_window(Window *selection_window) {
  window_set_click_config_provider(selection_window, selection_click_config_provider);
  window_set_window_handlers(selection_window, (WindowHandlers) {
    .load = selection_window_load,
    .unload = selection_window_unload,
  });
}

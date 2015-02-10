#include <pebble.h>
#include "util.h"

static const int MAX_TIME_TEXT_LEN = 9;
static const int MAX_TIME_SELECTION_TEXT_LEN = 10;
static const int STARTING_INTERVAL = 15;
static const int MIN_INTERVAL = 5;
static const int MAX_INTERVAL = 60;
static const int INTERVAL_CHANGE = 5;
static const int INTERVAL_SELECTION_BUTTON_REPEATING_DELAY = 200; //milliseconds
  
static Window *selection_window;
static Window *flight_window;
static TextLayer *text_layer;
static TextLayer *time_selection_layer;
static char *time_selection_text;
static int current_interval;
static Layer *airplane_layer;
static bool leftWingSelected = false;
static bool rightWingSelected = false;
static TextLayer *leftElapsed;
static TextLayer *rightElapsed;
static TextLayer *leftRemaining;
static TextLayer *rightRemaining;
static char *leftElapsedBuffer;
static char *rightElapsedBuffer;
static char *leftRemainingBuffer;
static char *rightRemainingBuffer;
static time_t leftStartTime;
static time_t rightStartTime;
static time_t leftRemainingTargetTime;
static time_t rightRemainingTargetTime;

// ---------------- selection window --------------

static char *text_for_minutes() {
  snprintf(time_selection_text, MAX_TIME_SELECTION_TEXT_LEN, "%i mins", current_interval);
  return time_selection_text;
}

static void update_interval_text() {
  text_layer_set_text(time_selection_layer, text_for_minutes());
}

static void selection_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  window_stack_push(flight_window, true);
}

static void selection_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (current_interval < MAX_INTERVAL) {
    current_interval += INTERVAL_CHANGE;
    update_interval_text();
  }
}

static void selection_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (current_interval > MIN_INTERVAL) {
    current_interval -= INTERVAL_CHANGE;
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

// ------------------- Flight window ---------------------
  
static void flight_draw_airplane(Layer *layer, GContext *context) {
  graphics_context_set_stroke_color(context, GColorBlack);
  graphics_context_set_fill_color(context, GColorBlack);
  
  //assume y center is 78
  GPoint topOfTail = GPoint(10, 60);
  GPoint bottomOfTail = GPoint(10, 96);
  GPoint leftOfFuselage = GPoint(5, 78);
  GPoint rightOfFuselage = GPoint(139, 78);
  GPoint leftWingTopLeft = GPoint(120, 9);
  GPoint rightWingTopLeft = GPoint(120, 78);
  GSize wingSize = GSize(14, 70);
  GRect leftWing = (GRect) { .origin = leftWingTopLeft, .size = wingSize };
  GRect rightWing = (GRect) { .origin = rightWingTopLeft, .size = wingSize };
  
  graphics_draw_line(context, topOfTail, bottomOfTail);
  graphics_draw_line(context, leftOfFuselage, rightOfFuselage);
  
  if (leftWingSelected)
    graphics_fill_rect(context, leftWing, 0, GCornerNone);
  else
    graphics_draw_rect(context, leftWing);
  
  if (rightWingSelected)
    graphics_fill_rect(context, rightWing, 0, GCornerNone);
  else
    graphics_draw_rect(context, rightWing);
}

static char *format_seconds(long seconds, char *buffer) {
  bool negative = seconds < 0;
  if (negative) {
    seconds *= -1;
    buffer[0] = '-';
  }
  
  int hours = seconds / 3600;
  int minutes = (seconds % 3600) / 60;
  struct tm t = { .tm_sec = seconds % 60, .tm_min = minutes, .tm_hour = hours };
  char *offset = negative ? buffer + sizeof(char) : buffer;
  if (hours > 0)
    strftime(offset, MAX_TIME_TEXT_LEN, "%H:%M:%S", &t);
  else
    strftime(offset, MAX_TIME_TEXT_LEN, "%M:%S", &t);
  
  return buffer;
}

static void flight_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  airplane_layer = layer_create(bounds);
  layer_set_update_proc(airplane_layer, flight_draw_airplane);  
  
  layer_add_child(window_layer, airplane_layer);
  
  leftRemaining = text_layer_create((GRect) { .origin = { 3, 39 }, .size = { bounds.size.w - 30, 20 }});
  text_layer_set_text(leftRemaining, "00:00");
  text_layer_set_text_alignment(leftRemaining, GTextAlignmentRight);
  layer_set_hidden(text_layer_get_layer(leftRemaining), true);
  layer_add_child(window_layer, text_layer_get_layer(leftRemaining));

  leftElapsed = text_layer_create((GRect) { .origin = { 3, 11 }, .size = { bounds.size.w - 30, 28 } });
  text_layer_set_text(leftElapsed, "00:00");
  text_layer_set_font(leftElapsed, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));
  text_layer_set_text_alignment(leftElapsed, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(leftElapsed));

  rightElapsed = text_layer_create((GRect) { .origin = { 3, 109 }, .size = { bounds.size.w - 30, 28 } });
  text_layer_set_text(rightElapsed, "00:00");
  text_layer_set_font(rightElapsed, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));
  text_layer_set_text_alignment(rightElapsed, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(rightElapsed));

  rightRemaining = text_layer_create((GRect) { .origin = { 3, 96 }, .size = { bounds.size.w - 30, 20 }});
  text_layer_set_text(rightRemaining, "00:00");
  text_layer_set_text_alignment(rightRemaining, GTextAlignmentRight);
  layer_set_hidden(text_layer_get_layer(rightRemaining), true);
  layer_add_child(window_layer, text_layer_get_layer(rightRemaining));
}

static void update_tick_on_wing(TextLayer *elapsedLayer, TextLayer *remainingLayer, char *elapsedBuffer, char *remainingBuffer, time_t *startTime, time_t *remainingTime, time_t tick) {
  if (*startTime <= 0) {
    *startTime += tick;
  }

  long timeDiff = (tick - *startTime);
  text_layer_set_text(elapsedLayer, format_seconds(timeDiff, elapsedBuffer));
  
  timeDiff = (tick - *remainingTime);
  text_layer_set_text(remainingLayer, format_seconds(timeDiff, remainingBuffer));
}

static void flight_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (NULL == tick_time)
    return;

  time_t tick = local_mktime(tick_time);
  
  if (leftWingSelected) {
    update_tick_on_wing(leftElapsed, leftRemaining, leftElapsedBuffer, leftRemainingBuffer, &leftStartTime, &leftRemainingTargetTime, tick);
  } else if (rightWingSelected) {
    update_tick_on_wing(rightElapsed, rightRemaining, rightElapsedBuffer, rightRemainingBuffer, &rightStartTime, &rightRemainingTargetTime, tick);
  }
}
  
static void flight_window_unload(Window *window) {
  layer_destroy(airplane_layer);
  text_layer_destroy(leftElapsed);
  text_layer_destroy(leftRemaining);
  text_layer_destroy(rightElapsed);
  text_layer_destroy(rightRemaining);
}

static void flight_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (!leftWingSelected) {
    layer_set_hidden(text_layer_get_layer(leftRemaining), false);
    
    leftRemainingTargetTime = time(NULL) + current_interval * 60;
    //TODO move starttime setting here
  }
  if (rightWingSelected) {
    layer_set_hidden(text_layer_get_layer(rightRemaining), true);
    if (rightStartTime != 0) {
      rightStartTime = rightStartTime - time(NULL);
    }
  }
  
  leftWingSelected = true;
  rightWingSelected = false;
  
  layer_mark_dirty(airplane_layer);
}

static void flight_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (!rightWingSelected) {
    layer_set_hidden(text_layer_get_layer(rightRemaining), false);
    
    rightRemainingTargetTime = time(NULL) + current_interval * 60;
    //TODO move starttime setting here
  }
  if (leftWingSelected) {
    layer_set_hidden(text_layer_get_layer(leftRemaining), true);
    if (leftStartTime != 0) {
      leftStartTime = leftStartTime - time(NULL);
    }
  }

  rightWingSelected = true;
  leftWingSelected = false;
  
  layer_mark_dirty(airplane_layer);
}

static void flight_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, flight_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, flight_down_click_handler);
}

// ----------- init -------------

static void init_vars() {
  current_interval = STARTING_INTERVAL;
  time_selection_text = (char *) calloc(MAX_TIME_SELECTION_TEXT_LEN, sizeof(char));
  leftElapsedBuffer = (char *) calloc(MAX_TIME_TEXT_LEN, sizeof(char));
  rightElapsedBuffer = (char *) calloc(MAX_TIME_TEXT_LEN, sizeof(char));
  leftRemainingBuffer = (char *) calloc(MAX_TIME_TEXT_LEN, sizeof(char));
  rightRemainingBuffer = (char *) calloc(MAX_TIME_TEXT_LEN, sizeof(char));
}

static void init_selection_window() {
  selection_window = window_create();
  window_set_click_config_provider(selection_window, selection_click_config_provider);
  window_set_window_handlers(selection_window, (WindowHandlers) {
    .load = selection_window_load,
    .unload = selection_window_unload,
  });
}

static void init_flight_window() {
  flight_window = window_create();
  window_set_click_config_provider(flight_window, flight_click_config_provider);
  window_set_window_handlers(flight_window, (WindowHandlers) {
    .load = flight_window_load,
    .unload = flight_window_unload,
  });
  
  tick_timer_service_subscribe(SECOND_UNIT, flight_tick_handler);
}

static void init(void) {
  init_vars();
  init_selection_window();
  init_flight_window();
  
  const bool animated = true;
  window_stack_push(selection_window, animated);
}

static void deinit(void) {
  window_destroy(selection_window);
  window_destroy(flight_window);
  
  free(time_selection_text);
  free(leftElapsedBuffer);
  free(rightElapsedBuffer);
  free(leftRemainingBuffer);
  free(rightRemainingBuffer);
  tick_timer_service_unsubscribe(); 
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
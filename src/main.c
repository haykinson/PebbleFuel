#include <pebble.h>
#include "modules/util.h"
#include "modules/tank.h"
#include "modules/buzzer.h"
#include "windows/flight.h"
#include "windows/settings.h"

static const int MAX_TIME_TEXT_LEN = 9;
static const int THRESHOLD_FOR_BUZZ_NOTIFY = 0; //will buzz at this many seconds remaining

static const uint32_t left_pattern[] = { 200, 100, 200, 100, 200, 100, 200 };
static const uint32_t right_pattern[] = { 300, 100, 1300 };


static Window *selection_window;
static Window *flight_window;
static Layer *airplane_layer;
static time_t leftStartTime;
static time_t rightStartTime;
static time_t leftRemainingTargetTime;
static time_t rightRemainingTargetTime;
static bool paused;
static time_t pauseStartTime;
static time_t lastLeftDiff;
static time_t lastRightDiff;
static Tank *left_tank;
static Tank *right_tank;

#if defined(PBL_COLOR)
#define LEFT_WING_COLOR  GColorRed
#define RIGHT_WING_COLOR GColorGreen
#define UNFILLED_COLOR   GColorClear
#else
#define LEFT_WING_COLOR  GColorBlack
#define RIGHT_WING_COLOR GColorBlack
#define UNFILLED_COLOR   GColorClear
#endif

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
  
  if (left_tank->selected) {
    graphics_context_set_fill_color(context, LEFT_WING_COLOR);
    graphics_fill_rect(context, leftWing, 0, GCornerNone);
  }
  else {
    graphics_context_set_fill_color(context, UNFILLED_COLOR);
    graphics_draw_rect(context, leftWing);
  }
  
  if (right_tank->selected) {
    graphics_context_set_fill_color(context, RIGHT_WING_COLOR);
    graphics_fill_rect(context, rightWing, 0, GCornerNone);
  }
  else {
    graphics_context_set_fill_color(context, UNFILLED_COLOR);
    graphics_draw_rect(context, rightWing);
  }
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

static void reset_buzz_notification_need() {
  left_tank->notified = false;
  right_tank->notified = false;
}

static void pause() {
  pauseStartTime = time(NULL);
  paused = true;
}

static void unpause() {
  if (pauseStartTime > 0) {
    long accumulatedPauseSeconds = time(NULL) - pauseStartTime;
    if (left_tank->selected) {
      leftStartTime += accumulatedPauseSeconds;
      leftRemainingTargetTime += accumulatedPauseSeconds;
    }
    if (right_tank->selected) {
      rightStartTime += accumulatedPauseSeconds;
      rightRemainingTargetTime += accumulatedPauseSeconds;
    }
  }
  
  pauseStartTime = 0;
  paused = false;
}

static void toggle_pause() {
  if (paused)
    unpause();
  else
    pause();
}

static void update_tick_on_wing(Tank * tank,
                                long *lastRecordedDiff,
                                time_t *startTime, 
                                time_t *remainingTime, 
                                time_t tick) {
  long pauseTime = 0;
  if (paused) {
    pauseTime = tick - pauseStartTime;
  }
  
  long timeDiff = (tick - *startTime) - pauseTime;
  *lastRecordedDiff = timeDiff;
  text_layer_set_text(tank->elapsed, format_seconds(timeDiff, tank->elapsedBuffer));
  
  timeDiff = (tick - *remainingTime) - pauseTime;
  text_layer_set_text(tank->remaining, format_seconds(timeDiff, tank->remainingBuffer));
  
  if (-timeDiff < THRESHOLD_FOR_BUZZ_NOTIFY) {
    buzz_tank(tank);
  }
}

static void update_tick_on_left_wing(time_t tick) {
  update_tick_on_wing(left_tank,
                      &lastLeftDiff,
                      &leftStartTime, 
                      &leftRemainingTargetTime, 
                      tick);
}

static void update_tick_on_right_wing(time_t tick) {
  update_tick_on_wing(right_tank,
                      &lastRightDiff,
                      &rightStartTime, 
                      &rightRemainingTargetTime, 
                      tick);
}

static void flight_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (NULL == tick_time)
    return;
  
  time_t tick = mktime(tick_time); //local_mktime(tick_time);
  
  if (left_tank->selected) {
    update_tick_on_left_wing(tick);
  } else if (right_tank->selected) {
    update_tick_on_right_wing(tick);
  }
}

static void create_elapsed_text_layer(Tank * tank, Layer *window_layer, GRect where) {
  tank->elapsed = text_layer_create(where);
  text_layer_set_text(tank->elapsed, "00:00");
  text_layer_set_font(tank->elapsed, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));
  text_layer_set_text_alignment(tank->elapsed, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(tank->elapsed));
}

static void create_remaining_text_layer(Tank *tank, Layer *window_layer, GRect where) {
  tank->remaining = text_layer_create(where);
  text_layer_set_text(tank->remaining, "00:00");
  text_layer_set_text_alignment(tank->remaining, GTextAlignmentRight);
  layer_set_hidden(text_layer_get_layer(tank->remaining), !tank->selected);
  layer_add_child(window_layer, text_layer_get_layer(tank->remaining));
}

static void flight_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  airplane_layer = layer_create(bounds);
  layer_set_update_proc(airplane_layer, flight_draw_airplane);  
  
  layer_add_child(window_layer, airplane_layer);

  create_elapsed_text_layer(left_tank, window_layer, (GRect)    { .origin = { 3, 11 },  .size = { bounds.size.w - 30, 28 }});
  create_elapsed_text_layer(right_tank, window_layer, (GRect)   { .origin = { 3, 109 }, .size = { bounds.size.w - 30, 28 }});
  create_remaining_text_layer(left_tank, window_layer, (GRect)  { .origin = { 3, 39 },  .size = { bounds.size.w - 30, 20 }});
  create_remaining_text_layer(right_tank, window_layer, (GRect) { .origin = { 3, 96 },  .size = { bounds.size.w - 30, 20 }});
  
  if (leftStartTime != 0)
    text_layer_set_text(left_tank->elapsed, format_seconds(lastLeftDiff, left_tank->elapsedBuffer));
  if (rightStartTime != 0)
    text_layer_set_text(right_tank->elapsed, format_seconds(lastRightDiff, right_tank->elapsedBuffer));
}

void destroy_layers(Tank *tank) {
  text_layer_destroy(tank->elapsed);
  text_layer_destroy(tank->remaining);
}
  
static void flight_window_unload(Window *window) {
  layer_destroy(airplane_layer);
  destroy_layers(left_tank);
  destroy_layers(right_tank);
}

static void flight_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  time_t tick = time(NULL);
  unpause();
  
  if (!left_tank->selected) {
    layer_set_hidden(text_layer_get_layer(left_tank->remaining), false);
    
    leftRemainingTargetTime = tick + get_interval() * 60;
    leftStartTime += tick;
    
    reset_buzz_notification_need();
  }
  if (right_tank->selected) {
    layer_set_hidden(text_layer_get_layer(right_tank->remaining), true);
    if (rightStartTime != 0) {
      rightStartTime = rightStartTime - tick;
    }
  }
  
  left_tank->selected = true;
  right_tank->selected = false;
  
  layer_mark_dirty(airplane_layer);
  update_tick_on_left_wing(tick);
}

static void flight_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  time_t tick = time(NULL);
  unpause();
  
  if (!right_tank->selected) {
    layer_set_hidden(text_layer_get_layer(right_tank->remaining), false);
    
    rightRemainingTargetTime = tick + get_interval() * 60;
    rightStartTime += tick;
    
    reset_buzz_notification_need();
  }
  if (left_tank->selected) {
    layer_set_hidden(text_layer_get_layer(left_tank->remaining), true);
    if (leftStartTime != 0) {
      leftStartTime = leftStartTime - tick;
    }
  }

  right_tank->selected = true;
  left_tank->selected = false;
  
  layer_mark_dirty(airplane_layer);
  update_tick_on_right_wing(tick);
}

static void flight_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  toggle_pause();
}

static void flight_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, flight_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, flight_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, flight_select_click_handler);
}

// ----------- init -------------

static void init_basic_windows() {
  selection_window = window_create();
  flight_window = window_create();
}

static void init_vars() {
  left_tank = tank_create();
  right_tank = tank_create();
  tank_set_pattern(left_tank, left_pattern);
  tank_set_pattern(right_tank, right_pattern);

  selection_init_vars(flight_window);
}

static void init_selection_window() {
  window_set_click_config_provider(selection_window, selection_click_config_provider);
  window_set_window_handlers(selection_window, (WindowHandlers) {
    .load = selection_window_load,
    .unload = selection_window_unload,
  });
}

static void init_flight_window() {
  window_set_click_config_provider(flight_window, flight_click_config_provider);
  window_set_window_handlers(flight_window, (WindowHandlers) {
    .load = flight_window_load,
    .unload = flight_window_unload,
  });
  
  tick_timer_service_subscribe(SECOND_UNIT, flight_tick_handler);
}

static void init(void) {
  init_basic_windows();
  init_vars();
  init_selection_window();
  init_flight_window();
  
  const bool animated = true;
  window_stack_push(selection_window, animated);
}

static void deinit(void) {
  tick_timer_service_unsubscribe(); 

  window_destroy(selection_window);
  window_destroy(flight_window);
  
  selection_deinit_vars();

  tank_free(left_tank);
  tank_free(right_tank);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
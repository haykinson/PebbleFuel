#include "flight.h"

static const int THRESHOLD_FOR_BUZZ_NOTIFY = 0; //will buzz at this many seconds remaining

static const uint32_t left_pattern[] = { 200, 100, 200, 100, 200, 100, 200 };
static const uint32_t right_pattern[] = { 300, 100, 1300 };

static Layer *airplane_layer;

static bool paused;
static time_t pauseStartTime;
static Tank* all_tanks[2];

#if defined(PBL_COLOR)
#define LEFT_WING_COLOR  GColorRed
#define RIGHT_WING_COLOR GColorGreen
#define UNFILLED_COLOR   GColorClear
#else
#define LEFT_WING_COLOR  GColorBlack
#define RIGHT_WING_COLOR GColorBlack
#define UNFILLED_COLOR   GColorClear
#endif

  
static void flight_draw_airplane(Layer *layer, GContext *context) {
  graphics_context_set_stroke_color(context, GColorBlack);
  graphics_context_set_fill_color(context, GColorBlack);
  
  //assume y center is 78
  GPoint topOfTail = GPoint(10, 60);
  GPoint bottomOfTail = GPoint(10, 96);
  GPoint leftOfFuselage = GPoint(5, 78);
  GPoint rightOfFuselage = GPoint(139, 78);
  
  graphics_draw_line(context, topOfTail, bottomOfTail);
  graphics_draw_line(context, leftOfFuselage, rightOfFuselage);

  for (int i = 0; i < 2; i++) {
    Tank *tank = all_tanks[i];
    if (NULL != tank && tank->selected) {
      graphics_context_set_fill_color(context, tank->enabledColor);
      graphics_fill_rect(context, tank->location, 0, GCornerNone);  
    } else {
      graphics_context_set_fill_color(context, tank->disabledColor);
      graphics_draw_rect(context, tank->location);      
    }
  }
}

static void reset_buzz_notification_need() {
  for (int i = 0; i < 2; i++) {
    Tank *tank = all_tanks[i];
    if (NULL != tank) {
      tank->notified = false;
    }
  }
}

static void pause() {
  pauseStartTime = time(NULL);
  paused = true;
}

static void unpause() {
  if (pauseStartTime > 0) {
    long accumulatedPauseSeconds = time(NULL) - pauseStartTime;

    for (int i = 0; i < 2; i++) {
      Tank *tank = all_tanks[i];
      if (NULL != tank && tank->selected) {
        tank->startTime += accumulatedPauseSeconds;
        tank->remainingTargetTime += accumulatedPauseSeconds;
      }
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

static void update_tick_on_wing(Tank * tank, time_t tick) {
  long pauseTime = 0;
  if (paused) {
    pauseTime = tick - pauseStartTime;
  }
  
  long timeDiff = (tick - tank->startTime) - pauseTime;
  tank->lastDiff = timeDiff;
  text_layer_set_text(tank->elapsed, format_seconds(timeDiff, tank->elapsedBuffer));
  
  timeDiff = (tick - tank->remainingTargetTime) - pauseTime;
  text_layer_set_text(tank->remaining, format_seconds(timeDiff, tank->remainingBuffer));
  
  if (-timeDiff < THRESHOLD_FOR_BUZZ_NOTIFY) {
    buzz_tank(tank);
  }
}

static void flight_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (NULL == tick_time)
    return;
  
  time_t tick = mktime(tick_time); //local_mktime(tick_time);
  
  for (int i = 0; i < 2; i++) {
    Tank *tank = all_tanks[i];
    if (NULL != tank && tank->selected) {
      update_tick_on_wing(tank, tick);
    }
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
  
  for (int i = 0; i < 2; i++) {
    Tank *tank = all_tanks[i];
    if (NULL != tank) {
      create_elapsed_text_layer(tank, window_layer, tank->elapsedTextLocation);
      create_remaining_text_layer(tank, window_layer, tank->remainingTextLocation);

      if (tank->startTime != 0) {
        text_layer_set_text(tank->elapsed, format_seconds(tank->lastDiff, tank->elapsedBuffer));
      }
    }  
  }
}

static void destroy_layers(Tank *tank) {
  text_layer_destroy(tank->elapsed);
  text_layer_destroy(tank->remaining);
}
  
static void flight_window_unload(Window *window) {
  layer_destroy(airplane_layer);

  for (int i = 0; i < 2; i++) {
    Tank *tank = all_tanks[i];
    if (NULL != tank) {
      destroy_layers(tank);
    }
  }
}

static void flight_click_handler(Tank *tank, Tank *otherTank) {
  time_t tick = time(NULL);
  unpause();
  
  if (!tank->selected) {
    layer_set_hidden(text_layer_get_layer(tank->remaining), false);
    
    tank->remainingTargetTime = tick + get_interval() * 60;
    tank->startTime += tick;
    
    reset_buzz_notification_need();
  }
  if (otherTank->selected) {
    layer_set_hidden(text_layer_get_layer(otherTank->remaining), true);
    if (otherTank->startTime != 0) {
      otherTank->startTime = otherTank->startTime - tick;
    }
  }
  
  tank->selected = true;
  otherTank->selected = false;
  
  layer_mark_dirty(airplane_layer);
  update_tick_on_wing(tank, tick);

}

static void flight_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  flight_click_handler(all_tanks[0], all_tanks[1]);
}

static void flight_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  flight_click_handler(all_tanks[1], all_tanks[0]);
}

static void flight_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  toggle_pause();
}

static void flight_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, flight_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, flight_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, flight_select_click_handler);
}

void flight_init_vars(Window *flight_window) {
  Tank *left_tank = tank_create();
  Tank *right_tank = tank_create();
  tank_set_pattern(left_tank, left_pattern);
  tank_set_pattern(right_tank, right_pattern);
  tank_set_colors(left_tank, LEFT_WING_COLOR, UNFILLED_COLOR);
  tank_set_colors(right_tank, RIGHT_WING_COLOR, UNFILLED_COLOR);

  //TODO remove dependency on UI here, or make a better dependency altogether
  Layer *window_layer = window_get_root_layer(flight_window);
  GRect bounds = layer_get_bounds(window_layer);

  GPoint leftWingTopLeft = GPoint(120, 9);
  GPoint rightWingTopLeft = GPoint(120, 78);
  GSize wingSize = GSize(14, 70);
  GRect leftWing = (GRect) { .origin = leftWingTopLeft, .size = wingSize };
  GRect rightWing = (GRect) { .origin = rightWingTopLeft, .size = wingSize };

  tank_set_location(left_tank, leftWing);
  tank_set_location(right_tank, rightWing);

  tank_set_text_locations(left_tank, 
    (GRect) { .origin = { 3, 11 },  .size = { bounds.size.w - 30, 28 }},
    (GRect) { .origin = { 3, 39 },  .size = { bounds.size.w - 30, 20 }});
  tank_set_text_locations(right_tank, 
    (GRect) { .origin = { 3, 109 }, .size = { bounds.size.w - 30, 28 }},
    (GRect) { .origin = { 3, 96 },  .size = { bounds.size.w - 30, 20 }});

  all_tanks[0] = left_tank;
  all_tanks[1] = right_tank;
}

void flight_deinit_vars() {
  for (int i = 0; i < 2; i++) {
	Tank *tank = all_tanks[i];
    if (NULL != tank) {
      tank_free(tank);
    }
  }
}

void flight_init_window(Window *flight_window) {
  window_set_click_config_provider(flight_window, flight_click_config_provider);
  window_set_window_handlers(flight_window, (WindowHandlers) {
    .load = flight_window_load,
    .unload = flight_window_unload,
  });
  
  tick_timer_service_subscribe(SECOND_UNIT, flight_tick_handler);
}

#include "flight.h"

static const int THRESHOLD_FOR_BUZZ_NOTIFY = 0; //will buzz at this many seconds remaining

static const uint32_t left_pattern[] = { 200, 100, 200, 100, 200, 100, 200, 2000, 200, 100, 200, 100, 200, 100, 200 };
static const uint32_t right_pattern[] = { 300, 100, 1300, 2000, 300, 100, 1300 };

static Layer *airplane_layer;

static bool initialized = false;
static bool paused;
static TankUI* all_tanks[2];

#define TANK_COUNT 2

#if defined(PBL_COLOR)
#define LEFT_WING_COLOR  GColorRed
#define RIGHT_WING_COLOR GColorGreen
#define UNFILLED_COLOR   GColorClear
#else
#define LEFT_WING_COLOR  GColorBlack
#define RIGHT_WING_COLOR GColorBlack
#define UNFILLED_COLOR   GColorClear
#endif

TankUI **flight_get_tanks() {
  return all_tanks;
}

  
static void flight_draw_airplane(Layer *layer, GContext *context) {
  graphics_context_set_stroke_color(context, GColorBlack);
  graphics_context_set_fill_color(context, GColorBlack);
  
  //assume y center is 94
  GPoint topOfTail = GPoint(10, 76);
  GPoint bottomOfTail = GPoint(10, 112);
  GPoint leftOfFuselage = GPoint(5, 94);
  GPoint rightOfFuselage = GPoint(139, 94);
  
  graphics_draw_line(context, topOfTail, bottomOfTail);
  graphics_draw_line(context, leftOfFuselage, rightOfFuselage);

  for (int i = 0; i < TANK_COUNT; i++) {
    TankUI *tankui = all_tanks[i];
    if (NULL != tankui && tankui->tank->selected) {
      graphics_context_set_fill_color(context, tankui->enabledColor);
      graphics_fill_rect(context, tankui->location, 0, GCornerNone);
    } else {
      graphics_context_set_fill_color(context, tankui->disabledColor);
      graphics_draw_rect(context, tankui->location);      
    }
  }
}

static void reset_buzz_notification_need() {
  for (int i = 0; i < TANK_COUNT; i++) {
    TankUI *tankui = all_tanks[i];
    if (NULL != tankui) {
      tankui->tank->notified = false;
    }
  }
}

static void pause() {
	//TODO make better!
	if (all_tanks[0]->tank->selected) {
		tank_pause(all_tanks[0]->tank);
	} else {
		tank_pause(all_tanks[1]->tank);
	}

  paused = true;
  set_paused(paused);
}

static void unpause() {
	//TODO make better!
	if (all_tanks[0]->tank->selected) {
		tank_unpause(all_tanks[0]->tank, time(NULL));
	} else {
		tank_unpause(all_tanks[1]->tank, time(NULL));
	}

  paused = false;
  set_paused(paused);
}

static void toggle_pause() {
  if (paused)
    unpause();
  else
    pause();
}

static void flight_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  StatusBarLayer *status_bar = status_bar_layer_create();
  status_bar_layer_set_separator_mode(status_bar, StatusBarLayerSeparatorModeDotted);
#if defined(PBL_COLOR)
  status_bar_layer_set_colors(status_bar, GColorCeleste, GColorBulgarianRose);
#endif

  layer_add_child(window_layer, status_bar_layer_get_layer(status_bar));
  
  airplane_layer = layer_create(bounds);
  layer_set_update_proc(airplane_layer, flight_draw_airplane);  
  
  layer_add_child(window_layer, airplane_layer);

  //TODO change init code  
  for (int i = 0; i < TANK_COUNT; i++) {
    TankUI *tankui = all_tanks[i];
    tankui_create_layers(tankui);
    tankui_update_elapsed_time(tankui);
    if (tankui->tank->selected) {
    	layer_set_hidden(text_layer_get_layer(tankui->remainingLayer), false);
    }
  }

  initialized = true;
}
  
static void flight_window_unload(Window *window) {
  layer_destroy(airplane_layer);

  for (int i = 0; i < TANK_COUNT; i++) {
  	TankUI *tankui = all_tanks[i];
	tankui_destroy_layers(tankui);
  }
}

static void flight_click_handler(TankUI *tankui, TankUI *otherTankUI) {
  time_t tick = time(NULL);
  unpause();
  
  if (!tankui->tank->selected) {
    layer_set_hidden(text_layer_get_layer(tankui->remainingLayer), false);
    
    tank_unpause(tankui->tank, tick);

    //TODO this feels awkward...
    if (!tankui->tank->initialized) {
	    tank_set_started(tankui->tank, tick);
    	tank_set_expires(tankui->tank, tick + get_interval() * 60);
    }

    reset_buzz_notification_need();
  }
  if (otherTankUI->tank->selected) {
    layer_set_hidden(text_layer_get_layer(otherTankUI->remainingLayer), true);
    tank_pause(otherTankUI->tank);
  }

  
  tankui->tank->selected = true;
  otherTankUI->tank->selected = false;
  
  layer_mark_dirty(airplane_layer);
  //TODO force update right now

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

void flight_init_vars(Window *flight_window, PersistTankV1 **tankConfig) {
  Layer *window_layer = window_get_root_layer(flight_window);
  GRect bounds = layer_get_bounds(window_layer);

  TankUI *left_tank = tankui_create_with_tank(tank_create(tankConfig[0]), window_layer);
  TankUI *right_tank = tankui_create_with_tank(tank_create(tankConfig[1]), window_layer);

  tank_set_tick_callback(left_tank->tank, tankui_update_tick, left_tank);
  tank_set_tick_callback(right_tank->tank, tankui_update_tick, right_tank);

  tank_set_pattern(left_tank->tank, left_pattern);
  tank_set_pattern(right_tank->tank, right_pattern);
  tankui_set_colors(left_tank, LEFT_WING_COLOR, UNFILLED_COLOR);
  tankui_set_colors(right_tank, RIGHT_WING_COLOR, UNFILLED_COLOR);

  //TODO remove dependency on UI here, or make a better dependency altogether

  GPoint leftWingTopLeft = GPoint(120, 25);
  GPoint rightWingTopLeft = GPoint(120, 94);
  GSize wingSize = GSize(14, 70);
  GRect leftWing = (GRect) { .origin = leftWingTopLeft, .size = wingSize };
  GRect rightWing = (GRect) { .origin = rightWingTopLeft, .size = wingSize };

  tankui_set_location(left_tank, leftWing);
  tankui_set_location(right_tank, rightWing);

  tankui_set_text_locations(left_tank, 
    (GRect) { .origin = { 3, 27 },  .size = { bounds.size.w - 30, 28 }},
    (GRect) { .origin = { 3, 55 },  .size = { bounds.size.w - 30, 20 }});
  tankui_set_text_locations(right_tank, 
    (GRect) { .origin = { 3, 125 }, .size = { bounds.size.w - 30, 28 }},
    (GRect) { .origin = { 3, 112 },  .size = { bounds.size.w - 30, 20 }});

  all_tanks[0] = left_tank;
  all_tanks[1] = right_tank;
}

void flight_deinit_vars() {
  for (int i = 0; i < TANK_COUNT; i++) {
	TankUI *tankui = all_tanks[i];
    if (NULL != tankui) {
      tank_free(tankui->tank);
      tankui_destroy(tankui);
    }
  }
}

void flight_init_window(Window *flight_window) {
  window_set_click_config_provider(flight_window, flight_click_config_provider);
  window_set_window_handlers(flight_window, (WindowHandlers) {
    .load = flight_window_load,
    .unload = flight_window_unload,
  });

  Tank **tanks = malloc(sizeof(Tank *) * TANK_COUNT);
  for (int i = 0; i < TANK_COUNT; i++) {
  	tanks[i] = all_tanks[i]->tank;
  }

  timing_set_tanks(tanks, TANK_COUNT);
  
  tick_timer_service_subscribe(SECOND_UNIT, timing_update_tick);
}

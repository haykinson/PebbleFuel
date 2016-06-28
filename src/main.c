#include <pebble.h>
#include "modules/util.h"
#include "modules/tank.h"
#include "modules/buzzer.h"
#include "windows/flight.h"
#include "windows/settings.h"


static Window *selection_window;
static Window *flight_window;

// ------------------- Flight window ---------------------


// ----------- init -------------

static void init_basic_windows() {
  selection_window = window_create();
  flight_window = window_create();
}

static void init_vars() {
  flight_init_vars(flight_window);
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
  flight_deinit_vars();

}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
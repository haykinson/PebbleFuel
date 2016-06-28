#include <pebble.h>
#include "windows/flight.h"
#include "windows/settings.h"

static Window *selection_window;
static Window *flight_window;

static void init_basic_windows() {
  selection_window = window_create();
  flight_window = window_create();
}

static void init_vars() {
  flight_init_vars(flight_window);
  selection_init_vars(flight_window);
}

static void init(void) {
  init_basic_windows();
  init_vars();
  selection_init_window(selection_window);
  flight_init_window(flight_window);
  
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
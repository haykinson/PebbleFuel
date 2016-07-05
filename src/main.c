#include <pebble.h>
#include "windows/flight.h"
#include "windows/settings.h"
#include "modules/timing.h"
#include "modules/persistence.h"
#include "modules/tank.h"

static Window *selection_window;
static Window *flight_window;

static PersistBaseDataV1 currentConfig;
static PersistTankV1 **tankConfig;

static void init_basic_windows() {
  selection_window = window_create();
  flight_window = window_create();
}

static void init_vars() {
  flight_init_vars(flight_window, tankConfig);
  selection_init_vars(flight_window, &currentConfig);
}

static void create_default_config() {
  currentConfig.num_tanks = 2;
  currentConfig.interval = 15;
  currentConfig.running = false;
  currentConfig.time_paused = 0;

  tankConfig = malloc(sizeof(PersistTankV1 *) * currentConfig.num_tanks);
  for (int i = 0; i < 2; i++) {
    tankConfig[i] = malloc(sizeof(PersistTankV1));
    tankConfig[i]->selected = false;
    tankConfig[i]->started = 0;
    tankConfig[i]->elapsed = 0;
  }
}

static void read_config() {
  create_default_config();
  /*
  APP_LOG(APP_LOG_LEVEL_INFO, "Starting up");

  bool has_config = persistence_has_config();
  APP_LOG(APP_LOG_LEVEL_INFO, "Config: %s", has_config ? "yes" : "no");

  if (has_config) {
    persistence_read_config();

    currentConfig = *persistence_get_config();
    tankConfig = persistence_get_tank_config();

    APP_LOG(APP_LOG_LEVEL_INFO, "Interval: %ld", currentConfig.interval);
    APP_LOG(APP_LOG_LEVEL_INFO, "Tank Config[0] selected: %s", tankConfig[0]->selected ? "yes" : "no");
    APP_LOG(APP_LOG_LEVEL_INFO, "Tank Config[0] started: %ld", tankConfig[0]->started);
    APP_LOG(APP_LOG_LEVEL_INFO, "Tank Config[0] elapsed: %ld", tankConfig[0]->elapsed);
    APP_LOG(APP_LOG_LEVEL_INFO, "Tank Config[1] selected: %s", tankConfig[1]->selected ? "yes" : "no");
    APP_LOG(APP_LOG_LEVEL_INFO, "Tank Config[1] started: %ld", tankConfig[1]->started);
    APP_LOG(APP_LOG_LEVEL_INFO, "Tank Config[1] elapsed: %ld", tankConfig[1]->elapsed);
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "Creating default config");
    create_default_config();
  }
  */
}


void gather_current_config() {
  currentConfig.num_tanks = 2; //TODO
  currentConfig.interval = get_interval();
  currentConfig.running = !get_paused();
  currentConfig.time_paused = 0; //TODO

  //reset tank config
  if (NULL != tankConfig) {
    for (int i = 0; i < (int) currentConfig.num_tanks; i++) {
      if (NULL != tankConfig[i]) {
        free(tankConfig[i]);
      }
    }
    free(tankConfig);
  }

  TankUI **real_tanks = flight_get_tanks();

  tankConfig = malloc(sizeof(PersistTankV1 *) * currentConfig.num_tanks);
  for (int i = 0; i < 2; i++) {
    tankConfig[i] = malloc(sizeof(PersistTankV1));
    tankConfig[i]->selected = real_tanks[i]->tank->selected; 
    tankConfig[i]->started = real_tanks[i]->tank->started;
    tankConfig[i]->elapsed = 0; //TODO
  }

}


static void write_config() {
  gather_current_config();
  APP_LOG(APP_LOG_LEVEL_INFO, "Gathered config");
  persistence_write_config(&currentConfig, tankConfig);
  APP_LOG(APP_LOG_LEVEL_INFO, "Wrote config");
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

void schedule_wakeups() {
  if (!get_paused()) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Exiting, would schedule wakeup in %ld", (long) (get_remaining_target_time() - time(NULL)));
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "Existing, no wakeup cause paused");
  }
}

int main(void) {
  read_config();

  init();
  app_event_loop();

  //write_config();
  //schedule_wakeups();

  deinit();
}
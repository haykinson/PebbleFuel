#include <pebble.h>
#include "windows/flight.h"
#include "windows/settings.h"
#include "modules/timing.h"
#include "modules/persistence.h"
#include "modules/tank.h"

#define CONFIG_TIME_CUTOFF 86400

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
  currentConfig.last_tick = 0;

  tankConfig = malloc(sizeof(PersistTankV1 *) * currentConfig.num_tanks);
  for (int i = 0; i < 2; i++) {
    tankConfig[i] = malloc(sizeof(PersistTankV1));
    tankConfig[i]->selected = false;
    tankConfig[i]->started = 0;
    tankConfig[i]->expires = 0;
    tankConfig[i]->remaining = 0;
    tankConfig[i]->elapsed = 0;
    tankConfig[i]->initialized = false;
    tankConfig[i]->paused = true;
  }
}

static void read_config() {
  //create_default_config();
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Starting up");

  bool has_config = persistence_has_config();
  APP_LOG(APP_LOG_LEVEL_INFO, "Config: %s", has_config ? "yes" : "no");

  if (has_config) {
    persistence_read_config();

    currentConfig = *persistence_get_config();
    //ensure not past the cutoff
    if (currentConfig.last_tick > 0 && (time(NULL) - currentConfig.last_tick) > CONFIG_TIME_CUTOFF) {
      int lastInterval = currentConfig.interval;
      APP_LOG(APP_LOG_LEVEL_INFO, "Past saved time cutoff; going with default config except interval");
      create_default_config();
      currentConfig.interval = lastInterval;
      return;
    }


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
}

static void cleanup_config() {
  if (NULL != tankConfig) {
    for (int i = 0; i < (int)currentConfig.num_tanks; i++) {
      if (NULL != tankConfig[i]) {
        free(tankConfig[i]);
        tankConfig[i] = NULL;
      }
    }
    free(tankConfig);
    tankConfig = NULL;
  }
}

void gather_current_config() {
  currentConfig.num_tanks = 2; //TODO
  currentConfig.interval = get_interval();
  currentConfig.running = !get_paused();
  currentConfig.time_paused = 0; //TODO
  currentConfig.last_tick = time(NULL);

  cleanup_config();

  TankUI **real_tanks = flight_get_tanks();

  //TODO pause??

  tankConfig = malloc(sizeof(PersistTankV1 *) * currentConfig.num_tanks);
  for (int i = 0; i < 2; i++) {
    tankConfig[i] = malloc(sizeof(PersistTankV1));
    tankConfig[i]->selected = real_tanks[i]->tank->selected; 
    tankConfig[i]->initialized = real_tanks[i]->tank->initialized;
    tankConfig[i]->paused = real_tanks[i]->tank->paused;
    tankConfig[i]->started = real_tanks[i]->tank->started;
    tankConfig[i]->elapsed = real_tanks[i]->tank->elapsed;
    tankConfig[i]->remaining = real_tanks[i]->tank->remaining;
    tankConfig[i]->expires = real_tanks[i]->tank->expires;
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

  flight_deinit_window(flight_window);

  window_destroy(selection_window);
  window_destroy(flight_window);
  
  selection_deinit_vars();
  flight_deinit_vars();

  cleanup_config();
}

void schedule_wakeups() {
  bool scheduled = false;

  TankUI **real_tanks = flight_get_tanks();

  for (int i = 0; i < 2; i++) {
    Tank *tank = real_tanks[i]->tank;
    if (tank->selected && !tank->paused) {
      time_t wakeup_time = tank->expires;
      APP_LOG(APP_LOG_LEVEL_INFO, "Exiting, would schedule wakeup in %ld", (long) (wakeup_time));
      WakeupId id = wakeup_schedule(wakeup_time, 0, false);

      if (id > 0) {
        APP_LOG(APP_LOG_LEVEL_INFO, "Wakeup scheduled with id %d", (int) id);
        currentConfig.wakeup_id = id;
        scheduled = true;
      }
    }
  }

  if (!scheduled) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Existing, no wakeup cause paused");
  }
}

static void check_wakeups() {
  if(launch_reason() == APP_LAUNCH_WAKEUP) {
    // The app was started by a wakeup event.
    WakeupId id = 0;
    int32_t reason = 0;

    // Get details and handle the event appropriately
    wakeup_get_launch_event(&id, &reason);
    APP_LOG(APP_LOG_LEVEL_INFO, "Launched because of wakeup id %d with reason %d", (int) id, (int) reason);
    currentConfig.wakeup_id = 0;
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "Didn't launch with wakeups, checking if any were set to cancel");
    if (currentConfig.wakeup_id > 0) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Found config for wakeup id %d; canceling all wakeups", currentConfig.wakeup_id);

      wakeup_cancel_all();

      currentConfig.wakeup_id = 0;
    }
  }

}

int main(void) {
  read_config();

  check_wakeups();

  init();
  app_event_loop();

  schedule_wakeups();
  write_config();

  deinit();
}
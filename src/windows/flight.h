#pragma once

#include <pebble.h>

#include "../modules/tank.h"
#include "../modules/util.h"
#include "../modules/buzzer.h"

//TODO remove window cross-dependency
#include "../windows/settings.h"

void flight_init_vars(Window *flight_window);
void flight_deinit_vars();
void flight_tick_handler(struct tm *tick_time, TimeUnits units_changed);
void flight_click_config_provider(void *context);
void flight_window_load(Window *window);
void flight_window_unload(Window *window);
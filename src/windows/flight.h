#pragma once

#include <pebble.h>

#include "../modules/tank.h"
#include "../modules/util.h"
#include "../modules/buzzer.h"
#include "../modules/timing.h"

void flight_init_vars(Window *flight_window);
void flight_deinit_vars();
void flight_init_window(Window *flight_window);
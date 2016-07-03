#pragma once

#include <pebble.h>

#include "../modules/tank.h"
#include "../modules/util.h"
#include "../modules/buzzer.h"
#include "../modules/timing.h"
#include "../modules/persistence.h"
#include "../modules/tankui.h"

TankUI **flight_get_tanks();
void flight_init_vars(Window *flight_window, PersistTankV1 **tankConfig);
void flight_deinit_vars();
void flight_init_window(Window *flight_window);
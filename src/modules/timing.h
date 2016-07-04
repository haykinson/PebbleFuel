#pragma once

#include <pebble.h>
#include "tank.h"

int get_interval();
void set_interval(int new_value);
void set_remaining_target_time(time_t when);
time_t get_remaining_target_time();
void set_paused(bool is_paused);
bool get_paused();
void timing_set_tanks(Tank **tanks, int numTanks);
void timing_update_tick(struct tm *tick_time, TimeUnits units_changed);
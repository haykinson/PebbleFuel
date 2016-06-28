#pragma once

#include <pebble.h>

int get_interval();
void set_interval(int new_value);
void set_remaining_target_time(time_t when);
time_t get_remaining_target_time();
void set_paused(bool is_paused);
bool get_paused();
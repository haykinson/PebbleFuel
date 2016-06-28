#pragma once

#include <pebble.h>

#include "../modules/timing.h"

void selection_init_vars();
void selection_deinit_vars();
int get_interval();
void selection_init_window(Window *selection_window);
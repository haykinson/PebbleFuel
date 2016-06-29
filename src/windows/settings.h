#pragma once

#include <pebble.h>

#include "../modules/timing.h"
#include "../modules/persistence.h"

void selection_init_vars(Window *flight_window, PersistBaseDataV1 *config);
void selection_deinit_vars();
int get_interval();
void selection_init_window(Window *selection_window);
#pragma once

#include <pebble.h>

typedef struct {
	uint32_t num_tanks;
	uint32_t interval;
	bool running;
	time_t time_paused;
} PersistBaseDataV1;

typedef struct {
	bool selected;
	time_t started;
	time_t elapsed;
} PersistTankV1;

bool persistence_has_config();
bool persistence_read_config();
PersistBaseDataV1 *persistence_get_config();

void persistence_write_config(PersistBaseDataV1 *newConfig);
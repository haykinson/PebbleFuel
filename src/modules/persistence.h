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
	bool paused;
	bool initialized;
	time_t started;
	time_t elapsed;
	time_t expires;
	time_t remaining;
} PersistTankV1;

bool persistence_has_config();
bool persistence_read_config();
PersistBaseDataV1 *persistence_get_config();
PersistTankV1 **persistence_get_tank_config();

void persistence_write_config(PersistBaseDataV1 *newConfig, PersistTankV1 **tankConfig);
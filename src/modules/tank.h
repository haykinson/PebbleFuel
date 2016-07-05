#pragma once

#include <pebble.h>
#include "../modules/persistence.h"
#include "../modules/util.h"

typedef struct Tank Tank;

struct Tank {
	bool selected;
	bool notified;
	VibePattern *vibePattern;
	void (*callback)(Tank *, time_t, void *);
	void *callbackContext;
	
	bool paused;
	bool initialized;
	time_t started;
	time_t elapsed;
	time_t latestTick;
	time_t expires;
	time_t remaining;

};

Tank *tank_create(PersistTankV1 *tankConfig);
void tank_free(Tank *tank);
void tank_set_pattern(Tank *tank, const uint32_t *pattern);
void tank_update_tick(Tank *tank, time_t tick);
void tank_set_tick_callback(Tank *tank, void (*callback)(Tank *, time_t, void *), void *callbackContext);

void tank_set_started(Tank *tank, time_t started);
void tank_set_expires(Tank *tank, time_t expires);
void tank_pause(Tank *tank);
void tank_unpause(Tank *tank, time_t tick);
time_t tank_get_elapsed(Tank *tank);
time_t tank_get_remaining(Tank *tank);


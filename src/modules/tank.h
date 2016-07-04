#pragma once

#include <pebble.h>
#include "../modules/persistence.h"
#include "../modules/util.h"

typedef struct Tank Tank;

struct Tank {
	bool selected;
	time_t startTime;
	time_t remainingTargetTime;
	bool notified;
	time_t lastDiff;
	VibePattern *vibePattern;
	void (*callback)(Tank *, time_t);
};

Tank *tank_create(PersistTankV1 *tankConfig);
void tank_free(Tank *tank);
void tank_set_pattern(Tank *tank, const uint32_t *pattern);
void tank_update_tick(Tank *tank, time_t tick);
void tank_set_tick_callback(Tank *tank, void (*callback)(Tank *, time_t));
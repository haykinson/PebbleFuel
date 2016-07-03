#pragma once

#include <pebble.h>
#include "../modules/persistence.h"
#include "../modules/util.h"

typedef struct Tank {
	bool selected;
	time_t startTime;
	time_t remainingTargetTime;
	bool notified;
	time_t lastDiff;
	VibePattern *vibePattern;
} Tank;

Tank *tank_create(PersistTankV1 *tankConfig);
void tank_free(Tank *tank);
void tank_set_pattern(Tank *tank, const uint32_t *pattern);

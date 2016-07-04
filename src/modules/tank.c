#include "tank.h"

Tank *tank_create(PersistTankV1 *tankConfig) {
	Tank * tank = (Tank*) malloc(sizeof(Tank));

	if (NULL != tankConfig) {
		tank->selected = tankConfig->selected;
		tank->startTime = tankConfig->started;
		tank->remainingTargetTime = tankConfig->started + 100; //TODO
		//TODO elapsed
	}
	return tank;
}

void tank_free(Tank *tank) {
	if (NULL != tank) {
		if (NULL != tank->vibePattern) {
			free(tank->vibePattern);
		}
 		free(tank);
	}
}

void tank_set_pattern(Tank *tank, const uint32_t *pattern) {
	if (NULL == tank) {
		return;
	}

	//if previously set, free it
	if (NULL != tank->vibePattern) {
		free(tank->vibePattern);
	}

	tank->vibePattern = (VibePattern *) malloc(sizeof(VibePattern));

    tank->vibePattern->durations = (uint32_t *) pattern;
    tank->vibePattern->num_segments = ARRAY_LENGTH(pattern);
}

void tank_set_tick_callback(Tank *tank, void (*callback)(Tank *, time_t)) {
	if (NULL == tank) {
		return;
	}

	tank->callback = callback;
}

void tank_update_tick(Tank *tank, time_t tick) {
	if (NULL == tank) {
		return;
	}

	//TODO

	if (NULL != tank->callback) {
		tank->callback(tank, tick);
	}
}
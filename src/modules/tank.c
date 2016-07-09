#include "tank.h"

Tank *tank_create(PersistTankV1 *tankConfig) {
	Tank * tank = (Tank*) malloc(sizeof(Tank));
	tank->initialized = false;
	tank->paused = true;

	if (NULL != tankConfig) {
		tank->selected = tankConfig->selected;
		//tank->startTime = tankConfig->started;
		//tank->remainingTargetTime = tankConfig->started + 100; //TODO
		//TODO elapsed
	}
	return tank;
}

void tank_free(Tank *tank) {
	if (NULL != tank) {
		if (NULL != tank->vibePattern) {
			free(tank->vibePattern);
			tank->vibePattern = NULL;
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

void tank_set_tick_callback(Tank *tank, void (*callback)(Tank *, time_t, void *), void *callbackContext) {
	if (NULL == tank) {
		return;
	}

	tank->callback = callback;
	tank->callbackContext = callbackContext;
}

void tank_update_tick(Tank *tank, time_t tick) {
	if (NULL == tank || tank->paused) {
		return;
	}

	tank->latestTick = tick;
	//APP_LOG(APP_LOG_LEVEL_INFO, "Update tick to %ld", tick);

	if (NULL != tank->callback) {
		tank->callback(tank, tick, tank->callbackContext);
	}
}

void tank_set_started(Tank *tank, time_t started) {
	if (NULL == tank) {
		return;
	}

	tank->started = started;
	tank->initialized = true;
}

void tank_set_expires(Tank *tank, time_t expires) {
	if (NULL == tank) {
		return;
	}

	tank->expires = expires;
}

void tank_pause(Tank *tank) {
	if (NULL == tank || tank->paused || !tank->initialized) {
		return;
	}

	tank->elapsed = tank_get_elapsed(tank);
	tank->remaining = tank_get_remaining(tank);
	tank->paused = true;

	//APP_LOG(APP_LOG_LEVEL_INFO, "Tank Paused, set: elapsed %ld, remaining %ld", tank->elapsed, tank->remaining);
}

void tank_unpause(Tank *tank, time_t tick) {
	if (NULL == tank || !tank->paused) {
		return;
	}

	//APP_LOG(APP_LOG_LEVEL_INFO, "Tank Unpausing: elapsed %ld, remaining %ld", tank->elapsed, tank->remaining);

	tank->started = tick - tank->elapsed;
	tank->expires = tick - tank->remaining;
	tank->paused = false;
}

time_t tank_get_elapsed(Tank *tank) {
	if (NULL == tank) {
		return 0;
	}

	//APP_LOG(APP_LOG_LEVEL_INFO, "Get Elapsed: last tick %ld, started %ld", tank->latestTick, tank->started);

	return tank->latestTick - tank->started;
}

time_t tank_get_remaining(Tank *tank) {
	if (NULL == tank) {
		return 0;
	}

	//APP_LOG(APP_LOG_LEVEL_INFO, "Get remaining: last tick %ld, expires %ld", tank->latestTick, tank->expires);

	return tank->latestTick - tank->expires;
}


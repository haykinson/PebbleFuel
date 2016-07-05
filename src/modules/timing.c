#include "timing.h"

static int interval;
static time_t remaining_target_time;
static bool paused = true;
static Tank **tanks_ = NULL;
static int numTanks_ = 0;

int get_interval() {
	return interval;
}

void set_interval(int new_value) {
	interval = new_value;
}

void set_remaining_target_time(time_t when) {
	remaining_target_time = when;
}

time_t get_remaining_target_time() {
	return remaining_target_time;
}

void set_paused(bool is_paused) {
	paused = is_paused;
}

bool get_paused() {
	return paused;
}

void timing_set_tanks(Tank **tanks, int numTanks) {
	tanks_ = tanks;
	numTanks_ = numTanks;
}

void timing_update_tick(struct tm *tick_time, TimeUnits units_changed) {
	//APP_LOG(APP_LOG_LEVEL_INFO, "Calling timing tick update");

	if (NULL == tanks_) {
		APP_LOG(APP_LOG_LEVEL_INFO, "tanks not set");
		return;
	}

	if (NULL == tick_time) {
		APP_LOG(APP_LOG_LEVEL_INFO, "no tick time");
		return;
	}

	time_t tick = mktime(tick_time);

	for (int i = 0; i < numTanks_; i++) {
		Tank *tank = tanks_[i];
		if (tank->selected) {
			tank_update_tick(tank, tick);
		}
 	}
}
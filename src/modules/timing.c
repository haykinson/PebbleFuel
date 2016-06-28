#include "timing.h"

static int interval;
static time_t remaining_target_time;
static bool paused = true;

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
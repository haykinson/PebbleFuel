#pragma once

#include <pebble.h>

typedef struct Tank {
	bool selected;
	TextLayer *elapsed;
	TextLayer *remaining;
	char *elapsedBuffer;
	char *remainingBuffer;
	time_t startTime;
	time_t remainingTargetTime;
	bool notified;
	time_t lastDiff;
	VibePattern *vibePattern;

} Tank;

Tank *tank_create();
void tank_free(Tank *tank);

void tank_set_pattern(Tank *tank, const uint32_t *pattern);

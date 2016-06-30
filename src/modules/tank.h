#pragma once

#include <pebble.h>
#include "../modules/persistence.h"

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
	GColor enabledColor;
	GColor disabledColor;
	GRect location;
	GRect elapsedTextLocation;
	GRect remainingTextLocation;
} Tank;

Tank *tank_create(PersistTankV1 *tankConfig);
void tank_free(Tank *tank);

void tank_set_pattern(Tank *tank, const uint32_t *pattern);
void tank_set_colors(Tank *tank, GColor enabledColor, GColor disabledColor);
void tank_set_location(Tank *tank, GRect location);
void tank_set_text_locations(Tank *tank, GRect elapsedTextLocation, GRect remainingTextLocation);
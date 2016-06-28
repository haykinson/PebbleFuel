#include "tank.h"

static const int MAX_TIME_TEXT_LEN = 9;

Tank *tank_create() {
	Tank * tank = (Tank*) malloc(sizeof(Tank));
	tank->elapsedBuffer = (char *) calloc(MAX_TIME_TEXT_LEN, sizeof(char));
	tank->remainingBuffer = (char *) calloc(MAX_TIME_TEXT_LEN, sizeof(char));
	return tank;
}

void tank_free(Tank *tank) {
	if (NULL != tank) {
		if (NULL != tank->vibePattern) {
			free(tank->vibePattern);
		}
		if (NULL != tank->elapsedBuffer) {
			free(tank->elapsedBuffer);
		}
		if (NULL != tank->remainingBuffer) {
			free(tank->remainingBuffer);
		}

 		free(tank);
	}
}

void tank_set_pattern(Tank *tank, const uint32_t *pattern) {
	//if previously set, free it
	if (NULL != tank->vibePattern) {
		free(tank->vibePattern);
	}

	tank->vibePattern = (VibePattern *) malloc(sizeof(VibePattern));

    tank->vibePattern->durations = (uint32_t *) pattern;
    tank->vibePattern->num_segments = ARRAY_LENGTH(pattern);
}

void tank_set_colors(Tank *tank, GColor enabledColor, GColor disabledColor) {
	if (NULL != tank) {
		tank->enabledColor = enabledColor;
		tank->disabledColor = disabledColor;
	}
}

void tank_set_location(Tank *tank, GRect location) {
	if (NULL != tank) {
		tank->location = location;
	}
}

void tank_set_text_locations(Tank *tank, GRect elapsedTextLocation, GRect remainingTextLocation) {
	if (NULL != tank) {
		tank->elapsedTextLocation = elapsedTextLocation;
		tank->remainingTextLocation = remainingTextLocation;
	}
}
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
	//tank->buzzPattern =  (uint32_t *) pattern;
	tank->vibePattern = (VibePattern *) malloc(sizeof(VibePattern));

    tank->vibePattern->durations = (uint32_t *) pattern;
    tank->vibePattern->num_segments = ARRAY_LENGTH(pattern);
}

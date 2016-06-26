#include "tank.h"

Tank *tank_create() {
	Tank * tank = (Tank*) malloc(sizeof(Tank));
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
	//tank->buzzPattern =  (uint32_t *) pattern;
	tank->vibePattern = (VibePattern *) malloc(sizeof(VibePattern));

    tank->vibePattern->durations = (uint32_t *) pattern;
    tank->vibePattern->num_segments = ARRAY_LENGTH(pattern);
}

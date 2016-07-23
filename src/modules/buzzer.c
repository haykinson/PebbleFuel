#include "buzzer.h"

void buzz_tank(Tank * tank) {
	if (NULL == tank || NULL == tank->vibePattern) {
		return;
	}

	if (!tank->notified) {
		//APP_LOG(APP_LOG_LEVEL_INFO, "*******BZZZZZZ*******");
		light_enable_interaction();
		vibes_enqueue_custom_pattern(*(tank->vibePattern));

		tank->notified = true;
	}
}
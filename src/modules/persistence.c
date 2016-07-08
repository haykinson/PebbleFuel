#include "persistence.h"

static const uint32_t CURRENT_VERSION = 1;
static const uint32_t MIN_VERSION = 1;

static uint32_t key_version = 0; //int; 1 or higher
static uint32_t key_num_tanks = 1; //int
static uint32_t key_interval = 2; //int; minutes
static uint32_t key_running = 3; //bool; true when running, false when paused
static uint32_t key_time_paused = 4; //int; timestamp when paused, or 0

static uint32_t key_base_for_offsets = 10000;
static uint32_t keyoffset_tank_selected = 0; //bool
static uint32_t keyoffset_tank_paused = 1; //bool
static uint32_t keyoffset_tank_initialized = 2; //bool
static uint32_t keyoffset_tank_started = 3; //int; timestamp when started
static uint32_t keyoffset_tank_elapsed = 4; //int; total time elapsed since started
static uint32_t keyoffset_tank_expires = 5; //int; timestamp when expires
static uint32_t keyoffset_tank_remaining = 6; //int; total time until expires

static PersistBaseDataV1 baseData;
static PersistTankV1 **tankData;
static uint32_t version = 0;
static bool version_ok = false;
static bool finished_read = false;

bool persistence_has_config() {
	if (persist_exists(key_version)) {
		version = persist_read_int(key_version);

		version_ok = (version >= MIN_VERSION && version <= CURRENT_VERSION);
		APP_LOG(APP_LOG_LEVEL_INFO, "Read version %ld, which is ok: %s", version, version_ok ? "yes" : "no");

		return version_ok;
	} else {
		APP_LOG(APP_LOG_LEVEL_INFO, "Persistence: version does not exist");
		return false;
	}
}

bool persistence_read_config() {
	if (!version_ok) {
		return false;
	}

	baseData.num_tanks = persist_read_int(key_num_tanks);
	baseData.interval = persist_read_int(key_interval);
	baseData.running = persist_read_bool(key_running);
	baseData.time_paused = (time_t) persist_read_int(key_time_paused);

	APP_LOG(APP_LOG_LEVEL_INFO, "Persistence: reading tanks");

	tankData = malloc(sizeof(PersistTankV1*) * baseData.num_tanks);
	for (int i = 0; i < (int) baseData.num_tanks; i++) {
		PersistTankV1 *tank = malloc(sizeof(PersistTankV1));
		tank->selected = persist_read_bool(key_base_for_offsets * (i+1) + keyoffset_tank_selected);
		tank->paused = persist_read_bool(key_base_for_offsets * (i+1) + keyoffset_tank_paused);
		tank->initialized = persist_read_bool(key_base_for_offsets * (i+1) + keyoffset_tank_initialized);
		tank->started = (time_t) persist_read_int(key_base_for_offsets * (i+1) + keyoffset_tank_started);
		tank->elapsed = (time_t) persist_read_int(key_base_for_offsets * (i+1) + keyoffset_tank_elapsed);
		tank->expires = (time_t) persist_read_int(key_base_for_offsets * (i+1) + keyoffset_tank_expires);
		tank->remaining = (time_t) persist_read_int(key_base_for_offsets * (i+1) + keyoffset_tank_remaining);

		tankData[i] = tank;
	}

	APP_LOG(APP_LOG_LEVEL_INFO, "Persistence: done reading tanks");

	finished_read = true;

	return true;
}

PersistBaseDataV1 *persistence_get_config() {
	if (!finished_read) {
		return NULL;
	}

	return &baseData;
}

PersistTankV1 **persistence_get_tank_config() {
	if (!finished_read) {
		return NULL;
	}

	return tankData;
}

void persistence_write_config(PersistBaseDataV1 *newConfig, PersistTankV1 **tankConfig) {
	if (NULL == newConfig || NULL == tankConfig) {
		return;
	}

	baseData = *newConfig;

	persist_write_int(key_version, CURRENT_VERSION);
	persist_write_int(key_num_tanks, baseData.num_tanks);
	persist_write_int(key_interval, baseData.interval);
	persist_write_bool(key_running, baseData.running);
	persist_write_int(key_time_paused, (uint32_t) baseData.time_paused);

	tankData = tankConfig;

	for (int i = 0; i < (int) baseData.num_tanks; i++) {
		PersistTankV1 *tank = tankConfig[i];
		persist_write_bool(key_base_for_offsets * (i+1) + keyoffset_tank_selected, tank->selected);
		persist_write_bool(key_base_for_offsets * (i+1) + keyoffset_tank_paused, tank->paused);
		persist_write_bool(key_base_for_offsets * (i+1) + keyoffset_tank_initialized, tank->initialized);
		persist_write_int(key_base_for_offsets * (i+1) + keyoffset_tank_started, tank->started);
		persist_write_int(key_base_for_offsets * (i+1) + keyoffset_tank_elapsed, tank->elapsed);
		persist_write_int(key_base_for_offsets * (i+1) + keyoffset_tank_expires, tank->expires);
		persist_write_int(key_base_for_offsets * (i+1) + keyoffset_tank_remaining, tank->remaining);
	}
}

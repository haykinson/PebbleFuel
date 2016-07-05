#pragma once

#include <pebble.h>
#include "tank.h"

typedef struct TankUI {
	Tank *tank;
	Layer *windowLayer;

	TextLayer *elapsedLayer;
	TextLayer *remainingLayer;
	char *elapsedBuffer;
	char *remainingBuffer;

	GColor enabledColor;
	GColor disabledColor;
	GRect location;
	GRect elapsedTextLocation;
	GRect remainingTextLocation;
} TankUI;

TankUI *tankui_create_with_tank(Tank *tank, Layer *windowLayer);
void tankui_update_elapsed_time(TankUI *tankui);
void tankui_update_remaining_time(TankUI *tankui);
void tankui_set_location(TankUI *tankui, GRect location);
void tankui_set_text_locations(TankUI *tankui, GRect elapsedTextLocation, GRect remainingTextLocation);
void tankui_set_colors(TankUI *tankui, GColor enabledColor, GColor disabledColor);
void tankui_destroy(TankUI *tankui);
void tankui_create_layers(TankUI *tankui);
void tankui_destroy_layers(TankUI *tankui);
void tankui_update_tick(Tank *tank, time_t tick, void *updateContext);
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
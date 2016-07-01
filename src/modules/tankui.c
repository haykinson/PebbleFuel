#include "tankui.h"

static void create_elapsed_text_layer(TankUI * tankui) {
	tankui->elapsedLayer = text_layer_create(tankui->elapsedTextLocation);
	text_layer_set_text(tankui->elapsedLayer, "00:00");
	text_layer_set_font(tankui->elapsedLayer, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));
	text_layer_set_text_alignment(tankui->elapsedLayer, GTextAlignmentRight);
	layer_add_child(tankui->windowLayer, text_layer_get_layer(tankui->elapsedLayer));
}

static void create_remaining_text_layer(TankUI *tankui) {
	tankui->remainingLayer = text_layer_create(tankui->remainingTextLocation);
	text_layer_set_text(tankui->remainingLayer, "00:00");
	text_layer_set_text_alignment(tankui->remainingLayer, GTextAlignmentRight);
	layer_set_hidden(text_layer_get_layer(tankui->remainingLayer), true);
	layer_add_child(tankui->windowLayer, text_layer_get_layer(tankui->remainingLayer));
}

static void tankui_create_layers(TankUI *tankui) {
	if (NULL != tankui) {
		create_elapsed_text_layer(tankui);
		create_remaining_text_layer(tankui);

		tankui_update_elapsed_time(tankui);
	}  
}

static void tankui_destroy_layers(TankUI *tankui) {
	if (NULL != tankui) {
		if (NULL != tankui->elapsedLayer) {
			text_layer_destroy(tankui->elapsedLayer);
		}
		if (NULL != tankui->remainingLayer) {
			text_layer_destroy(tankui->remainingLayer);
		}
	}
}

void tankui_set_colors(TankUI *tankui, GColor enabledColor, GColor disabledColor) {
	if (NULL != tankui) {
		tankui->enabledColor = enabledColor;
		tankui->disabledColor = disabledColor;
	}
}

void tankui_set_location(TankUI *tankui, GRect location) {
	if (NULL != tankui) {
		tankui->location = location;
	}
}

void tankui_set_text_locations(TankUI *tankui, GRect elapsedTextLocation, GRect remainingTextLocation) {
	if (NULL != tankui) {
		tankui->elapsedTextLocation = elapsedTextLocation;
		tankui->remainingTextLocation = remainingTextLocation;
	}
}

void tankui_update_elapsed_time(TankUI *tankui) {
	if (tankui->tank->startTime != 0) {
		text_layer_set_text(tankui->elapsedLayer, format_seconds(tankui->tank->lastDiff, tankui->elapsedBuffer));
	}
}

TankUI *tankui_create_with_tank(Tank *tank, Layer *windowLayer) {
	TankUI *tankui = (TankUI *) malloc(sizeof(TankUI));
	tankui->tank = tank;
	tankui->windowLayer = windowLayer;

	tankui->elapsedBuffer = (char *) calloc(MAX_TIME_TEXT_LEN, sizeof(char));
	tankui->remainingBuffer = (char *) calloc(MAX_TIME_TEXT_LEN, sizeof(char));


	tankui_create_layers(tankui);

	return tankui;
}

void tankui_destroy(TankUI *tankui) {
	//does not free the tank that it holds a reference to

	if (NULL != tankui) {
		tankui_destroy_layers(tankui);

		if (NULL != tankui->elapsedBuffer) {
			free(tankui->elapsedBuffer);
		}
		if (NULL != tankui->remainingBuffer) {
			free(tankui->remainingBuffer);
		}

		free(tankui);
	}
}
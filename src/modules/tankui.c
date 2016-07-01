#include "tankui.h"

static void create_elapsed_text_layer(TankUI * tankui, GRect where) {
	tankui->elapsedLayer = text_layer_create(where);
	text_layer_set_text(tankui->elapsedLayer, "00:00");
	text_layer_set_font(tankui->elapsedLayer, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));
	text_layer_set_text_alignment(tankui->elapsedLayer, GTextAlignmentRight);
	layer_add_child(tankui->windowLayer, text_layer_get_layer(tankui->elapsedLayer));
}

static void create_remaining_text_layer(TankUI *tankui, GRect where) {
	tankui->remainingLayer = text_layer_create(where);
	text_layer_set_text(tankui->remainingLayer, "00:00");
	text_layer_set_text_alignment(tankui->remainingLayer, GTextAlignmentRight);
	layer_set_hidden(text_layer_get_layer(tankui->remainingLayer), true);
	layer_add_child(tankui->windowLayer, text_layer_get_layer(tankui->remainingLayer));
}

static void tankui_create_layers(TankUI *tankui) {
	if (NULL != tankui) {
		create_elapsed_text_layer(tankui, tank->elapsedTextLocation);
		create_remaining_text_layer(tankui, tank->remainingTextLocation);

		if (tank->startTime != 0) {
			text_layer_set_text(tank->elapsed, format_seconds(tank->lastDiff, tank->elapsedBuffer));
		}
	}  
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


TankUI *tankui_create_with_tank(Tank *tank, Window *windowLayer) {
	TankUI *tankui = (TankUI *) malloc(sizeof(TankUI));
	tankui->tank = tank;
	tankui->windowLayer = windowLayer;

	tankui_create_layers(tankui);

	return tankui;
}

void tankui_destroy(TankUI *tankui) {
	//does not free the tank that it holds a reference to

	if (NULL != tankui) {
		free(tankui);
	}
}
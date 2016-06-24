#pragma once

#include <pebble.h>

char *text_for_minutes();
void update_interval_text();
void selection_select_click_handler(ClickRecognizerRef recognizer, void *context);
void selection_up_click_handler(ClickRecognizerRef recognizer, void *context);
void selection_down_click_handler(ClickRecognizerRef recognizer, void *context);
void selection_click_config_provider(void *context);
void selection_window_load(Window *window);
void selection_window_unload(Window *window);
void selection_init_vars();
void selection_deinit_vars();
int get_interval();
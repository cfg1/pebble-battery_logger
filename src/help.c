#include <pebble.h>
#include "help.h"

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_gothic_14;
static TextLayer *s_textlayer_1;
static ScrollLayer *s_scroll_layer_1;

static char help_text[600] = "App to log all battery events and calculate the battery lifetime out of this. The app uses an AppWorker in the background to save all events on the battery to persistent storage. The UI displays the logs from persistent storage.\n\nEvents:\n"
                             " 0: nothing changed\n"
                             " 1: start of discharging\n"
                             " 2: start of charging\n"
                             " 3: battery full\n"
                             " 4: percent changed\n\n"
                             "First time is since state change, second time in brackets [] is time since last event.\n\n"
                             "Background worker launch codes:\n"
                             " 0: OK\n"
                             " 1: No worker available\n"
                             " 4: Already running\n"
                             " 5: User has to decide to run it\n";

static void initialise_ui(void) {
  s_window = window_create();
  Layer *window_layer = window_get_root_layer(s_window);
  #ifndef PBL_SDK_3
    //window_set_fullscreen(s_window, true);
  #endif
  
  s_res_gothic_14 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  
  
  GRect bounds = layer_get_bounds(window_layer);
  GRect max_text_bounds = GRect(0, 0, bounds.size.w, 2000);
  s_scroll_layer_1 = scroll_layer_create(bounds);
  scroll_layer_set_click_config_onto_window(s_scroll_layer_1, s_window);
  
  // s_textlayer_1
  s_textlayer_1 = text_layer_create(max_text_bounds);
  text_layer_set_background_color(s_textlayer_1, GColorWhite);
  text_layer_set_text_color(s_textlayer_1, GColorBlack);
  text_layer_set_text(s_textlayer_1, help_text); //help_text);
  text_layer_set_font(s_textlayer_1, s_res_gothic_14);
  //layer_add_child(window_layer, (Layer *)s_textlayer_1);
  
  
  scroll_layer_add_child(s_scroll_layer_1, text_layer_get_layer(s_textlayer_1));
  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer_1));
  
  
  
  GSize max_size = text_layer_get_content_size(s_textlayer_1);
  text_layer_set_size(s_textlayer_1, max_size);
  scroll_layer_set_content_size(s_scroll_layer_1, GSize(bounds.size.w, max_size.h + 4));
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(s_textlayer_1);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_help(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_help(void) {
  window_stack_remove(s_window, true);
}

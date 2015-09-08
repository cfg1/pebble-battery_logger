#include "main.h"
#include "help.h"

static Window *window;
static TextLayer *text_layer;
static ScrollLayer *scroll_layer;
static TextLayer *status_bar_layer;

#define STATUS_HIGH 15

#define TEXT_LENGTH 7000
static char text[TEXT_LENGTH] = "No data loaded.";
static char text_buffer[TEXT_LENGTH];
//static char text[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam quam tellus, fermentu  m quis vulputate quis, vestibulum interdum sapien. Vestibulum lobortis pellentesque pretium. Quisque ultricies purus e  u orci convallis lacinia. Cras a urna mi. Donec convallis ante id dui dapibus nec ullamcorper erat egestas. Aenean a m  auris a sapien commodo lacinia. Sed posuere mi vel risus congue ornare. Curabitur leo nisi, euismod ut pellentesque se  d, suscipit sit amet lorem. Aliquam eget sem vitae sem aliquam ornare. In sem sapien, imperdiet eget pharetra a, lacin  ia ac justo. Suspendisse at ante nec felis facilisis eleifend.";
static char text_header[200] = " ";
static char text_devider[] =  " ------------------------- \n";
static char time_str[25];
static char time1_str[25];
static char time2_str[25];

static time_t period_begin = 0;
static time_t last_event = 0;

//char help_text[1000] = "This is the help window ...";
  
static GRect bounds, max_text_bounds;

static AppWorkerResult AWresult;


void print_time(char *s, int size_s, time_t time_diff, int mode){
  
  //mode = 0: 
  //    7 s
  // 1:15 m
  //   10 m
  // 1:05 h
  //   10 h
  // 1:03 d
  //   10 d
  // 3000 d
  
  //mode == 1: 
  //    7 s
  //    1 m
  //   10 m
  //    1 h
  //   10 h
  //    1 d
  //   10 d
  // 3000 d
  
  //mode == 2:
  // 00 d 00:00:00 h
  
  int days    = time_diff / (24*3600);
  int hours   = (time_diff % (24*3600)) / 3600;
  int minutes = (time_diff % 3600) / 60;
  int seconds = (time_diff % 60);

  if (mode == 2){
    snprintf(s, size_s, "%02d d, %02d:%02d:%02d h", days, hours, minutes, seconds);
  } else {
    if (days == 0){
      if (hours == 0){
        if (minutes == 0){
          snprintf(s, size_s, "%d s", seconds);
        } else {
          if ((minutes < 10) && (mode == 0))
            snprintf(s, size_s, "%d:%02d m", minutes, seconds);
          else
            snprintf(s, size_s, "%d m", minutes);
        }
      } else {
        if ((hours < 10)  && (mode == 0))
          snprintf(s, size_s, "%d:%02d h", hours, minutes);
        else
          snprintf(s, size_s, "%d h", hours);
      }
    } else {
      if ((days < 10) && (mode == 0))
        snprintf(s, size_s, "%d:%02d d", days, hours);
      else
        snprintf(s, size_s, "%d d", days);
    }
  }
}



// load from beginning to end (to calculate runtimes) and put the text allways onto the top:
static void load_data(void){
  time_t time_stamp = 0;
  time_t period_since_begin = 0;
  time_t period_since_last_event = 0;
  
  if (app_worker_is_running()){
    snprintf(text_header, sizeof(text_header), "Worker is running\nLaunch result = %d\n%s", (int)AWresult, text_devider);
  } else {
    snprintf(text_header, sizeof(text_header), "Worker is NOT running\nLaunch result = %d\n%s", (int)AWresult, text_devider);
  }
  strcpy(text, "");
  //snprintf(text, TEXT_LENGTH, "%s", text_header);
  
  int read_pos = -1, write_pos = -1;
  int key = KEY_CURRENT_INDEX;
  int number_of_entries_read = 0;
  if (persist_exists(key)){
    read_pos = persist_read_int(key);
    write_pos = read_pos; //save which was the write_pos from the worker
    while (1){
      
      read_pos++;
      if (read_pos > NUMBER_OF_ENTRIES) read_pos = 0;
      if (read_pos < 0) read_pos = NUMBER_OF_ENTRIES-1;
      
      key = KEY_EVENT_STRING_001+read_pos;
      if (persist_exists(key)){
        uint8_t byte_array[3];
        persist_read_data(key, byte_array, sizeof(byte_array));
        
        //read time stamp:
        key = KEY_EVENT_TIME_STAMP_001+read_pos;
        if (persist_exists(key)){
          time_stamp = (time_t)persist_read_int(key);
          struct tm* loctime = localtime(&time_stamp);
          strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H:%M:%S:\n", loctime);
          number_of_entries_read++;
        } else {
          snprintf(time_str, sizeof(time_str), "no time stamp found:\n");
          time_stamp = 0;
        }
        switch (byte_array[0]){ //message
          case 1: 
          case 2:
          case 3:
            period_begin = time_stamp;
            strcpy(text_buffer, text);
            snprintf(text, sizeof(text), "%s%s", text_devider, text_buffer);
            break;
        }
        period_since_begin = time_stamp - period_begin;
        period_since_last_event = time_stamp - last_event;
        if (byte_array[0] > 0) last_event = time_stamp;
        
        print_time(time1_str, sizeof(time1_str), period_since_begin, 0);
        print_time(time2_str, sizeof(time2_str), period_since_last_event, 0);
        
        strcpy(text_buffer, text);
        snprintf(text, TEXT_LENGTH, "%s    %d: %d %% --> %d %%\n    %s [%s]\n%s", time_str, byte_array[0], byte_array[2], byte_array[1], time1_str, time2_str, text_buffer);
      }
      
      if (read_pos == write_pos) break;
      if (number_of_entries_read > NUMBER_OF_ENTRIES_TO_SHOW) break;
    }
    
    strcpy(text_buffer, text);
    snprintf(text, TEXT_LENGTH, "%s%s\n", text_header, text_buffer);
  } else {
    snprintf(text, TEXT_LENGTH, "%sNo data found.\n", text_header);
  }
  
  text_layer_set_size(text_layer, GSize(bounds.size.w, 2000));
  text_layer_set_text(text_layer, text); 
  GSize max_size = text_layer_get_content_size(text_layer);
  text_layer_set_size(text_layer, max_size);
  scroll_layer_set_content_size(scroll_layer, GSize(bounds.size.w, max_size.h + 4 + STATUS_HIGH));
}

/* reverse loading:
static void load_data(void){
  if (app_worker_is_running()){
    snprintf(text, sizeof(text), "Background-Worker \n  is running\nLaunch result = %d\n ---------------------- \n", (int)AWresult);
  } else {
    snprintf(text, sizeof(text), "Background-Worker \n  is NOT running\nLaunch result = %d\n ---------------------- \n", (int)AWresult);
  }
  
  int read_pos = -1, write_pos = -1;
  int key = KEY_CURRENT_INDEX;
  if (persist_exists(key)){
    read_pos = persist_read_int(key);
    write_pos = read_pos; //save which was the write_pos from the worker
    while (1){
      key = KEY_EVENT_STRING_001+read_pos;
      if (persist_exists(key)){
        uint8_t byte_array[3];
        persist_read_data(key, byte_array, sizeof(byte_array));
        
        //read time stamp:
        key = KEY_EVENT_TIME_STAMP_001+read_pos;
        if (persist_exists(key)){
          time_t time_stamp = (time_t)persist_read_int(key);
          struct tm* loctime = localtime(&time_stamp);
          strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H:%M:%S:\n", loctime);
        } else {
          snprintf(time_str, sizeof(time_str), "no time stamp found:\n");
        }
        snprintf(text, sizeof(text), "%s%s   %d: %d %% --> %d %%\n", text, time_str, byte_array[0], byte_array[2], byte_array[1]);
      }
      
      read_pos--;
      if (read_pos > NUMBER_OF_ENTRIES) read_pos = 0;
      if (read_pos < 0) read_pos = NUMBER_OF_ENTRIES-1;
      
      if (read_pos == write_pos) break;
    }
    
  } else {
    snprintf(text, sizeof(text), "%sNo data found.\n", text);
  }
  
  text_layer_set_size(text_layer, GSize(bounds.size.w, 2000));
  text_layer_set_text(text_layer, text); 
  GSize max_size = text_layer_get_content_size(text_layer);
  text_layer_set_size(text_layer, max_size);
  scroll_layer_set_content_size(scroll_layer, GSize(bounds.size.w, max_size.h + 4));
}
*/

static void timer_callback(void *data) {
  load_data();
}

static void handle_battery(BatteryChargeState charge_state){
  //register a one shot timer to reload data:
  app_timer_register(500, timer_callback, NULL); //ms
}




static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Select");
  //create a new window with help text:
  show_help();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Down");
}

void scroll_config_provider(void *context) { //sets select button to ScrollLayer
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void click_config_provider(void *context) { //sets all clicks to window
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void handle_second_tick(struct tm* current_time, TimeUnits units_changed){
  time_t now = time(NULL);
  time_t time_since_last_event = now - last_event;
  time_t time_since_last_state = now - period_begin;
  
  static char time_string_1[25];
  static char time_string_2[25];
  static char status_string[100];
  print_time(time_string_1, sizeof(time_string_1), time_since_last_state, 2);
  print_time(time_string_2, sizeof(time_string_2), time_since_last_event, 0);
  snprintf(status_string, sizeof(status_string), "%s [%s]", time_string_1, time_string_2);
  text_layer_set_text(status_bar_layer, status_string);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);
  max_text_bounds = GRect(0, 0, bounds.size.w, 2000);
  
  scroll_layer = scroll_layer_create(bounds);
  scroll_layer_set_click_config_onto_window(scroll_layer, window);
  
  /*ScrollLayerCallbacks cbacks;
  cbacks.click_config_provider = &scroll_config_provider;*/
  scroll_layer_set_callbacks(scroll_layer, (ScrollLayerCallbacks){.click_config_provider=scroll_config_provider});
  

  text_layer = text_layer_create(max_text_bounds);
  text_layer_set_text(text_layer, text); 
  text_layer_set_text_alignment(text_layer, GTextAlignmentLeft);
  text_layer_set_text_color(text_layer, GColorBlack);
  text_layer_set_background_color(text_layer, GColorWhite);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
  
  GSize max_size = text_layer_get_content_size(text_layer);
  text_layer_set_size(text_layer, max_size);
  scroll_layer_set_content_size(scroll_layer, GSize(bounds.size.w, max_size.h + 4 + STATUS_HIGH));
  
  
  scroll_layer_add_child(scroll_layer, text_layer_get_layer(text_layer));
  
  layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));
  
  status_bar_layer = text_layer_create(GRect(0, bounds.size.h-STATUS_HIGH, 144, STATUS_HIGH));
  text_layer_set_text_alignment(status_bar_layer, GTextAlignmentCenter);
  text_layer_set_text_color(status_bar_layer, GColorWhite);
  text_layer_set_background_color(status_bar_layer, GColorBlack);
  text_layer_set_font(status_bar_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(status_bar_layer));
}

static void window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  text_layer_destroy(status_bar_layer);
  text_layer_destroy(text_layer);
  scroll_layer_destroy(scroll_layer);
}

static void init(void) {
  window = window_create();
  //window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
	  .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
  
  AWresult = app_worker_launch();
  load_data();
  battery_state_service_subscribe(&handle_battery);
  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
}

static void deinit(void) {
  battery_state_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

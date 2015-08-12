#include <pebble_worker.h>
#include "background_worker.h"
  
//static char info_string[30];
  
static BatteryChargeState old_charge_state;
  
static void handle_battery(BatteryChargeState charge_state){
  //charge_state_int = 0: Not plugged (discharging)
  //charge_state_int = 1: plugged and charging
  //charge_state_int = 2: plugged and full
  
  
  int message = 0;
  
  time_t now = time(NULL);
  
  int old_charge_state_int = 0; //does not have to be static, is calculated from old_charge_state
  int charge_state_int = 0;
  
  
  
  if (old_charge_state.is_plugged){
    if (old_charge_state.is_charging) old_charge_state_int = 1; else old_charge_state_int = 2;
  } else old_charge_state_int = 0; // = actual charge state
  
  if (charge_state.is_plugged){
    if (charge_state.is_charging) charge_state_int = 1; else charge_state_int = 2;
  } else charge_state_int = 0; // = actual charge state
  
  
  
  if ((old_charge_state_int > 0) && (charge_state_int == 0)){ //start discharging
    message = 1;
  } else if ((old_charge_state_int == 0) && (charge_state_int == 1)){ //start charging (after discharging)
    message = 2;
  } else if ((old_charge_state_int < 2) && (charge_state_int == 2)){ //start of full (and plugged)
    message = 3;
  } else if (charge_state.charge_percent != old_charge_state.charge_percent){
    message = 4; //change of battery percent value
  } else {
    message = 0; // no reason detected
  }
    
  if (message){
    
    int write_pos = 0;
    int key = KEY_CURRENT_INDEX;
    if (persist_exists(key)) write_pos = persist_read_int(key);
    write_pos++;
    if (write_pos > NUMBER_OF_ENTRIES) write_pos = 0;
    if (write_pos < 0) write_pos = 0;
    persist_write_int(key, write_pos);
    
    persist_write_int(KEY_EVENT_TIME_STAMP_001+write_pos, (int)now);
    //snprintf(info_string, sizeof(info_string), "m = %d: %d %% --> %d %%", message, charge_state.charge_percent, old_charge_state.charge_percent);
    //persist_write_string(KEY_EVENT_STRING_001+ write_pos, info_string);
    //there is a bug in persist_write_string so that the worker crashes on Aplite with this command. Using a byte array:
    uint8_t byte_array[3];
    byte_array[0] = (uint8_t)message;
    byte_array[1] = (uint8_t)charge_state.charge_percent;
    byte_array[2] = (uint8_t)old_charge_state.charge_percent;
    persist_write_data(KEY_EVENT_STRING_001+write_pos, byte_array, sizeof(byte_array));
  }
  
  
  
  old_charge_state = charge_state;
}

static void init() {
  // Initialize your worker here
  
  old_charge_state = battery_state_service_peek();
  handle_battery(old_charge_state); //init battery info
  battery_state_service_subscribe(&handle_battery);
  
}

static void deinit() {
  // Deinitialize your worker here
  battery_state_service_unsubscribe();
}

int main(void) {
  init();
  worker_event_loop();
  deinit();
}
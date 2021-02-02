#include "pebble.h"
#include "math.h"
#include "main.h"
#include "main_screen.h"
#include "summary_screen.h"

int num_meters;
time_t resume_time;
uint16_t resume_time_ms;

// declare state structure
TimerState state;

// declare config structure
AppConfig config;

// Matching values to those in 'Settings'
typedef enum {
  AppKeyDistUnit,
  AppKeyLapDist,
  AppKeyDispMode,
  AppKeyColorMain,
  AppKeyColorAccent,
  AppKeyBackButtonLongPress
} AppKey;

// store app settings in persistent storage
static void store_config(void){
  persist_write_data(CONFIG_PKEY, &config, sizeof(config));
}

// executed when app message is received (i.e. from saving app settings on phone)
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO,"READING CONFIG INPUTS");
  
  // Assign all parameters to appropriate config structure fields
  Tuple *dist_unit = dict_find(iter, AppKeyDistUnit);
  if(dist_unit) {
    config.dist_unit = dist_unit->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO,"dist_unit = %u",config.dist_unit);
  }
  Tuple *lap_dist = dict_find(iter, AppKeyLapDist);
  if(lap_dist) {
    config.lap_dist = lap_dist->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO,"lap_dist = %u",(int)(lap_dist->value->int32));
  }
  Tuple *disp_mode = dict_find(iter, AppKeyDispMode);
  if(disp_mode) {
    config.disp_mode = disp_mode->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO,"disp_mode = %u",config.disp_mode);
  }
  Tuple *main_color = dict_find(iter, AppKeyColorMain);
  if(disp_mode) {
    config.main_color = main_color->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO,"main_color = %06x",config.main_color);
  }
  Tuple *accent_color = dict_find(iter, AppKeyColorAccent);
  if(disp_mode) {
    config.accent_color = accent_color->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO,"accent_color = %06x",config.accent_color);
  }
  Tuple *back_button_long_press = dict_find(iter, AppKeyBackButtonLongPress);
  if(back_button_long_press) {
    config.back_button_long_press = back_button_long_press->value->int32 > 0;
    APP_LOG(APP_LOG_LEVEL_INFO,"back_button_long_press = %u",config.back_button_long_press);
  }

  // App should now update to take the user's preferences into account
  store_config();
  reset_current_run();
}

// initialize app
static void init(void) {  
  // Register to be notified about inbox received events
  app_message_register_inbox_received((AppMessageInboxReceived) inbox_received_handler);
  app_message_open(64,64);
  
  // if timer state exists in persistent storage, load; otherwise, initialize with defaults
  // Since the run timer uses epoch time, it can run "in the background" without losing time
  if(persist_exists(STATE_PKEY)){
    persist_read_data(STATE_PKEY, &state, sizeof(state));
  }else{
    state = (TimerState){
      .paused = true,
      .started = false,
      .actionbar_toggled = false,
      .num_laps = 0,
      .num_dist = 0,
      .last_elapsed_time_ms = 0,
      .lap_start_time_ms = 0,
      .previous_lap_ms = 0,
      .best_lap_ms = 1000*SECONDS_PER_HOUR*5,
      .worst_lap_ms = 0
    };
  }
  
  // if config exists in persistent storage, load; otherwise, initialize with defaults
  if(persist_exists(CONFIG_PKEY)){
    persist_read_data(CONFIG_PKEY, &config, sizeof(config));
  }else{
    config = (AppConfig){
      .dist_unit = 0,
      .disp_mode = 0,
      .lap_dist = 162,
      .main_color = 0xFFFFFF,
      .accent_color = 0x000000,
      .back_button_long_press = false
    };
  }
  
  // load main window
  show_main_screen();
  
  // set up tick timer to update clock time
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

// called when app closes
static void deinit(void) {
  // write data to persistent storage
  persist_write_data(STATE_PKEY, &state, sizeof(state));
  persist_write_data(CONFIG_PKEY, &config, sizeof(config));
  
  // close the screen
  hide_main_screen();
}

// main app function
int main(void) {
  init();
  app_event_loop();
  deinit();
}
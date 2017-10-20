#include <pebble.h>
#include "main_screen.h"
#include "main.h"
#include "summary_screen.h"

// declare global values
GColor main_gcolor;
GColor accent_gcolor;

static Window *s_window;

static GBitmap *s_res_image_action_icon_reset;
static GBitmap *s_res_image_action_icon_play_pause;
static GBitmap *s_res_image_action_icon_increment;
static GBitmap *s_res_image_action_icon_finish;
static GBitmap *s_res_image_action_icon_play;
static GBitmap *s_res_image_action_icon_pause;

static GFont s_res_distFont;
static GFont s_res_timerFont_large;
static GFont s_res_timerFont_small;
static GFont s_res_timeFont;
static GFont s_res_statFont;

static ActionBarLayer *s_actionbarlayer_1;

static BitmapLayer *s_bitmaplayer_bar_1;
static BitmapLayer *s_bitmaplayer_bar_2;
static BitmapLayer *s_bitmaplayer_bar_3;
static BitmapLayer *s_bitmaplayer_bar_4;
static BitmapLayer *s_bitmaplayer_titlebox_1;
static BitmapLayer *s_bitmaplayer_titlebox_2;
static BitmapLayer *s_bitmaplayer_unitbox_1;
static BitmapLayer *s_bitmaplayer_unitbox_2;

static TextLayer *s_textlayer_laps_num;
static TextLayer *s_textlayer_laps_title;
static TextLayer *s_textlayer_dist_unit;
static TextLayer *s_textlayer_dist_num;
static TextLayer *s_textlayer_dist_title;
static TextLayer *s_textlayer_timer;
static TextLayer *s_textlayer_time;
static TextLayer *s_textlayer_stats;

// position info
static uint8_t s_titleHeight = 25;
static uint8_t s_boxWidth_lap = 41;
static uint8_t s_boxWidth_dist = 69;
static uint8_t s_boxPos_y = 95;
static uint8_t s_boxPos_x1 = 1;
static uint8_t s_boxPos_x2 = 44;

static uint8_t s_barPos_x = 42;
static uint8_t s_barPos_y = 120;
static uint8_t s_barWidth = 2;
static uint8_t s_barHeight = 31;

static uint8_t s_unitboxHeight = 17;
static uint8_t s_unitPos_y = 150;
static uint8_t s_digitPos_y = 118;
static uint8_t s_digitPos_x1 = 0;
static uint8_t s_digitPos_x2 = 43;
static uint8_t s_digitHeight = 40;

static uint8_t s_statsPos_x = 5;
static uint8_t s_statsPos_y = 47;
static uint8_t s_statsWidth = 104;
static uint8_t s_statsHeight = 50;

// app timer delay (refresh period in ms)
const int delta = 100;

// time variables used for run timing
static time_t s_current_time;
static uint16_t s_current_time_ms = 0;

// pointer to timer object
AppTimer *timer;

// callback for up button
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  // depending on what actionbar icon is shown, either increment or reset
  if(state.actionbar_toggled){
    reset_current_run();
  }else{
    if(!state.paused){
      increment_lap_count();
    }
  }
}

// callback for down button
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
 // currently not used
}

// callback for select button
static void select_click_handler(ClickRecognizerRef recognizer, void*context){
  if(state.actionbar_toggled){
    finish_current_run();
  }else{
    if(state.started){
      // timer started: play/pause
      if(state.paused){
        resume_current_run();
      }else{
        pause_current_run();        
      }
    }else{
      // ready to start a new timer
      start_new_run();
    }
  }  
}

// callback for long press of select button
static void toggle_click_handler(ClickRecognizerRef recognizer, void *context){
  state.actionbar_toggled = !state.actionbar_toggled;
  update_action_bar();
}

// assigns callbacks to each button click type
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) down_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT,0, (ClickHandler)toggle_click_handler,NULL);
  window_single_click_subscribe(BUTTON_ID_SELECT,(ClickHandler)select_click_handler);
}

// called every time the minute changes
void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_wall_time(); 
}

// executes when app timer is called
void timer_callback(void *data){
  // only doing anything when not paused
  if(!state.paused){
    // get current time
    time_ms(&s_current_time,&s_current_time_ms);
    
    // determine how many ms have passed since the last resume
    int delta_time_ms = (s_current_time - state.resume_time)*1000 + (s_current_time_ms - state.resume_time_ms);
    
    // update the total elapsed time and current lap time
    state.elapsed_time_ms = state.last_elapsed_time_ms + delta_time_ms;
    state.elapsed_lap_time_ms = state.elapsed_time_ms - state.lap_start_time_ms;
    
    // update the screen to show times
    update_timer_display();
  }
  // set timer to execute again after set delay
  timer = app_timer_register(delta, (AppTimerCallback) timer_callback, NULL);
}

// creates all layers in this window and allocates heap memory
static void initialise_ui(void) {
  s_window = window_create();
  
  s_res_image_action_icon_reset = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_RESET);
  s_res_image_action_icon_play_pause = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_PLAY_PAUSE);
  s_res_image_action_icon_play = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_PLAY);
  s_res_image_action_icon_pause = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_PAUSE);
  s_res_image_action_icon_increment = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_INCREMENT);
  s_res_image_action_icon_finish = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_FINISH);
  
  s_res_distFont = fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM );
  s_res_timerFont_large = fonts_get_system_font(FONT_KEY_LECO_36_BOLD_NUMBERS); 
  s_res_timerFont_small = fonts_get_system_font(FONT_KEY_LECO_28_LIGHT_NUMBERS);
  s_res_timeFont = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_res_statFont = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS );
  
  // s_actionbarlayer_1
  s_actionbarlayer_1 = action_bar_layer_create();
  action_bar_layer_add_to_window(s_actionbarlayer_1, s_window);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_SELECT, s_res_image_action_icon_play_pause);
  action_bar_layer_set_click_config_provider(s_actionbarlayer_1, click_config_provider);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_actionbarlayer_1);
   
  // s_bitmaplayer_bar_1_bar bar 1
  s_bitmaplayer_bar_1 = bitmap_layer_create(GRect(s_barPos_x, s_barPos_y, s_barWidth, s_barHeight));
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_bitmaplayer_bar_1);
  
  // s_bitmaplayer_titlebox_1
  s_bitmaplayer_titlebox_1 = bitmap_layer_create(GRect(s_boxPos_x1, s_boxPos_y, s_boxWidth_lap, s_titleHeight));
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_bitmaplayer_titlebox_1);
  
  // s_bitmaplayer_titlebox_2
  s_bitmaplayer_titlebox_2 = bitmap_layer_create(GRect(s_boxPos_x2, s_boxPos_y, s_boxWidth_dist, s_titleHeight));
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_bitmaplayer_titlebox_2);

  // s_bitmaplayer_unitbox_1
  s_bitmaplayer_unitbox_1 = bitmap_layer_create(GRect(s_boxPos_x1, s_unitPos_y, s_boxWidth_lap, s_unitboxHeight));
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_bitmaplayer_unitbox_1);
  
  // s_bitmaplayer_unitbox_2
  s_bitmaplayer_unitbox_2 = bitmap_layer_create(GRect(s_boxPos_x2, s_unitPos_y, s_boxWidth_dist, s_unitboxHeight));
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_bitmaplayer_unitbox_2);

  // s_textlayer_laps_num
  s_textlayer_laps_num = text_layer_create(GRect(s_digitPos_x1, s_digitPos_y, s_boxWidth_lap, s_digitHeight));
  text_layer_set_background_color(s_textlayer_laps_num, GColorClear);
  text_layer_set_text_alignment(s_textlayer_laps_num, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_laps_num, s_res_distFont);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_laps_num);
  
  // s_textlayer_laps_title
  s_textlayer_laps_title = text_layer_create(GRect(s_boxPos_x1, s_boxPos_y, s_boxWidth_lap, s_titleHeight));
  text_layer_set_background_color(s_textlayer_laps_title, GColorClear);
  text_layer_set_text(s_textlayer_laps_title, "Laps");
  text_layer_set_text_alignment(s_textlayer_laps_title, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_laps_title);
  
  // s_textlayer_dist_unit
  s_textlayer_dist_unit = text_layer_create(GRect(s_boxPos_x2, s_unitPos_y-2, s_boxWidth_dist, s_unitboxHeight));
  text_layer_set_background_color(s_textlayer_dist_unit, GColorClear);
  text_layer_set_text_alignment(s_textlayer_dist_unit, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_dist_unit);
  
  // s_textlayer_dist_num
  s_textlayer_dist_num = text_layer_create(GRect(s_digitPos_x2, s_digitPos_y, s_boxWidth_dist+4, s_digitHeight));
  text_layer_set_background_color(s_textlayer_dist_num, GColorClear);
  text_layer_set_text_alignment(s_textlayer_dist_num, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_dist_num, s_res_distFont);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_dist_num);
  
  // s_textlayer_dist_title
  s_textlayer_dist_title = text_layer_create(GRect(s_boxPos_x2, s_boxPos_y, s_boxWidth_dist+2, s_titleHeight));
  text_layer_set_background_color(s_textlayer_dist_title, GColorClear);
  text_layer_set_text(s_textlayer_dist_title, "Distance");
  text_layer_set_text_alignment(s_textlayer_dist_title, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_dist_title);
  
  // s_textlayer_time
  s_textlayer_time = text_layer_create(GRect(0, -8, 114, 50));
  text_layer_set_background_color(s_textlayer_time, GColorClear);
  text_layer_set_text_alignment(s_textlayer_time, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_time, s_res_timeFont);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_time);
  
  // s_textlayer_timer
  s_textlayer_timer = text_layer_create(GRect(0, 8, 114, 50));
  text_layer_set_background_color(s_textlayer_timer, GColorClear);
  text_layer_set_text_alignment(s_textlayer_timer, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_timer, s_res_timerFont_large);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_timer);
  
  // s_textlayer_stats
  s_textlayer_stats = text_layer_create(GRect(s_statsPos_x, s_statsPos_y, s_statsWidth, s_statsHeight));
  text_layer_set_background_color(s_textlayer_stats, GColorClear);
  text_layer_set_text_alignment(s_textlayer_stats, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_stats, s_res_statFont);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_stats);
  
  // update screen text and color scheme
  update_main_screen();
}

// frees memory from the heap
static void destroy_ui(void) {
  window_destroy(s_window);
  action_bar_layer_destroy(s_actionbarlayer_1);
  bitmap_layer_destroy(s_bitmaplayer_bar_1);
  bitmap_layer_destroy(s_bitmaplayer_titlebox_1);
  bitmap_layer_destroy(s_bitmaplayer_titlebox_2);
  bitmap_layer_destroy(s_bitmaplayer_unitbox_1);
  bitmap_layer_destroy(s_bitmaplayer_unitbox_2);
  
  text_layer_destroy(s_textlayer_laps_num);
  text_layer_destroy(s_textlayer_laps_title);
  text_layer_destroy(s_textlayer_dist_unit);
  text_layer_destroy(s_textlayer_dist_num);
  text_layer_destroy(s_textlayer_dist_title);
  text_layer_destroy(s_textlayer_time);
  text_layer_destroy(s_textlayer_timer);
  text_layer_destroy(s_textlayer_stats);
  
  gbitmap_destroy(s_res_image_action_icon_reset);
  gbitmap_destroy(s_res_image_action_icon_play_pause);
  gbitmap_destroy(s_res_image_action_icon_increment);
  gbitmap_destroy(s_res_image_action_icon_finish);
  gbitmap_destroy(s_res_image_action_icon_play);
  gbitmap_destroy(s_res_image_action_icon_pause);
}

// called when leaving the app
static void handle_window_unload(Window* window) {
  app_timer_cancel(timer);
  destroy_ui();
}

// called when the window is brought to the foreground (appears)
static void handle_window_update(Window* window){
  update_main_screen();
}

// initialize interface, register timer
void show_main_screen(void) {
  initialise_ui();
  timer = app_timer_register(delta, (AppTimerCallback) timer_callback, NULL);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
    .appear = handle_window_update
  });
  window_stack_push(s_window, true);
}

// remove window from stack
void hide_main_screen(void) {
  window_stack_remove(s_window, true);
}

// update number of laps and distance with proper unit
void update_distance_display(void){
  // preallocate char arrays
  static char dist_text[6];
  static char laps_text[3];
  
  // set lap number
  snprintf(laps_text,sizeof(laps_text),"%u",state.num_laps);
  
  // set distance number and unit
  if(config.dist_unit == 0){
    // metric unit
    if(state.num_dist < METERS_PER_KM){
      // under a km - display meters
      snprintf(dist_text,sizeof(dist_text),"%u",state.num_dist);
      text_layer_set_text(s_textlayer_dist_unit,"meters");
    }else{
      // over a km - display km
      int km = state.num_dist/METERS_PER_KM;
      int firstDecimal = (state.num_dist%1000)/100;
      int secondDecimal = (state.num_dist%100)/10;
      snprintf(dist_text,sizeof(dist_text),"%u.%u%u", km,firstDecimal,secondDecimal);
      text_layer_set_text(s_textlayer_dist_unit,"km");
    }
  }else{
    // imperial unit
    if(state.num_dist < YARDS_PER_MILE){
      // under a mile - display yards
      snprintf(dist_text,sizeof(dist_text),"%u",state.num_dist);
      text_layer_set_text(s_textlayer_dist_unit,"yards");
    }else{
      // over a mile - display miles
      int milesHundredths = state.num_dist*100/YARDS_PER_MILE;
      int miles = milesHundredths/100;
      int firstDecimal = (milesHundredths%100)/10;
      int secondDecimal = milesHundredths%10;
      snprintf(dist_text,sizeof(dist_text),"%u.%u%u", miles,firstDecimal,secondDecimal);
      text_layer_set_text(s_textlayer_dist_unit,"miles");
    }   
  }
  // update text layers
  text_layer_set_text(s_textlayer_dist_num,dist_text);
  text_layer_set_text(s_textlayer_laps_num,laps_text);
}

// update main timer and stats display (called at regular intervals)
void update_timer_display(void){
  // get individual digits
  int digit_tenths = (state.elapsed_time_ms%1000)/100;
  int digit_seconds = (state.elapsed_time_ms/1000)%60;
  int digit_minutes = (state.elapsed_time_ms/1000/60)%60;
  int digit_hours = state.elapsed_time_ms/1000/60/60;
  
  // preallocate char array
  static char s_elapsed_buffer[8];
  
  // Set font, position, and displayed digits depending on elapsed time
  if (digit_minutes < 1){
    text_layer_set_font(s_textlayer_timer,s_res_timerFont_large);
    layer_set_frame(text_layer_get_layer(s_textlayer_timer),GRect(0, 8, 114, 50));
    snprintf(s_elapsed_buffer,sizeof(s_elapsed_buffer),"%u.%u",digit_seconds,digit_tenths);
  }else if(digit_hours<1){
    text_layer_set_font(s_textlayer_timer,s_res_timerFont_large);
    layer_set_frame(text_layer_get_layer(s_textlayer_timer),GRect(0, 8, 114, 50));
    snprintf(s_elapsed_buffer,sizeof(s_elapsed_buffer),"%u:%02u",digit_minutes,digit_seconds);
  }else{
    text_layer_set_font(s_textlayer_timer,s_res_timerFont_small);
    layer_set_frame(text_layer_get_layer(s_textlayer_timer),GRect(0, 15, 114, 50));
    snprintf(s_elapsed_buffer,sizeof(s_elapsed_buffer),"%u:%02u:%02u",digit_hours,digit_minutes,digit_seconds);
  }
  
  // update the text layer for the main timer
  text_layer_set_text(s_textlayer_timer,s_elapsed_buffer);
  
  // preallocate for stats display
  static char s_stat_buffer[30];
  
  // get digits for current lap time
  int lap_digit_tenths = (state.elapsed_lap_time_ms%1000)/100;
  int lap_digit_seconds = (state.elapsed_lap_time_ms/1000)%60;
  int lap_digit_minutes = state.elapsed_lap_time_ms/1000/60;
  
  // depending on display mode, either get previous lap pace or previous lap time
  if(config.disp_mode){ // last lap pace
    int lastPace;
    if(config.dist_unit){ // imperial
      lastPace = state.previous_lap_ms*YARDS_PER_MILE/config.lap_dist;
    }else{ // metric
      lastPace = state.previous_lap_ms*METERS_PER_KM/config.lap_dist;
    }
    int lastPaceMin = lastPace/1000/SECONDS_PER_MINUTE;
    int lastPaceSec = (lastPace/1000)%SECONDS_PER_MINUTE;
    snprintf(s_stat_buffer,sizeof(s_stat_buffer),"%u:%02u.%u\n%u:%02u",lap_digit_minutes,lap_digit_seconds,lap_digit_tenths,lastPaceMin,lastPaceSec);
  }else{ // last lap time
    int prev_digit_tenths = (state.previous_lap_ms%1000)/100;
    int prev_digit_seconds = (state.previous_lap_ms/1000)%60;
    int prev_digit_minutes = state.previous_lap_ms/1000/60;
    snprintf(s_stat_buffer,sizeof(s_stat_buffer),"%u:%02u.%u\n%u:%02u.%u",lap_digit_minutes,lap_digit_seconds,lap_digit_tenths,prev_digit_minutes,prev_digit_seconds,prev_digit_tenths);
  }
  
  // update the text layer for the stats
  text_layer_set_text(s_textlayer_stats,s_stat_buffer);
}

// update time on status bar
void update_wall_time(void){
  static char s_buffer[9];
  clock_copy_time_string(s_buffer, sizeof(s_buffer));
  text_layer_set_text(s_textlayer_time, s_buffer);
}

// update action bar icons depending on current state
void update_action_bar(void){
  if(state.actionbar_toggled){
    // if toggled, show icons for reset and finish run
    action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_UP, s_res_image_action_icon_reset);
    action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_SELECT, s_res_image_action_icon_finish);
    action_bar_layer_clear_icon(s_actionbarlayer_1,BUTTON_ID_DOWN);
  }else{
    // if not toggled, show play/pause/increment icons
    if(state.paused){
      if(state.started){  
        // timer not running, but has been started; show play button
        action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_SELECT, s_res_image_action_icon_play);
      }else{
        // timer not running and not started since reset; show play-pause button
        action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_SELECT, s_res_image_action_icon_play_pause);
      }
      // timer not running; hide increment button
      action_bar_layer_clear_icon(s_actionbarlayer_1,BUTTON_ID_UP);
    }else{
      // timer running; show pause and increment buttons
      action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_SELECT, s_res_image_action_icon_pause);
      action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_UP, s_res_image_action_icon_increment);
    }
  }
}

// update color scheme of main screen
void update_colors(void){
  // determine colors depending on display capabilities
//   main_gcolor = PBL_IF_COLOR_ELSE(GColorFromHEX(config.main_color), GColorWhite);
//   accent_gcolor = PBL_IF_COLOR_ELSE(GColorFromHEX(config.accent_color), GColorBlack);
  main_gcolor = GColorFromHEX(config.main_color);
  accent_gcolor = GColorFromHEX(config.accent_color);
  
  // set colors for all elements on main window
  window_set_background_color(s_window, main_gcolor);
  action_bar_layer_set_background_color(s_actionbarlayer_1, accent_gcolor);
  bitmap_layer_set_background_color(s_bitmaplayer_bar_1, accent_gcolor);
  bitmap_layer_set_background_color(s_bitmaplayer_titlebox_1, accent_gcolor);
  bitmap_layer_set_background_color(s_bitmaplayer_titlebox_2, accent_gcolor);
  bitmap_layer_set_background_color(s_bitmaplayer_unitbox_1, accent_gcolor);
  bitmap_layer_set_background_color(s_bitmaplayer_unitbox_2, accent_gcolor);
  text_layer_set_text_color(s_textlayer_laps_num, accent_gcolor);
  text_layer_set_text_color(s_textlayer_laps_title, main_gcolor);
  text_layer_set_text_color(s_textlayer_dist_unit, main_gcolor);
  text_layer_set_text_color(s_textlayer_dist_num, accent_gcolor);
  text_layer_set_text_color(s_textlayer_dist_title, main_gcolor);
  text_layer_set_text_color(s_textlayer_time, accent_gcolor);
  text_layer_set_text_color(s_textlayer_timer, accent_gcolor);
  text_layer_set_text_color(s_textlayer_stats, accent_gcolor);
}

// update all elements on main screen; macro for calling other update functions
void update_main_screen(void){
  update_timer_display();
  update_distance_display();
  update_action_bar();
  update_colors();
}

// Mark timer as having been started, and begin timing
void start_new_run(void){
  state.started = true;
  resume_current_run();
}

// mark timer as paused and save current elapsed time
void pause_current_run(void){
  state.paused = true;
  state.last_elapsed_time_ms = state.elapsed_time_ms;
  update_action_bar();
}

// mark timer as resumed and save epoch time
void resume_current_run(void){
  state.paused = false;
  time_ms(&state.resume_time,&state.resume_time_ms);
  update_action_bar();
}

// set all values to default to prepare for new run to begin
void reset_current_run(void){
  state.last_elapsed_time_ms = 0;
  state.lap_start_time_ms = 0;
  
  state.num_laps = 0;
  state.num_dist = 0;
  
  state.elapsed_time_ms = 0;
  state.elapsed_lap_time_ms = 0;
  state.previous_lap_ms = 0;
  state.previous_lap_ms = 0;
  state.best_lap_ms = 1000*SECONDS_PER_HOUR*5;
  state.worst_lap_ms = 0;
  
  state.actionbar_toggled = false;
  state.started = false;
  state.paused = true;
  update_main_screen();
}

// finish the current run; if timer is running, stop it and increment the lap count; display results
void finish_current_run(void){
  if(state.paused==false){
    pause_current_run();
    increment_lap_count();
  }
  show_summary_screen();
}

// add 1 to the lap count and calculate stats
void increment_lap_count(void){
  // do nothing if lap count is over 99
  if (state.num_laps >= 99){
    return;
  }
  state.num_laps++;
  state.num_dist = state.num_laps*config.lap_dist;
  
  state.lap_start_time_ms = state.elapsed_time_ms; // mark the time of the new lap beginning
  state.previous_lap_ms = state.elapsed_lap_time_ms; // store previous lap time for display
  state.elapsed_lap_time_ms = 0; // reset new lap time
  
  // calculate lap stats
  state.average_lap_ms = state.lap_start_time_ms/state.num_laps;
  if(state.previous_lap_ms < state.best_lap_ms){
    state.best_lap_ms = state.previous_lap_ms;
  } 
  if(state.previous_lap_ms > state.worst_lap_ms){
     state.worst_lap_ms = state.previous_lap_ms;
  }
  update_distance_display();
}

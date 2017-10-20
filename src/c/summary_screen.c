#include <pebble.h>
#include "summary_screen.h"
#include "main.h"

static Window *s_window;
static GFont s_res_gothic_24_bold;
static GFont s_res_gothic_18_bold;
static TextLayer *s_textlayer_sum_headings;
static TextLayer *s_textlayer_sum_data;
static TextLayer *s_textlayer_sum_title;
static TextLayer *s_textlayer_lap_title;
static TextLayer *s_textlayer_lap_headings;
static TextLayer *s_textlayer_lap_data;

GColor main_gcolor;
GColor accent_gcolor;

// create all window layers
static void initialise_ui(void) {
  s_window = window_create();
  window_set_background_color(s_window,main_gcolor);
  
  s_res_gothic_24_bold = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_res_gothic_18_bold = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  
  // s_textlayer_sum_headings
  s_textlayer_sum_headings = text_layer_create(GRect(15, 28, 55, 56));
  text_layer_set_background_color(s_textlayer_sum_headings, GColorClear);
  text_layer_set_text(s_textlayer_sum_headings, "Time: Dist: Pace:");
  text_layer_set_text_color(s_textlayer_sum_headings, accent_gcolor);
  text_layer_set_font(s_textlayer_sum_headings, s_res_gothic_18_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_sum_headings);
  
  // s_textlayer_sum_data
  s_textlayer_sum_data = text_layer_create(GRect(60, 28, 70, 56));
  text_layer_set_background_color(s_textlayer_sum_data, GColorClear);
  text_layer_set_text(s_textlayer_sum_data, "25:50 4.63 km 5:45 ");
  text_layer_set_text_color(s_textlayer_sum_data, accent_gcolor);
  text_layer_set_text_alignment(s_textlayer_sum_data, GTextAlignmentRight);
  text_layer_set_font(s_textlayer_sum_data, s_res_gothic_18_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_sum_data);
  
  // s_textlayer_sum_title
  s_textlayer_sum_title = text_layer_create(GRect(0, 0, 144, 28));
  text_layer_set_background_color(s_textlayer_sum_title, accent_gcolor);
  text_layer_set_text_color(s_textlayer_sum_title, main_gcolor);
  text_layer_set_text(s_textlayer_sum_title, "Summary");
  text_layer_set_text_alignment(s_textlayer_sum_title, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_sum_title, s_res_gothic_24_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_sum_title);
  
  // s_textlayer_lap_title
  s_textlayer_lap_title = text_layer_create(GRect(0, 84, 144, 28));
  text_layer_set_background_color(s_textlayer_lap_title, accent_gcolor);
  text_layer_set_text_color(s_textlayer_lap_title, main_gcolor);
  text_layer_set_text(s_textlayer_lap_title, "Lap Times");
  text_layer_set_text_alignment(s_textlayer_lap_title, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_lap_title, s_res_gothic_24_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_lap_title);
  
  // s_textlayer_lap_headings
  s_textlayer_lap_headings = text_layer_create(GRect(15, 112, 60, 56));
  text_layer_set_background_color(s_textlayer_lap_headings, GColorClear);
  text_layer_set_text(s_textlayer_lap_headings, "Best: Average: Worst:");
  text_layer_set_text_color(s_textlayer_lap_headings, accent_gcolor);
  text_layer_set_font(s_textlayer_lap_headings, s_res_gothic_18_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_lap_headings);
  
  // s_textlayer_lap_data
  s_textlayer_lap_data = text_layer_create(GRect(70, 112, 60, 56));
  text_layer_set_background_color(s_textlayer_lap_data, GColorClear);
  text_layer_set_text(s_textlayer_lap_data, "48.2 55.0 1:05.2 ");
  text_layer_set_text_color(s_textlayer_lap_data, accent_gcolor);
  text_layer_set_text_alignment(s_textlayer_lap_data, GTextAlignmentRight);
  text_layer_set_font(s_textlayer_lap_data, s_res_gothic_18_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_lap_data);
}

// destroy window layers to free heap memory
static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(s_textlayer_sum_headings);
  text_layer_destroy(s_textlayer_sum_data);
  text_layer_destroy(s_textlayer_sum_title);
  text_layer_destroy(s_textlayer_lap_title);
  text_layer_destroy(s_textlayer_lap_headings);
  text_layer_destroy(s_textlayer_lap_data);
}

// called when window is closed
static void handle_window_unload(Window* window) {
  destroy_ui();
}

// callback to display screen
void show_summary_screen(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
  update_summary_screen();
}

// callback to hide screen
void hide_summary_screen(void) {
  window_stack_remove(s_window, true);
}

// show relevant summary data
void update_summary_screen(void){
  // summary section
  int timeHour = state.elapsed_time_ms/1000/SECONDS_PER_HOUR;
  int timeMin = (state.elapsed_time_ms/1000/SECONDS_PER_MINUTE)%MINUTES_PER_HOUR;
  int timeSec = (state.elapsed_time_ms/1000)%SECONDS_PER_MINUTE;
  int dist;
  int distDecimal;
  int pace;
  char unitBuffer[3];
  
  // depending on units, set distance and pace strings differently
  if(config.dist_unit==0){ // metric
    dist = state.num_dist/METERS_PER_KM;
    distDecimal = (state.num_dist%1000)/10;
    pace = state.lap_start_time_ms*METERS_PER_KM/state.num_dist; // pace in milliseconds per km
    strcpy(unitBuffer,"km");
  }else{ // imperial
    int distHundredths = state.num_dist*100/YARDS_PER_MILE;
    dist = distHundredths/100;
    distDecimal = distHundredths%100;
    pace = state.lap_start_time_ms*YARDS_PER_MILE/state.num_dist; // pace in milliseconds per mile
    strcpy(unitBuffer,"mi");
  }
  
  // display time, distance, and pace info
  int paceMin = pace/1000/SECONDS_PER_MINUTE;
  int paceSec = (pace/1000)%SECONDS_PER_MINUTE;
  static char s_sum_data_buffer[] = "00:00:00\n99.99 km\n10:00/km"; 
  snprintf(s_sum_data_buffer,sizeof(s_sum_data_buffer),"%u:%02u:%02u\n%u.%02u %s\n%u:%02u/%s",timeHour,timeMin,timeSec,dist,distDecimal,unitBuffer,paceMin,paceSec,unitBuffer);
  text_layer_set_text(s_textlayer_sum_data, s_sum_data_buffer);
  
  // lap times section
  int bestMin = state.best_lap_ms/1000/SECONDS_PER_MINUTE;
  int bestSec = (state.best_lap_ms/1000)%SECONDS_PER_MINUTE;
  int bestTenths = (state.best_lap_ms%1000)/100;
  int averageMin = state.average_lap_ms/1000/SECONDS_PER_MINUTE;
  int averageSec = (state.average_lap_ms/1000)%SECONDS_PER_MINUTE;
  int averageTenths = (state.average_lap_ms%1000)/100;
  int worstMin = state.worst_lap_ms/1000/SECONDS_PER_MINUTE;
  int worstSec = (state.worst_lap_ms/1000)%SECONDS_PER_MINUTE;
  int worstTenths = (state.worst_lap_ms%1000)/100;
  
  static char s_lap_data_buffer[] = "100:00.0\n100:05.0\n100:10.0";
  snprintf(s_lap_data_buffer,sizeof(s_lap_data_buffer),"%u:%02u.%u\n%u:%02u.%u\n%u:%02u.%u",bestMin,bestSec,bestTenths,averageMin,averageSec,averageTenths,worstMin,worstSec,worstTenths);
  text_layer_set_text(s_textlayer_lap_data, s_lap_data_buffer);
}
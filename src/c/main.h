#pragma once
#define STATE_PKEY 13
#define CONFIG_PKEY 14
#define METERS_PER_KM 1000
#define YARDS_PER_MILE 1760

// structure to store config preferences
typedef struct{
  int dist_unit;
  int lap_dist;
  int disp_mode;
  int main_color;
  int accent_color;
} AppConfig;

// define structure used for persistently storing state of timer
typedef struct {
  // state flags
  bool paused;
  bool started;
  bool actionbar_toggled;
  
  // distance
  int num_laps;
  int num_dist;
  
  // time
  unsigned long int elapsed_time_ms; // elapsed time up to present
  unsigned long int elapsed_lap_time_ms; // elapsed lap time up to present
  unsigned long int last_elapsed_time_ms; // elapsed time up to last resume
  time_t resume_time;          // epoch time when last resumed
  uint16_t resume_time_ms;     // milliseconds of epoch time when last resumed
  unsigned long int lap_start_time_ms;  // time when lap last started (in reference to elapsed time)
  unsigned long int previous_lap_ms;
  unsigned long int best_lap_ms;
  unsigned long int average_lap_ms;
  unsigned long int worst_lap_ms;
} TimerState;

extern TimerState state;
extern AppConfig config;
extern GColor main_gcolor;
extern GColor accent_gcolor;
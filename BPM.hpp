#ifndef BPM_INCLUDED
#define BPM_INCLUDED
//#define ENABLE_STEP_DEBUG // for enabling step debug output

#include "MidiSetup.hpp"

// utility functions for calculating BPM etc

// defines for clock type and time signature etc

#define SEQUENCE_LENGTH_STEPS   16
#define STEPS_PER_BEAT          4
#define SEQUENCE_LENGTH_BEATS   SEQUENCE_LENGTH_STEPS / STEPS_PER_BEAT
#define BEATS_PER_BAR           SEQUENCE_LENGTH_BEATS
#define BARS_PER_PHRASE         4

#define TICKS_PER_STEP  (PPQN/STEPS_PER_BEAT)

byte cc_value_clock_tick_ratio = PPQN;

unsigned long received_ticks = 0;
unsigned long last_tick_received_at = 0;
unsigned long last_tick_at = 0;   // not sure if this is necessary/duplicate?
unsigned long first_tick_received_at = 0;

// for telling the rest of the program about what step and beat we're on
int current_total_step = 0;
int current_total_beat = 0;
int current_total_bar  = 0;
int current_step = 0; 
int current_beat = 0; 
int current_bar  = 0;
int current_phrase = 0;
int current_song_position = 0;

unsigned int song_position; // number of beats in total


bool is_bpm_on_beat = false;
bool is_bpm_on_step = false;
bool is_bpm_on_bar  = false;
bool is_bpm_on_phrase=false;

double bpm_current = 60.0f; //120.0f;
double last_bpm = bpm_current;

bool bpm_internal_mode = false;

// stuff for calculating BPM
#define last_beat_sample_size 4
int last_beat_stamp[last_beat_sample_size];
int ph = 0;
unsigned long last_beat_at = 0;
static unsigned long last_ticked = 0;


// stubs
void bpm_receive_clock_tick ();
void bpm_reset_clock (int offset = 0);

#endif

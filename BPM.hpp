#ifndef BPM_INCLUDED
#define BPM_INCLUDED
//#define ENABLE_STEP_DEBUG // for enabling step debug output

#include "MidiSetup.hpp"

// utility functions for calculating BPM etc

// defines for clock type and time signature etc

#define STEPS_PER_BEAT          4
#define SEQUENCE_LENGTH_STEPS   16
#define SEQUENCE_LENGTH_BEATS   (SEQUENCE_LENGTH_STEPS / STEPS_PER_BEAT)
#define BEATS_PER_BAR           SEQUENCE_LENGTH_BEATS
#define BARS_PER_PHRASE         4
#define STEPS_PER_BAR           (STEPS_PER_BEAT*BEATS_PER_BAR)

#define TICKS_PER_STEP  (PPQN/STEPS_PER_BEAT)
#define DEFAULT_CLOCK_TICK_RATIO    1      // use this to send standard MIDI clock at 24 PPQN
//#define DEFAULT_CLOCK_TICK_RATIO  PPQN   // or use this to only send 1 clock tick per beat

byte cc_value_clock_tick_ratio = DEFAULT_CLOCK_TICK_RATIO; 

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


// prototypes
void bpm_receive_clock_tick ();
void bpm_reset_clock (int offset = 0);
void bpm_update_status( unsigned int received_ticks );



int euclidian_seed_modifier = 0;
int euclidian_seed_modifier_2 = 0;
bool euclidian_seed_use_phrase      = true;

unsigned int get_euclidian_seed() {
  int seed = euclidian_seed_modifier;
  if (euclidian_seed_modifier_2 > 0)  seed *= (256 + euclidian_seed_modifier_2 * 2);
  if (euclidian_seed_use_phrase)      seed += current_phrase + 1;
  if (seed == 0) seed = 1;
  return seed;
}



class bpm_status {
public:
  // for telling the rest of the program about what step and beat we're on
  int current_total_step = 0;
  int current_total_beat = 0;
  int current_total_bar  = 0;
  int current_step = 0; 
  int current_beat = 0; 
  int current_bar  = 0;
  int current_phrase = 0;
  int current_song_position = 0;
    
  bool is_bpm_on_beat = false;
  bool is_bpm_on_step = false;
  bool is_bpm_on_bar  = false;
  bool is_bpm_on_phrase=false;

  void update( unsigned int received_ticks ) {
    current_total_step = (received_ticks/TICKS_PER_STEP);
    current_total_beat = current_total_step / STEPS_PER_BEAT;
    current_total_bar  = current_total_beat / BARS_PER_PHRASE;
    current_step    =  current_total_step % SEQUENCE_LENGTH_STEPS;
    current_beat    =  current_step / STEPS_PER_BEAT; //(ticks/24);//%16;
    current_bar     =  (received_ticks/(TICKS_PER_STEP*STEPS_PER_BEAT*BEATS_PER_BAR)) % BARS_PER_PHRASE;
    current_phrase  =  (received_ticks/(TICKS_PER_STEP*STEPS_PER_BEAT*BEATS_PER_BAR)) / BARS_PER_PHRASE;
    //Serial.printf("bpm_update_status: current_phrase is %i from received_ticks %i\r\n", current_phrase, received_ticks);
  
    is_bpm_on_beat   = (0==received_ticks%PPQN);
    is_bpm_on_step   = (0==received_ticks%TICKS_PER_STEP);
    is_bpm_on_bar    = is_bpm_on_beat && current_beat == 0;
    is_bpm_on_phrase = is_bpm_on_bar && (current_bar % BARS_PER_PHRASE) == 0;
    if (is_bpm_on_beat) {
      current_song_position = received_ticks/PPQN;  // TODO: need to take into account that song position is set by the DAW sometimes....
      //Serial.printf("current_beat is %i, current song position is %i\r\n", current_beat, current_song_position);
    }
  }
};

#endif

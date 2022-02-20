#ifndef EUCLIDIAN_INCLUDED
#define EUCLIDIAN_INCLUDED

#ifndef DEFAULT_MUTATE_MINIMUM_PATTERN
#define DEFAULT_MUTATE_MINIMUM_PATTERN  0     // all patterns above 0
//#define DEFAULT_MUTATE_MINIMUM_PATTERN  1   // default to 1 so kick doesn't change
#endif
#ifndef DEFAULT_RESET_BEFORE_MUTATE
#define DEFAULT_RESET_BEFORE_MUTATE   true
#endif

#include "Drums.h"
#include "BPM.hpp"
#include "Envelopes.h"


// bass, melody

#define NUM_MIDI_OUTS 4
#define NUM_PATTERNS (NUM_TRIGGERS+NUM_ENVELOPES+NUM_MIDI_OUTS) //(NUM_TRIGGERS+NUM_ENVELOPES+1) //17   // 11 triggers + 5 envelopes + 2 midi output
#define PATTERN_BASS (NUM_TRIGGERS+NUM_ENVELOPES)
#define PATTERN_MELODY (PATTERN_BASS+1)
#define PATTERN_PAD_ROOT  (PATTERN_MELODY+1)
#define PATTERN_PAD_PITCH (PATTERN_MELODY+2)

#if (PATTERN_PAD_PITCH != NUM_PATTERNS-1)
COMPILE FAIL -- NUM_PATTERNS doesnt match PATTERN_PAD_PITCH !
#endif

#define CC_EUCLIDIAN_MUTATE_DENSITY       7     // automatically mutate density on/off
#define CC_EUCLIDIAN_ACTIVE_STATUS_START  32    // active status of euclidian tracks starts at this CC and goes up to same+NUM_PATTERNS
#define CC_EUCLIDIAN_ACTIVE_STATUS_END    (CC_EUCLIDIAN_ACTIVE_STATUS_START + NUM_PATTERNS) // (50 when num_patterns = 2)
#define CC_EUCLIDIAN_SET_AUTO_PLAY        16    // enable/disable autoplaying on internal BPM
#define CC_EUCLIDIAN_SET_MUTATE_MODE      20    // 0 = NONE, 1 = SUBTLE, 2 = TOTAL
#define CC_EUCLIDIAN_SEED_MODIFIER        22    // euclidian random "bank" X
#define CC_EUCLIDIAN_SEED_MODIFIER_2      23    // euclidian random "bank" Y
#define CC_EUCLIDIAN_RESET_BEFORE_MUTATE  24    // reset sequences to 0 before mutation?
#define CC_EUCLIDIAN_SET_MINIMUM_PATTERN  25    // lowest number pattern to automutate
#define CC_EUCLIDIAN_SET_MAXIMUM_PATTERN  26    // highest number pattern to automutate
#define CC_EUCLIDIAN_SEED_USE_PHRASE      27    // increment seed according to BPM phrase (ie automutate)
#define CC_EUCLIDIAN_FILLS                28    // enable/disable fills on last bar of phrase, 0=off
#define CC_EUCLIDIAN_DENSITY              114   // experimental: global density of euclidian patterns

#define CC_EUCLIDIAN_HIHAT_SHUFF          9     // enable/disable hihat shuffle, 0=off
#define CC_EUCLIDIAN_CLAP_FLAM            8     // enable/disable clap flam, 0=off

// so need master "mutate enabled"
//    and then sub-options within that:-
//      mutate with masking
//      seed based on bar number, for repeatability
//        set 

enum mutate_modes : int {
  EUCLIDIAN_MUTATE_MODE_NONE = 0,
  EUCLIDIAN_MUTATE_MODE_SUBTLE = 1,
  EUCLIDIAN_MUTATE_MODE_TOTAL = 2,
  EUCLIDIAN_MUTATE_MODE_ACIDBANGER = 3,
  EUCLIDIAN_MUTATE_MODE_MASKED = 4,
  EUCLIDIAN_MUTATE_MODE_MAX = 5
};
int euclidian_mutate_mode = EUCLIDIAN_MUTATE_MODE_TOTAL;

int euclidian_mutate_minimum_pattern = DEFAULT_MUTATE_MINIMUM_PATTERN; // default 1 so that kick never mutates
int euclidian_mutate_maximum_pattern = NUM_PATTERNS;

float max_euclidian_density = 1.2f;

bool euclidian_reset_before_mutate  = DEFAULT_RESET_BEFORE_MUTATE;

bool mutate_harmony_root    = false;
bool mutate_enabled         = false;
bool mask_enabled           = false;
bool euclidian_fills_enabled = true;

bool euclidian_auto_play    = true;

bool euclidian_shuffle_hats = false; //true;
bool euclidian_flam_clap    = false; //true;

typedef struct {
  bool active_status = true;
  int steps, pulses, rotation, duration;
  bool stored[32];
  int original_steps;  // for storing the original generated size of the pattern, in case we resize it etc
  int trigger;
  int tie_on = 0;
} pattern_t;

pattern_t patterns[NUM_PATTERNS];


void initialise_euclidian();
bool handle_euclidian_ccs(byte channel, byte number, byte value);
bool euclidian_set_auto_play (bool enable = true);
void process_euclidian(int ticks);


#endif

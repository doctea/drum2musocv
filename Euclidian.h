#ifndef EUCLIDIAN_INCLUDED
#define EUCLIDIAN_INCLUDED

#define NUM_PATTERNS 17   // 11 triggers + 5 envelopes + 1 midi output
#define BASS_PATTERN 17

#define CC_EUCLIDIAN_ACTIVE_STATUS_START  32  // active status of euclidian tracks starts at this CC and goes up to same+NUM_PATTERNS
#define CC_EUCLIDIAN_SET_AUTO_PLAY        16  
#define CC_EUCLIDIAN_SET_MUTATE_MODE      20
#define CC_EUCLIDIAN_SEED_MODIFIER        22
#define CC_EUCLIDIAN_RESET_BEFORE_MUTATE  23

// so need master "mutate enabled"
//    and then sub-options within that:-
//      mutate with masking
//      seed based on bar number, for repeatability
//        set 

/*enum mutate_modes : int {
  EUCLIDIAN_MUTATE_MODE_NONE = 0,
  EUCLIDIAN_MUTATE_MODE_RANDOM = 1,
  EUCLIDIAN_MUTATE_MODE_SEED_BAR = 2,
  EUCLIDIAN_MUTATE_MODE_RESET_ON_STOP = 3,
  EUCLIDIAN_MUTATE_MODE_MAX = 4
};*/

int euclidian_seed_modifier = 0;
bool euclidian_reset_before_mutate = false;

bool mutate_enabled = false;
bool mask_enabled = false;

bool euclidian_auto_play = true;
//int euclidian_mutate_mode = EUCLIDIAN_MUTATE_MODE_RANDOM;

typedef struct {
  bool active_status = true;
  int steps, pulses, rotation, duration;
  bool stored[32];
  int original_steps;  // for storing the original generated size of the pattern, in case we resize it etc
} pattern_t;

pattern_t patterns[NUM_PATTERNS];


void initialise_euclidian();
bool handle_euclidian_ccs(byte channel, byte number, byte value);
bool euclidian_set_auto_play (bool enable = true);


#endif

#ifndef EUCLIDIAN_INCLUDED
#define EUCLIDIAN_INCLUDED

#define NUM_PATTERNS 17   // 11 triggers + 5 envelopes + 1 midi output
#define BASS_PATTERN 17

#define CC_EUCLIDIAN_ACTIVE_STATUS_START  32  // active status of euclidian tracks starts at this CC and goes up to same+NUM_PATTERNS
#define CC_EUCLIDIAN_SET_AUTO_PLAY        16  

bool mutate_enabled = false;
bool mask_enabled = true;

bool euclidian_auto_play = true;

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

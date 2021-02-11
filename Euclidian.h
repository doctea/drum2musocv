#ifndef EUCLIDIAN_INCLUDED
#define EUCLIDIAN_INCLUDED

#define NUM_PATTERNS 17   // 11 triggers + 5 envelopes + 1 midi output

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


#endif

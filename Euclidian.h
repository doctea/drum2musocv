#define NUM_PATTERNS 16

bool mutate_enabled = false;
bool mask_enabled = false;

typedef struct {
  int steps, pulses, rotation;
  bool stored[32];
  int original_steps;  // for storing the original generated size of the pattern, in case we resize it etc
} pattern_t;

pattern_t patterns[NUM_PATTERNS];

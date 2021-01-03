#ifndef ENVELOPES_INCLUDED
#define ENVELOPES_INCLUDED

// STRUCTS

enum envelope_types : byte {
  ENV_CRASH = 0,
  ENV_SPLASH = 1,
  ENV_WOBBLY = 2,
  ENV_RIDE_BELL = 3,
  ENV_RIDE_CYMBAL = 4
  // TODO: more envelope types...
};
#define NUM_ENVELOPES 5

#define ENV_CC_SPAN   8   // how many CCs to reserve per-envelope
#define ENV_CC_START  64  // what number CC the envelope controls begin at

#define SUSTAIN_MINIMUM 32
#define ENV_MAX_ATTACK 32
#define ENV_MAX_HOLD   32
#define ENV_MAX_DECAY  32
#define ENV_MAX_RELEASE 64

//#define TEST_LFOS

enum stage : byte {
  OFF = 0,
  //DELAY,  // time
  ATTACK,
  HOLD, // time
  DECAY,
  SUSTAIN,
  RELEASE,
  //END = 0
  LFO_SYNC_RATIO_HOLD_AND_DECAY,
  LFO_SYNC_RATIO_SUSTAIN_AND_RELEASE
};
// above enums also used as the envelope CC offsets

//#define LFO_SYNC_RATIO (RELEASE+1)

typedef struct envelope_state {
//#ifndef TEST_LFOS
  byte stage = OFF;
/*#else
  byte stage = LFO_SYNC_RATIO;
#endif*/

  byte velocity;         // triggered velocity
  byte actual_level;          // right now, the level
  byte stage_start_level;     // level at start of current stage

  // TODO: int delay_length = 5;  //  D - delay before atack starts
  unsigned int  attack_length = 0;     //  A - attack  - length of stage
  unsigned int  hold_length = PPQN * 2; //48;  //  H - hold    - length to hold at end of attack before decay
  unsigned int  decay_length = PPQN * 4; //384; //512;     //  D - decay   - length of stage
  float         sustain_ratio = 0.90f;  //  S - sustain - level to drop to after decay phase
  unsigned int  release_length = PPQN * 16; //768;   //  R - release - length (time to drop to 0)

  byte lfo_sync_ratio_hold_and_decay = 0;
  byte lfo_sync_ratio_sustain_and_release = 0;

  unsigned long stage_triggered_at = 0;
  unsigned long triggered_at = 0; 
  unsigned long last_sent_at = 0;

  byte midi_cc;

  byte last_sent_lvl;
};


// GLOBALS 

byte cc_value_sync_modifier = 31;  // initial global clock sync modifier

envelope_state envelopes[NUM_ENVELOPES];


// stubs
void update_envelope (byte env_num, byte velocity, bool state);
bool handle_envelope_ccs(byte channel, byte number, byte value);
void kill_envelopes();
  
#endif

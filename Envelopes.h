#ifndef ENVELOPES_INCLUDED
#define ENVELOPES_INCLUDED

// GLOBALS

byte cc_value_sync_modifier = 127;  // initial global clock sync modifier

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

// stubs
void update_envelope (byte env_num, byte velocity, bool state);
bool handle_envelope_ccs(byte channel, byte number, byte value);
void kill_envelopes();
  
#endif

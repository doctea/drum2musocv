#ifndef BASS_INCLUDED
#define BASS_INCLUDED

//#define BASS_DEBUG
// handling debugging output - pattern from https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c/1644898#1644898
#ifdef BASS_DEBUG
#define BASS_DEBUG 1
#else
#define BASS_DEBUG 0
#endif

#define BASS_printf(fmt, ...)   do { if (BASS_DEBUG) Serial.printf((fmt), ##__VA_ARGS__); } while (0)
#define BASS_println(fmt, ...)  do { if (BASS_DEBUG) Serial.println((fmt), ##__VA_ARGS__); } while (0)

// CONFIGURATION: messages targeted to channel _IN will be relayed on channel _OUT -- for passing through messages to Neutron (TODO: probably move this to a dedicated config file)
#define MIDI_CHANNEL_BASS_IN        8     // channel to receive direct bass playing
#define MIDI_CHANNEL_BASS_AUTO_IN   9     // channel to receive automatic bass notes
#define MIDI_CHANNEL_BASS_OUT       2     // channel to output bass notes on

#define CC_BASS_SET_ARP_MODE        17    // cc to set the bass arp mode
#define CC_BASS_ONLY_NOTE_HELD      18    // cc to set bass to only play in external mode if note is held

#define ARP_MODE_NONE         0
#define ARP_MODE_PER_BEAT     1
#define ARP_MODE_NEXT_ON_NOTE 2
#define ARP_MODE_MAX          3


#include "MidiInput.hpp"
#include "MidiOutput.hpp"
#include "BPM.hpp"              // for access to song position info
#include "ChannelState.hpp"

// prototypes to deal with circular dependency on MidiOutput (we need access to midioutput stuff here, but midioutput also needs access to stuff here)
void midi_bass_send_note_on(int pitch, int velocity, int channel = MIDI_CHANNEL_BASS_OUT);
void midi_bass_send_note_off(int pitch, int velocity = 0, int channel = MIDI_CHANNEL_BASS_OUT);

ChannelState bass = ChannelState();   // tracking notes that are held

// stuff for handling scales, for automatic bassline/arp generation

#define SCALE_SIZE  7

int bass_scale_offset[][SCALE_SIZE] = {
  { 0, 2, 4, 5, 7, 9, 11 },  // major scale
  { 0, 2, 3, 5, 7, 8, 10 },  // // minor scale (? check)
};
int bass_sequence[][4]     =   { // degrees of scale to play per chord -- ie, arp patterns
  { 0, 0, 0, 0 },
  { 0, 0, 0, 1 },
  { 0, 1, 1, 2 },
  { 0, 1, 0, 4 },
  //{ 0, 2, 4, 6 },
  /*{ 0, 2, 0, 4 },
    { 0, 3, 6, 4 }*/
};
int chord_progression[]    =   { 0, 5, 1, 4 };     // chord progression

#define BASS_NUM_SCALES             (sizeof(bass_scale_offset) / sizeof(bass_scale_offset[0]))      // how many scales we know about in total
#define BASS_NUM_SEQUENCES          (sizeof(bass_sequence) / sizeof(bass_sequence[0]))              // how many sequences we know about in total
#define BASS_SCALE_SIZE             ((int)(sizeof(bass_scale_offset[0])/sizeof(bass_scale_offset[0][0])))   // how many notes in each scale
#define BASS_SEQUENCE_LENGTH        ((int)(sizeof(bass_sequence[0])/sizeof(bass_sequence[0][0])))   // how many notes in arps
#define BASS_CHORD_PROGRESSION_LENGTH  ((int)(sizeof(chord_progression)/sizeof(chord_progression[0])))      // how many chords in progression

int bass_counter = 0;               // track current position in arp sequence
int bass_currently_playing = -1;    // track currently playing note, so that we know which one to turn off

int scale_number = 0;               // index of the current scale we're in
int chord_number = 0;               // index of the current chord degree that we're playing (0-6, well actually can be negative or go beyond that to access lower&higher octaves)
int sequence_number = 0;            // index of the arp sequence that we're currently playing

// TODO: make these configurable by UI/CC
bool bass_auto_scale       = false;   // automatically switch scales every phrase
bool bass_auto_progression = false;   // automatically play chords in progression order
bool bass_auto_arp         = false;   // choose notes to play from the current sequence (eg incrementing through them)
int bass_arp_mode          = ARP_MODE_NEXT_ON_NOTE;
bool bass_only_note_held = true;

/*
  scale chord number to octave table 
  -14,-13,-12,-11,-10,-9, -8 = octave -2
  -7, -6, -5, -4, -3, -2, -1 = octave -1
  0,   1,  2,  3,  4,  5,  6 = octave 0
  7,   8,  9, 10, 11, 12, 13 = octave +1
  14, 15, 16, 17, 18, 19, 20 = octave +2
*/

void debug_bass_scales() {
  // TODO: make this use the actual functions instead of these tests
  Serial.println("-------\r\ndebug_bass_scales");
  for (int scale_pitch = -20 ; scale_pitch < 20 ; scale_pitch++) {
    Serial.printf("scale_pitch %i: ", scale_pitch);

    int sd = scale_pitch % BASS_SCALE_SIZE;
    Serial.printf("degree of scale = %i, ", sd);

    int oct = scale_pitch / BASS_SCALE_SIZE;
    if (sd < 0) oct--;
    Serial.printf("octave is %i, ", oct);

    Serial.println("");
  }
  Serial.println("-------");
}


void initialise_bass() {
  debug_bass_scales();
}

// get the currently active scale
int get_scale_number() {
  if (bass_auto_scale) {
    // automatically change scales based on our current song phrase position
    return scale_number = (0 + current_phrase) % BASS_NUM_SCALES; // todo: make this switchable ..
  } 
  
  return 0; // default to major
}

// get the currently active chord in the scale
int get_chord_number() {
  if (bass_auto_progression) {
    // automatically change chord based on our current song bar position
    return chord_progression[current_bar % BASS_CHORD_PROGRESSION_LENGTH] % BASS_SCALE_SIZE; // todo: make this select from lowest held note?
  } 
  
  return 0; // default to root (default or whatever is held)
}

// get the currently active arp sequence index
int get_sequence_number() {
  if (bass_auto_arp) {
    // choose the arp sequence automatically based on the current song position
    sequence_number = current_bar % BASS_NUM_SEQUENCES;
  } else {
    sequence_number = 0;
  }
  
  return sequence_number;
}

// get the root note for the scale chord -- can go negative/higher than 6 to access other octaves
int bass_get_scale_note(int scale_degree = 0) {
  // todo: move this elsewhere to make this changeable..
  //int scale_number = 0;
  int scale_number = get_scale_number();
  int chord_number = get_chord_number();

  // temporary step through chord number based on the current_bar (so resets for each new phrase)
  //scale_degree += chord_number;

  int sd = (chord_number + scale_degree);
  int oct = sd / BASS_SCALE_SIZE;

  BASS_printf("bass_get_scale: in phrase:bar:beat %i:%i:%i, using scale %i ", current_phrase, current_bar, current_beat, scale_number );
  BASS_printf(" || chord %i, scale degree %i -> got scale offset %i and oct %i -> ",  chord_number, scale_degree, sd, oct);
  if (sd < 0) {
    sd = BASS_SCALE_SIZE + sd;  // convert to the actual scale degree
    oct--;                      // account for needing to be one octave below
  }
  BASS_printf("final oct:sd is %i:%i", oct, sd);

  int r = bass_scale_offset[scale_number][sd % BASS_SCALE_SIZE]
          +
          (oct * 12)
          ;

  BASS_printf(" -- final note is %i\r\n", r);
  return r;
}

// get the note to play based on the position in the arp sequence
int bass_get_sequence_note(int position = 0) {
  if (position % BASS_SEQUENCE_LENGTH == 0) BASS_printf("----- at bass arp sequence start (position %i)\r\n", position);
  int sequence_number = get_sequence_number();

  return bass_get_scale_note(
    bass_sequence [sequence_number] [position % BASS_SEQUENCE_LENGTH]
  );
}

int bass_get_sequence_pitch(int position = 0) {
  //bass_root = MIDI_BASS_ROOT_PITCH;// + current_phrase;
  if (bass.is_note_held()) {
    // bass is autoplaying, so ask it for what note it recommends for this position
    return bass.get_sequence_held_note(position);
  }
  // else base it on the root note and the sequence's position
  return bass.get_root_note() + bass_get_sequence_note(position);
}

void bass_reset_sequence() {
  bass_counter = 0;
}


// start note for bass 
void bass_note_on (int v = 127) {
  int pitch_offset = bass_get_sequence_pitch(bass_counter); // current_beat); // todo: configurable options to change based on bass_counter or to keep synced to the current_beat number
  //BASS_printf("bass_note_on: bass pitch is %i\r\n", pitch_offset);

  bass_currently_playing = pitch_offset;
  midi_bass_send_note_on(bass_currently_playing, v);

#ifdef DEBUG_BASS
  Serial.printf("BASS NOTE ON>>> %s :: %s\n", get_bass_info(), get_bass_info_2());
#endif
}

// start note and increment position counter
void bass_note_on_and_next(int v = 127) {
  if (!bpm_internal_mode && bass_only_note_held && !bass.is_note_held())
    return;
  
  bass_note_on(v);

  if (bass_arp_mode==ARP_MODE_NEXT_ON_NOTE)
    bass_counter++;
  else if (bass_arp_mode==ARP_MODE_PER_BEAT)
    bass_counter = current_beat;
  else if (bass_arp_mode==ARP_MODE_NONE)// dont arp at all
    bass_counter = 0;
}

void bass_note_off() {
  //BASS_printf("bass_note_off: bass pitch offset is %i\r\n", bass_currently_playing);
  if (!bass_currently_playing && (!bpm_internal_mode && bass_only_note_held && !bass.is_note_held()))
    return;

  if (bass_currently_playing >= 0)
    midi_bass_send_note_off(bass_currently_playing, 0, MIDI_CHANNEL_BASS_OUT);

  bass_currently_playing = -1;
}

void bass_set_arp_mode(int mode) {
  bass_arp_mode = mode % ARP_MODE_MAX;
}

void bass_set_only_note_held(int value) {
  bass_only_note_held = value; //= mode % ARP_MODE_MAX;
}

bool handle_bass_ccs(byte channel, byte number, byte value) {
  if (channel!=GM_CHANNEL_DRUMS) return false;

  if (number==CC_BASS_SET_ARP_MODE) {
    bass_set_arp_mode(value);
    return true;
  } else if (number==CC_BASS_ONLY_NOTE_HELD) {
    bass_set_only_note_held(value>0);
  }
  return false;
}



// get strings about the current scale/sequence/chords settings
char *get_bass_info() {
  static char output[20];

  sprintf(output, "s%i %i %i:%3s", scale_number, sequence_number, chord_number, get_note_name(bass_currently_playing).c_str());
  return output;
}

char *get_bass_info_2() {
  static char output[32];

  // auto scale, auto arp, auto progression, bass counter
  sprintf(output, "as aa ap bc\n%c  %c  %c  %2i\n", bass_auto_scale ? 'Y' : 'N', bass_auto_arp ? 'Y' : 'N', bass_auto_progression ? 'Y' : 'N', bass_counter);
  return output;
}

#endif

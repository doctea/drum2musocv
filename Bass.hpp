#ifndef BASS_INCLUDED
#define BASS_INCLUDED

// messages targeted to channel _IN will be relayed on channel _OUT -- for passing through messages to Neutron
#define MIDI_CHANNEL_BASS_IN   8
#define MIDI_CHANNEL_BASS_AUTO_IN   9
#define MIDI_CHANNEL_BASS_OUT  2


#define MIDI_C2   32
#define MIDI_C3   48
#define MIDI_C4   60  // middle C
#define MIDI_C5   72
#define MIDI_C6   88
#define MIDI_C7   104

#define MIDI_BASS_ROOT_PITCH  MIDI_C3

#include "MidiInput.hpp"
#include "BassHeld.hpp"

//#define BASS_DEBUG

// handling debugging output - pattern from https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c/1644898#1644898
#ifdef BASS_DEBUG
#define BASS_DEBUG 1
#else
#define BASS_DEBUG 0
#endif

#define BASS_printf(fmt, ...)   do { if (BASS_DEBUG) Serial.printf((fmt), ##__VA_ARGS__); } while (0)
#define BASS_println(fmt, ...)  do { if (BASS_DEBUG) Serial.println((fmt), ##__VA_ARGS__); } while (0)

#define SCALE_SIZE  7

int bass_scale_offset[][SCALE_SIZE] = {
  { 0, 2, 4, 5, 7, 9, 11 },  // major scale
  { 0, 2, 3, 5, 7, 8, 10 },  // // minor scale (? check)
};
int bass_sequence[][4]     =   { // degrees of scale to play per chord
  { 0, 0, 0, 0 },
  { 0, 0, 0, 1 },
  { 0, 1, 1, 2 },
  { 0, 1, 0, 4 },
  //{ 0, 2, 4, 6 },
  /*{ 0, 2, 0, 4 },
    { 0, 3, 6, 4 }*/
};
int chord_sequence[]    =   { 0, 5, 1, 4 };     // chord progression

#define BASS_NUM_SCALES             (sizeof(bass_scale_offset) / sizeof(bass_scale_offset[0]))
#define BASS_NUM_SEQUENCES          (sizeof(bass_sequence) / sizeof(bass_sequence[0]))
#define BASS_SCALE_SIZE             ((int)(sizeof(bass_scale_offset[0])/sizeof(bass_scale_offset[0][0])))
#define BASS_SEQUENCE_LENGTH        ((int)(sizeof(bass_sequence[0])/sizeof(bass_sequence[0][0])))
#define BASS_CHORD_SEQUENCE_LENGTH  ((int)(sizeof(chord_sequence)/sizeof(chord_sequence[0])))

bool bass_auto_note_held = false;

//int bass_root = MIDI_BASS_ROOT_PITCH;

int bass_counter = 0;               // track current position in sequence
int bass_currently_playing = -1;    // track currently playing note so that we know which one to turn off

int scale_number = 0;
int chord_number = 0;
int sequence_number = 0;

bool bass_auto_scale = false;
bool bass_auto_progression = false;
bool bass_auto_arp = true;

//int bass_sequence[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, -7, -6, -5, -4, -3, -2, -1};
//int bass_sequence[]   =   { 0, 5, -5, 10 };
//int bass_sequence[]   =   { 0, 4, 3, 1 };   // basically chord shape during this phase of development

/*
  -14,-13,-12,-11,-10,-9, -8 = octave -2
  -7, -6, -5, -4, -3, -2, -1 = octave -1
  0,   1,  2,  3,  4,  5,  6 = octave 0
  7,   8,  9, 10, 11, 12, 13 = octave +1
  14, 15, 16, 17, 18, 19, 20 = octave +2
*/

void debug_bass_scales() {
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

int get_scale_number() {
  if (bass_auto_scale) {
    return scale_number = (0 + current_phrase) % BASS_NUM_SCALES; // todo: make this switchable ..
  } else {
    return 0; // default to major
  }
}
int get_chord_number() {
  if (bass_auto_progression) {
    return chord_sequence[current_bar % BASS_CHORD_SEQUENCE_LENGTH] % BASS_SCALE_SIZE; // todo: make this select from lowest held note?
  } else {
    return 0;
  }
}

int bass_get_scale_note(int scale_degree = 0) {

  // todo: move this elsewhere to make this changeable..
  //int scale_number = 0;
  scale_number = get_scale_number();
  chord_number = get_chord_number();

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

int bass_get_sequence_note(int position = 0) {
  if (position % BASS_SEQUENCE_LENGTH == 0) BASS_printf("----- bass sequence restart (position %i)\r\n", position);
  if (bass_auto_arp) {
    sequence_number = current_bar % BASS_NUM_SEQUENCES;
  } else {
    sequence_number = 0;
  }

  return bass_get_scale_note(bass_sequence[sequence_number][position % BASS_SEQUENCE_LENGTH]);
}

int bass_get_sequence_pitch(int position = 0) {
  //bass_root = MIDI_BASS_ROOT_PITCH;// + current_phrase;
  if (is_bass_auto_note_held()) {
    return bass_get_sequence_held_note(position);
  }
  return get_bass_root_note() + bass_get_sequence_note(position);
}

void bass_reset_sequence() {
  bass_counter = 0;
}

// start note for
void bass_note_on (int v = 127) {
  int pitch_offset = bass_get_sequence_pitch(bass_counter); // current_beat); // todo: configurable options to change based on bass_counter or to keep synced to the current_beat number
  //BASS_printf("bass_note_on: bass pitch is %i\r\n", pitch_offset);

  bass_currently_playing = pitch_offset;
  MIDIOUT.sendNoteOn(bass_currently_playing, v, MIDI_CHANNEL_BASS_OUT);
}

void bass_note_on_and_next(int v = 127) {
  bass_note_on(v);

  bass_counter++;
}

void bass_note_off() {
  /*int last_bass_position = bass_counter-1%BASS_SEQUENCE_LENGTH;
    if (last_bass_position<0) last_bass_position = BASS_SEQUENCE_LENGTH + last_bass_position;
    int pitch_offset = bass_sequence[last_bass_position%BASS_SEQUENCE_LENGTH];*/
  //BASS_printf("bass_note_off: bass pitch offset is %i\r\n", bass_currently_playing);

  //MIDIOUT.sendNoteOff(MIDI_BASS_ROOT_PITCH + pitch_offset, v, MIDI_CHANNEL_BASS_OUT);
  if (bass_currently_playing >= 0)
    MIDIOUT.sendNoteOff(bass_currently_playing, 0, MIDI_CHANNEL_BASS_OUT);
  bass_currently_playing = -1;
}

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

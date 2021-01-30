#ifndef BASS_INCLUDED
#define BASS_INCLUDED

#include "MidiInput.hpp"

//#define BASS_DEBUG

// handling debugging output - pattern from https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c/1644898#1644898
#ifdef BASS_DEBUG
#define BASS_DEBUG 1
#else
#define BASS_DEBUG 0
#endif

#define BASS_printf(fmt, ...)   do { if (BASS_DEBUG) Serial.printf((fmt), ##__VA_ARGS__); } while (0)
#define BASS_println(fmt, ...)  do { if (BASS_DEBUG) Serial.println((fmt), ##__VA_ARGS__); } while (0)


int bass_scale_offset[] = { 0, 2, 4, 5, 7, 9, 11 } ; //, 12 };
#define BASS_SCALE_SIZE       ((int)(sizeof(bass_scale_offset)/sizeof(bass_scale_offset[0])))


//int bass_sequence[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, -7, -6, -5, -4, -3, -2, -1};
int bass_sequence[]   =   { 0, 5, -5, 10 };
#define BASS_SEQUENCE_LENGTH  ((int)(sizeof(bass_sequence)/sizeof(bass_sequence[0])))

int bass_counter = 0;               // track current position in sequence 
int bass_currently_playing = -1;    // track currently playing note so that we know which one to turn off 

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
    if (sd<0) oct--;
    Serial.printf("octave is %i, ", oct);

    Serial.println("");
  }  
  Serial.println("-------");
}


void initialise_bass() {
  debug_bass_scales();
}

int bass_get_scale_note(int scale_degree = 0) {
  int sd = scale_degree % BASS_SCALE_SIZE;
  int oct = scale_degree/BASS_SCALE_SIZE;

  BASS_printf("bass_get_scale: for scale degree %i, got scale offset %i and octave %i - ", scale_degree, sd, oct);
  if (sd<0) {
    sd = BASS_SCALE_SIZE + sd;  // convert to the actual scale degree
    oct--;                      // account for needing to be one octave below
  }
  BASS_printf("final oct:sd is %i:%i\r\n", oct, sd);
    
  return bass_scale_offset[sd] 
         + 
         (oct * 12)
         ;
}

int bass_get_sequence_note(int position = 0) {
  return bass_get_scale_note(bass_sequence[position%BASS_SEQUENCE_LENGTH]);
}

int bass_get_sequence_pitch(int position = 0) {
  return MIDI_BASS_ROOT_PITCH + bass_get_sequence_note(position);
}

void bass_reset_sequence() {
  bass_counter = 0;
}

// start note for
void bass_note_on (int v = 127) {
  int pitch_offset = bass_get_sequence_pitch(bass_counter);
  BASS_printf("bass_note_on: bass pitch is %i\r\n", pitch_offset);

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
  BASS_printf("bass_note_off: bass pitch offset is %i\r\n", bass_currently_playing);

  //MIDIOUT.sendNoteOff(MIDI_BASS_ROOT_PITCH + pitch_offset, v, MIDI_CHANNEL_BASS_OUT);
  if (bass_currently_playing>=0) 
    MIDIOUT.sendNoteOff(bass_currently_playing, 0, MIDI_CHANNEL_BASS_OUT);
  bass_currently_playing = -1;
}

#endif

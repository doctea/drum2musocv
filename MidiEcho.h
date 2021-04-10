#ifndef MIDIECHO_INCLUDED
#define MIDIECHO_INCLUDED

#include "Drums.h"
#include "Euclidian.h"

int get_pitch_for_trigger(int trigger);
int get_trigger_for_pitch(int pitch);
void echo_fire_trigger(int trigger, int velocity);
void echo_douse_trigger(int trigger, int velocity);
bool handle_midiecho_ccs(int channel, int number, int value);
void initialise_pitch_for_triggers();

bool midiecho_enabled = true;

int pitch_for_trigger[NUM_TRIGGERS + NUM_ENVELOPES];

void debug_pitch_for_trigger() {
  Serial.printf("=== trigger-pitch mapping for mode %i ===\r\n", MUSO_MODE);
  for (int i = 0 ; i < NUM_TRIGGERS ; i++) {
    Serial.printf("   trigger[%i] = pitch %i\r\n", i, pitch_for_trigger[i]);
  }
  Serial.printf("^^^ trigger-pitch mappings ^^^");
}

//String get_note_name(int pitch);

// TODO: move all this stuff below into some generic midi header
String get_note_name(int pitch);

#define MIDI_C2   32
#define MIDI_C3   48
#define MIDI_C4   60  // middle C
#define MIDI_C5   72
#define MIDI_C6   88
#define MIDI_C7   104

#define MIDI_BASS_ROOT_PITCH  MIDI_C3

#define CC_MIDIECHO_ENABLED   21      // enable echoing of output back to DAW over USB

// from https://github.com/FortySevenEffects/arduino_midi_library/blob/master/src/midi_Defs.h
/*! Enumeration of MIDI types */
/*enum MidiType: uint8_t
{
    InvalidType           = 0x00,    ///< For notifying errors
    NoteOff               = 0x80,    ///< Channel Message - Note Off
    NoteOn                = 0x90,    ///< Channel Message - Note On
    AfterTouchPoly        = 0xA0,    ///< Channel Message - Polyphonic AfterTouch
    ControlChange         = 0xB0,    ///< Channel Message - Control Change / Channel Mode
    ProgramChange         = 0xC0,    ///< Channel Message - Program Change
    AfterTouchChannel     = 0xD0,    ///< Channel Message - Channel (monophonic) AfterTouch
    PitchBend             = 0xE0,    ///< Channel Message - Pitch Bend
    SystemExclusive       = 0xF0,    ///< System Exclusive
  SystemExclusiveStart  = SystemExclusive,   ///< System Exclusive Start
    TimeCodeQuarterFrame  = 0xF1,    ///< System Common - MIDI Time Code Quarter Frame
    SongPosition          = 0xF2,    ///< System Common - Song Position Pointer
    SongSelect            = 0xF3,    ///< System Common - Song Select
    Undefined_F4          = 0xF4,
    Undefined_F5          = 0xF5,
    TuneRequest           = 0xF6,    ///< System Common - Tune Request
  SystemExclusiveEnd    = 0xF7,    ///< System Exclusive End
    Clock                 = 0xF8,    ///< System Real Time - Timing Clock
    Undefined_F9          = 0xF9,
    Tick                  = Undefined_F9, ///< System Real Time - Timing Tick (1 tick = 10 milliseconds)
    Start                 = 0xFA,    ///< System Real Time - Start
    Continue              = 0xFB,    ///< System Real Time - Continue
    Stop                  = 0xFC,    ///< System Real Time - Stop
    Undefined_FD          = 0xFD,
    ActiveSensing         = 0xFE,    ///< System Real Time - Active Sensing
    SystemReset           = 0xFF,    ///< System Real Time - System Reset
};*/

#endif

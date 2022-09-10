#ifndef MIDIOUTPUT_INCLUDED
#define MIDIOUTPUT_INCLUDED

#include "Config.h"

#include "UI.h"

#define WORKAROUND_MISSED_NOTEOFFS_DELAY 12  // 8 seems to be about the shortest time we can get away with, altho it still gets confused at times
#define WORKAROUND_MISSED_NOTEOFFS true     // DANGER -- uses a delay() in order to avoid things fucking up when sending lots of douse_triggers in a loop
  // weirdly, the midimuso seems to miss note offs sent for Harmony outputs if lots of note offs are sent rapidly to the bitbox *drums* output, 
  // as happens when stopping play or switching autoplay on/off rapidly etc

//#define OUT_DEBUG
#ifdef OUT_DEBUG
#define OUT_DEBUG 1
#else
#define OUT_DEBUG 0
#endif

// https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c/1644898#1644898
#define OUT_printf(fmt, ...) do { if (OUT_DEBUG) Serial.printf((fmt), ##__VA_ARGS__); } while (0)
#define OUT_println(fmt, ...) do { if (OUT_DEBUG) Serial.println((fmt), ##__VA_ARGS__); } while (0)

//BITBOX/melody settings
#define BITBOX_NOTE_MINIMUM         36  // https://1010music.com/wp-content/uploads/2020/08/bitbox-mk2-1.0.8-user-manual.pdf "MIDI inputs for notes 36 to 51 map to the pads", "EXT1 through EXT4 are assigned notes 55 to 52 for use as Recording triggers"

#define MIDI_CHANNEL_BITBOX_DRUMS_OUT (midi_channel_bitbox_drums_out)
extern int midi_channel_bitbox_drums_out;

//int midi_channel_bitbox_drums_out = DEFAULT_MIDI_CHANNEL_BITBOX_OUT;

#include "MidiSetup.hpp"
#include "Envelopes.h"    // for access to envelope info
#include "Harmony.hpp"    // for access to the harmony channel info
#include "Euclidian.h"    // cos we need to know the the number of patterns (actually we dont but..)
#include "ClockTriggerOutput.hpp"

int get_muso_pitch_for_trigger(int trigger);

// get the MIDIMUSO output pitch to use for this trigger
// take into account that some outputs may be missing or moved when in different muso modes and swap them appropriately
int get_muso_pitch_for_trigger_actual(int trigger)
;
#define LEFT_FOOT 0
#define RIGHT_FOOT 1
#define LEFT_HAND 2
#define RIGHT_HAND LEFT_HAND //3

extern int limb_count[4];
extern int limbs[NUM_PATTERNS];

void initialise_pitch_for_trigger_table ();
// functions for sending MIDI out

//static int i = 0;
void fire_trigger(byte trigger, byte velocity, bool internal = false);
void douse_trigger(byte trigger, byte velocity = 0, bool internal = false, bool tied = false) ;
void douse_all_triggers(bool internal = false);
void midi_send_envelope_level(byte envelope, byte level);
void midi_kill_notes_bitbox_drums();
void midi_kill_notes_muso_drums();

void midi_kill_notes();

void kill_notes();

#define DEBUG_CLOCK false

extern unsigned long last_clock_ticked;
extern unsigned long last_clock_ticked_millis;
void midi_send_clock(unsigned long received_ticks);

#endif

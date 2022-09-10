#ifndef MIDIINPUT_INCLUDED
#define MIDIINPUT_INCLUDED

#define RELAY_PROGRAM_CHANGE false   // only enable passing through program changes when we know its safe ie when only one midimuso is connected

#include "MidiSetup.hpp"
#include "Drums.h"
#include "MidiOutput.hpp"

// GLOBALS

extern byte activeNotes;             // tracking how many keys are held down
extern unsigned long last_input_at;  // timestamp we last received midi from host

// DEFINES

//TODO: make these CC values sensible and map them in FL
#define CC_SYNC_RATIO         110   // sync ratio of the envelope modulation in ticks, default 24
#define CC_CLOCK_TICK_RATIO   111   // send a clock tick for every X clock ticks, default 1

// IMPORTS

#include "MidiEcho.h"
#include "BPM.hpp"
#include "Envelopes.h"
#include "Euclidian.h"
#include "UI.h"

//#include "autobass_input.hpp"

// MIDI MESSAGE CALLBACKS

void handleNoteOff(byte channel, byte pitch, byte velocity);
void handleNoteOn(byte channel, byte pitch, byte velocity);
void handleControlChange(byte channel, byte number, byte value);
void handleSongPosition(unsigned int steps);

void handleClock();
void handleStart();
void handleContinue();
void handleStop();
void handleSystemExclusive(byte* array, unsigned size);
void handleProgramChange(byte channel, byte pcm);
// called every loop(), to read incoming midi and route if appropriate
void process_midi();

void setup_midi();

#endif

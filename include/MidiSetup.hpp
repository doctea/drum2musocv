#ifndef MIDISETUP_INCLUDED
#define MIDISETUP_INCLUDED
// SET UP MIDI PORTS

#include "Config.h"
#include <Arduino.h>
#include <MIDI.h>
#include <USB-MIDI.h> 

#define PPQN  24  // midi clock ticks per quarter-note -- ie length in ticks of 1 beat


#define MIDIOUT midiB
#define MIDIIN  MIDICoreUSB

extern midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> MIDIOUT;
extern midi::MidiInterface<usbMidi::usbMidiTransport> MIDIIN;

#endif

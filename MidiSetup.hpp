#ifndef MIDISETUP_INCLUDED
#define MIDISETUP_INCLUDED
// SET UP MIDI PORTS

#include "Config.h"

#define PPQN  24  // midi clock ticks per quarter-note -- ie length in ticks of 1 beat

#ifdef USB_NATIVE  // use native usb version, eg for seeduino or (presumably) other boards with Native USB support

#include <USB-MIDI.h> 
typedef USBMIDI_NAMESPACE::usbMidiTransport __umt;
typedef MIDI_NAMESPACE::MidiInterface<__umt> __ss;
__umt usbMIDI(0); // cableNr
__ss MIDICoreUSB((__umt&)usbMIDI);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, midiB);

#define MIDIOUT midiB
#define MIDIIN  MIDICoreUSB

#else              // arduino uno / serial midi version (for USBMidiKlik)

MIDI_CREATE_DEFAULT_INSTANCE();
#define MIDIOUT MIDI
#define MIDIIN  MIDI

#endif

#endif

#ifndef MIDIINPUT_INCLUDED
#define MIDIINPUT_INCLUDED

#include "Drums.h"

// GLOBALS

byte activeNotes = 0;             // tracking how many keys are held down
unsigned long last_input_at = 0;  // timestamp we last received midi from host

// DEFINES

//TODO: make these CC values sensible and map them in FL
#define CC_SYNC_RATIO   110

// IMPORTS

#include "MidiEcho.h"
#include "BPM.hpp"
#include "Envelopes.h"

// SET UP MIDI PORTS

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

// FUNCTIONS


void kill_notes() {
  // forget which triggers are active (doesn't actually send stop notes)
  for (int i = 0 ; i < NUM_TRIGGERS ; i++) {
    trigger_status[i] = TRIGGER_IS_OFF;
  }
  activeNotes = 0;
}

void fire_trigger(byte p, byte v, bool internal = false) { // p = keyboard note
  //Serial.printf("firing trigger %i\r\n", p);
    if (
      p>=MUSO_NOTE_MINIMUM && 
      p<=MUSO_NOTE_MAXIMUM) {
        trigger_status[p - MUSO_NOTE_MINIMUM] = v>0; // TRIGGER_IS_ON;
        MIDIOUT.sendNoteOn(p, v, MUSO_GATE_CHANNEL); //CHANNEL_DRUMS);  // output channel that the midimuso expects its triggers on
    } else if (
      //Serial.printf("is an envelope trigger!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      p>MUSO_NOTE_MAXIMUM && 
      p<=MUSO_NOTE_MAXIMUM + NUM_ENVELOPES) {
        update_envelope (p - (MUSO_NOTE_MAXIMUM), 127, true);
    }
#ifdef ENABLE_MIDI_ECHO
    if (internal) echo_fire_trigger(p-MUSO_NOTE_MINIMUM, v);
#endif
}
void douse_trigger(byte p, byte v, bool internal = false) {
    if (
      p>=MUSO_NOTE_MINIMUM && 
      p<=MUSO_NOTE_MAXIMUM) {
        trigger_status[p - MUSO_NOTE_MINIMUM] = TRIGGER_IS_OFF;
        MIDIOUT.sendNoteOff(p, v, MUSO_GATE_CHANNEL);   // hardcoded channel 16 for midimuso
    } else if (
      p>MUSO_NOTE_MAXIMUM && 
      p<=MUSO_NOTE_MAXIMUM + NUM_ENVELOPES) {
        update_envelope (p - (MUSO_NOTE_MAXIMUM), 127, false);
    }
#ifdef ENABLE_MIDI_ECHO
    if (internal) echo_douse_trigger(p-MUSO_NOTE_MINIMUM, v);
#endif
}


// MIDI MESSAGE CALLBACKS

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  byte p = pitch;
  byte v = velocity;

  activeNotes--;
  /*if (!process_triggers_for_pitch(p, v, false)) {
    p = convert_drum_pitch(p);
    douse_trigger(p, 0);
  }*/
  fire_trigger(MUSO_NOTE_MINIMUM+get_trigger_for_pitch(p), 0);
  last_input_at = millis();
}

void handleNoteOn(byte channel, byte pitch, byte velocity) {
  byte p = pitch;
  byte v = velocity;

  if (v==0) 
    handleNoteOff(channel, p, v);

  activeNotes++;
  /*if (!process_triggers_for_pitch(p, v, true)) {
    p = convert_drum_pitch(p);
    fire_trigger(p, v);
  }*/
  fire_trigger(MUSO_NOTE_MINIMUM+get_trigger_for_pitch(p), v);
  last_input_at = millis();
}


void handleControlChange(byte channel, byte number, byte value) {
  // pass thru control changes, shifted to channel 1
  // TODO: intercept our own control messages to do things like set envelope settings, LFO settings, trigger targets/choke linking..
  if (number==CC_SYNC_RATIO) {
    cc_value_sync_modifier = constrain(value,1,127); //1 + (value-1); // minimum of 1    
  } else if (!handle_envelope_ccs(channel, number, value)) {
    //MIDI.sendControlChange(number, value, 1); // pass thru unhandled CV
  }
  last_input_at = millis();
}

void handleSongPosition(unsigned int beats) {
  // TODO: put LFO / envelope timer into correct phase, if that is possible?

  // TODO: reset position to align with incoming clock...
  //    ticks_received = beats * PPQN ?
  // set the clock_count to an appropriate value in handle SongPosition ? clock_count = beats * 24 or somesuch, if its 24pqn ? beat is a 16th note i think tho so would it be * 96 ? need to check this
  song_position = beats/4;  // TODO: make this work with the BPM handling...
  last_input_at = millis();  
}

void handleClock() {
  //MIDIOUT.sendClock();  // do this in clock_tick now so we can control subdivisions
  
  bpm_receive_clock_tick();
}

void handleStart() {
  // TODO: start LFOs?
  MIDIOUT.sendStart();
  
  bpm_reset_clock();
}
void handleContinue() {
  // TODO: continue LFOs
  MIDIOUT.sendContinue();
  kill_envelopes();
}
void handleStop() {
  MIDIOUT.sendStop();
  // TODO: stop+reset LFOs
  Serial.println("Received STOP -- killing envelopes / resetting clock !");
  kill_envelopes();
  bpm_reset_clock(-1);  // -1 to make sure next tick is treated as first step of beat
  
#ifdef ENABLE_PIXELS
  kill_notes();
#endif

}

void handleSystemExclusive(byte* array, unsigned size) {
  // pass sysex messages through to the midimuso
  // TODO: configuration of drum2musocv via sysex?
  MIDIOUT.sendSysEx(size, array, false); // true/false means "array contains start/stop padding" -- think what we receive here is without padding..?
}

void midi_send_clock() {
  MIDIOUT.sendClock();
}


void setup_midi() {
  
#ifdef USB_NATIVE
  Serial.begin(115200);   // usb serial debug port
  //while (!Serial);
  
  MIDIOUT.begin(GM_CHANNEL_DRUMS);
#endif
  MIDIIN.begin(GM_CHANNEL_DRUMS);


  MIDIIN.turnThruOff();

  MIDIIN.setHandleNoteOn(handleNoteOn);
  MIDIIN.setHandleNoteOff(handleNoteOff);

  MIDIIN.setHandleControlChange(handleControlChange);

  MIDIIN.setHandleStop(handleStop);
  MIDIIN.setHandleStart(handleStart);
  MIDIIN.setHandleStart(handleContinue);

  MIDIIN.setHandleClock(handleClock);

  MIDIIN.setHandleSongPosition(handleSongPosition);

}

#endif

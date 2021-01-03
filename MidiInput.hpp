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

void fire_trigger(byte p, byte v, bool internal = false) { // p = keyboard note
  //Serial.printf("firing trigger %i\r\n", p);
    if (
      p>=MUSO_NOTE_MINIMUM && 
      p<MUSO_NOTE_MAXIMUM) {
        trigger_status[p - MUSO_NOTE_MINIMUM] = v>0; // TRIGGER_IS_ON;
        MIDIOUT.sendNoteOn(p, v, MUSO_GATE_CHANNEL); //CHANNEL_DRUMS);  // output channel that the midimuso expects its triggers on
    } else if (
      //Serial.printf("is an envelope trigger!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      p>=MUSO_NOTE_MAXIMUM && 
      p<MUSO_NOTE_MAXIMUM + NUM_ENVELOPES) {
        update_envelope (p - (MUSO_NOTE_MAXIMUM), v, true);
    }
#ifdef ENABLE_MIDI_ECHO
    if (internal) echo_fire_trigger(p-MUSO_NOTE_MINIMUM, v);
#endif
}

void douse_trigger(byte p, byte v, bool internal = false) {
    if (
      p>=MUSO_NOTE_MINIMUM && 
      p<MUSO_NOTE_MAXIMUM) {
        trigger_status[p - MUSO_NOTE_MINIMUM] = TRIGGER_IS_OFF;
        MIDIOUT.sendNoteOff(p, v, MUSO_GATE_CHANNEL);   // hardcoded channel 16 for midimuso
    } else if (
      p>=MUSO_NOTE_MAXIMUM && 
      p<MUSO_NOTE_MAXIMUM + NUM_ENVELOPES) {
        update_envelope (p - (MUSO_NOTE_MAXIMUM), 0, false);
    }
#ifdef ENABLE_MIDI_ECHO
    if (internal) echo_douse_trigger(p-MUSO_NOTE_MINIMUM, v);
#endif
}



void midi_kill_notes() {
  MIDIOUT.sendControlChange(123,0,MUSO_GATE_CHANNEL);
}

void kill_notes() {
  // forget which triggers are active (doesn't actually send stop notes)
  for (int i = 0 ; i < NUM_TRIGGERS+NUM_ENVELOPES ; i++) {
    trigger_status[i] = TRIGGER_IS_OFF;
    douse_trigger(MUSO_NOTE_MINIMUM+i, 0);
  }
  activeNotes = 0;

  midi_kill_notes();
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
  douse_trigger(MUSO_NOTE_MINIMUM+get_trigger_for_pitch(p), 0);
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


// -----------------------------------------------------------------------------
/*
byte convert_drum_pitch(byte pitch) {
  // in mode 0x0b there are 11 triggers available and a pitch out
  byte p;
  if (pitch >= GM_NOTE_MINIMUM && pitch <= GM_NOTE_MAXIMUM) {   // only process notes within GM drumkit range
    p = pitch;
    switch (pitch) {
      // right-hand column (triggers 1-5 running down)
      case GM_NOTE_ELECTRIC_BASS_DRUM:  p = MUSO_NOTE_GATE_1;   break; //Electric Bass Drum - C5 72
      case GM_NOTE_SIDE_STICK:          p = MUSO_NOTE_GATE_2;   break; //Side Stick - C#5/Db5 73
      case GM_NOTE_HAND_CLAP:           p = MUSO_NOTE_GATE_3;   break; //Hand Clap - D5 74
      case GM_NOTE_ELECTRIC_SNARE:      p = MUSO_NOTE_GATE_4;   break; //Electric Snare - D#5/Eb5 75
      case GM_NOTE_CRASH_CYMBAL_1:      p = MUSO_NOTE_GATE_5;   break; //Crash Cymbal 1 - F#5/Gb5 78
      
      // left-hand column (triggers 11-6 running down)
      case GM_NOTE_CLOSED_HI_HAT:       p = MUSO_NOTE_GATE_11;  break; //Closed Hi-hat - E5 76
      case GM_NOTE_OPEN_HI_HAT:         p = MUSO_NOTE_GATE_10;  break; //77 //Open Hi-hat - F5
      case GM_NOTE_PEDAL_HI_HAT:        p = MUSO_NOTE_GATE_9;   break; //Pedal Hi-hat - A#5/Bb5 82

      case GM_NOTE_LOW_TOM:             p = MUSO_NOTE_GATE_8;   break; //79 //Low Tom - G5
      case GM_NOTE_HIGH_TOM:            p = MUSO_NOTE_GATE_7;   break; //High Tom - G#5/Ab5 80
      case GM_NOTE_TAMBOURINE:          p = MUSO_NOTE_GATE_6;   break; //Tambourine - A5 81
      //case GM_NOTE_RIDE_BELL:           p = MUSO_GATE_12; break; //Ride Bell - B5 83  // there is no gate 12!
      //default: p = pitch + 12; //itch = 72; break;
      default:                          p = pitch;              break; // pass thru other notes unmodified
    }
  }
  return p;
}



bool process_triggers_for_pitch(byte pitch, byte velocity, bool state) {
  // in the 0x0b midimuso-cv mode, there are 5 CV outputs and a clock output
  // the mapping is currently all hardcoded here and in Drums.h
  // TODO: some way to link envelopes/triggers so as to be able to 'cut by' or choke/release hihats
  byte p;
  int trig = get_trigger_for_pitch(pitch);
  if (trig>NUM_TRIGGERS) {
    update_envelope(trig-NUM_TRIGGERS, velocity, state);
    return true;
  }
  return false;

  // previously ..
  switch (pitch) {
    //case GM_NOTE_PEDAL_HI_HAT:  // TODO: figure out how to link pedal hihat with envelopes so as to choke?  would need an envelope dedicated to the open hats i guess...
    //  return true 
    //  break;
    case GM_NOTE_CRASH_CYMBAL_2:  // cymbal crash 2
      // trigger envelope
      update_envelope(ENV_CRASH, velocity, state);
      return true;
      break;
    case GM_NOTE_SPLASH_CYMBAL:  // splash cymbal
      update_envelope(ENV_SPLASH, velocity, state);
      return true;
      break;
      //case GM_NOTE_:  // TODO: add more envelopes
      //  break;
    case GM_NOTE_VIBRA_SLAP:    
      update_envelope(ENV_WOBBLY, velocity, state);
      return true;
    case GM_NOTE_RIDE_BELL:
      update_envelope(ENV_RIDE_BELL, velocity, state);
      return true;
    case GM_NOTE_RIDE_CYMBAL_1:
      update_envelope(ENV_RIDE_CYMBAL, velocity, state);
      return true;
  }
  return false;
}
*/



#endif

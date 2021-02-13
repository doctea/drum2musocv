#ifndef MIDIINPUT_INCLUDED
#define MIDIINPUT_INCLUDED

#include "MidiSetup.hpp"
#include "Drums.h"
#include "MidiOutput.hpp"


// GLOBALS

byte activeNotes = 0;             // tracking how many keys are held down
unsigned long last_input_at = 0;  // timestamp we last received midi from host

// DEFINES

//TODO: make these CC values sensible and map them in FL
#define CC_SYNC_RATIO   110
#define CC_CLOCK_TICK_RATIO   111



// IMPORTS

#include "MidiEcho.h"
#include "BPM.hpp"
#include "Envelopes.h"
#include "Euclidian.h"
#include "UI.h"

//#include "Bass.hpp"

// MIDI MESSAGE CALLBACKS

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  if (channel==GM_CHANNEL_DRUMS) {
    byte p = pitch;
    byte v = velocity;
  
    activeNotes--;
    /*if (!process_triggers_for_pitch(p, v, false)) {
      p = convert_drum_pitch(p);
      douse_trigger(p, 0);
    }*/
    douse_trigger(get_trigger_for_pitch(p), 0);
    last_input_at = millis();
  }
}

void handleNoteOn(byte channel, byte pitch, byte velocity) {
  if (channel==GM_CHANNEL_DRUMS) {
    byte p = pitch;
    byte v = velocity;
  
    if (v==0) 
      handleNoteOff(channel, p, v);
  
    activeNotes++;
    /*if (!process_triggers_for_pitch(p, v, true)) {
      p = convert_drum_pitch(p);
      fire_trigger(p, v);
    }*/
    fire_trigger(get_trigger_for_pitch(p), v);
    last_input_at = millis();
  }
}


void handleControlChange(byte channel, byte number, byte value) {
  if (channel==GM_CHANNEL_DRUMS) {
    // pass thru control changes, shifted to channel 1
    // TODO: intercept our own control messages to do things like set envelope settings, LFO settings, trigger targets/choke linking..
    if (number==CC_SYNC_RATIO) {
      cc_value_sync_modifier = constrain(1+value,1,128); //add 1 !
      // "number of real ticks per 24 pseudoticks"? 24 = 1:1, 121 = 1:2, 48 = 2:1, 96 = 4:1 i think?
      // note actual CC value is 1 less than the intended value!
    } else if (number==CC_CLOCK_TICK_RATIO) {
      cc_value_clock_tick_ratio = constrain(value,0,127);
    } else if (handle_envelope_ccs(channel, number, value)) {
      //MIDI.sendControlChange(number, value, 1); // pass thru unhandled CV
    } else if (handle_euclidian_ccs(channel, number, value)) {

    } else if (handle_bass_ccs(channel, number, value)) {
      Serial.printf("Handled bass cc channel %i number %i value %i\r\n", channel, number, value);      
    } else if (handle_ui_ccs(channel, number, value)) {

    } else if (handle_midiecho_ccs(channel, number, value)) {
            
    } else {
      //MIDI.sendControlChange(number, value, 1); // pass thru unhandled CV
    }
    last_input_at = millis();
  } /*(else {
    MIDIOUT.sendControlChange(
  }(*/
}

void handleSongPosition(unsigned int steps) {
  // A type of MIDI message held in a MIDI sequencer or synchronizer telling a connected device how many 16th notes have elapsed since the beginning of a song. 
  // TODO: put LFO / envelope timer into correct phase, if that is possible?

  // TODO: reset position to align with incoming clock...
  //    ticks_received = beats * PPQN ?
  // set the clock_count to an appropriate value in handle SongPosition ? clock_count = beats * 24 or somesuch, if its 24pqn ? beat is a 16th note i think tho so would it be * 96 ? need to check this
  song_position = steps/4;  
  
  received_ticks = steps * TICKS_PER_STEP;    // this should work with the BPM handling...?
  bpm_update_status(received_ticks-1);

  //Serial.printf("Received song position of %i received_ticks which is %i steps ", received_ticks, steps);
  //Serial.printf("(current_step=%i) (and so should be song phrase %i?)\r\n",  current_step, current_phrase); //(received_ticks/(TICKS_PER_STEP*STEPS_PER_BEAT*BEATS_PER_BAR)) / BARS_PER_PHRASE;);
  
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
  Serial.println("Received STOP -- killing envelopes / notes / resetting clock !");
  douse_all_triggers();
  kill_envelopes();
  kill_notes();
  bpm_reset_clock(-1);  // -1 to make sure next tick is treated as first step of beat
  last_input_at = millis();
  
//#ifdef ENABLE_PIXELS
//  kill_notes();
//#endif

}

void handleSystemExclusive(byte* array, unsigned size) {
  // pass sysex messages through to the midimuso
  // TODO: configuration of drum2musocv via sysex?
  MIDIOUT.sendSysEx(size, array, false); // true/false means "array contains start/stop padding" -- think what we receive here is without padding..?
}


// called every loop(), to read incoming midi and route if appropriate
void process_midi() {
  if (MIDIIN.read()) {
    //Serial.printf("received message from MIDIIN, channel is %i: type is %i, ", MIDIIN.getChannel(), MIDIIN.getType()  );
    //Serial.printf("data1 is %i, data2 is %i\r\n", MIDIIN.getData1(), MIDIIN.getData2() );
    if (MIDIIN.getChannel()==MIDI_CHANNEL_BASS_IN) {
      // relay all incoming messages for the Neutron/bass
      //Serial.println("Received a message targeted at the neutron directly: %0x, %0x, %0x\n", MIDIIN.getType(), MIDIIN.getData1(), MIDIIN.getData2());
      MIDIOUT.send(MIDIIN.getType(),
                   MIDIIN.getData1(),
                   MIDIIN.getData2(),
                   MIDI_CHANNEL_BASS_OUT
      );
    } else if (MIDIIN.getChannel()==MIDI_CHANNEL_BASS_AUTO_IN) {
      if (MIDIIN.getType()==midi::MidiType::NoteOn) {
        bass.handle_note_on(MIDIIN.getData1(), MIDIIN.getData2());
      } else if (MIDIIN.getType()==midi::MidiType::NoteOff) {
        bass.handle_note_off(MIDIIN.getData1());
        if (!bass.is_note_held()) { // if this has meant all nodes have turned off, kill the existing note
          bass_note_off();
        }
      }
    }
    //todo: accept a note on another channel to set the root..?
    //      or actually, have CCs to set the root note, scale, etc..?
    // have separate midi input channels, one dedicated to triggering the autoplayer, and one for doing direct control, both output to the MIDI_CHANNEL_BASS_OUT
    //      so root note set by MIDI CC (0-127?) ie - 'middle C' or 'high F'
    //      another CC to set the scale (0 = major, 1 = minor, ...etc...)
    //      on the 'autobass' channel, track what keys are held
    //        lowest held note relative to scale root = chord number, so when triggered play appropriate note from the chord number
    //          if note isn't in scale then????
    //        highest held note relative to scale root = arp distance?  
    //      orrr
    //        lowest held note relative to scale root = chord number
    //          further held notes (in order of pressing?) = bass_sequence, used to play through sequence as needed..
    
  }
}

void setup_midi() {
  
#ifdef USB_NATIVE
  Serial.begin(115200);   // usb serial debug port
  //while (!Serial);
  
  MIDIOUT.begin(MIDI_CHANNEL_OMNI); //GM_CHANNEL_DRUMS);
#endif
  MIDIIN.begin(MIDI_CHANNEL_OMNI); //GM_CHANNEL_DRUMS);

  MIDIIN.turnThruOff();
  //MIDIIN.setThruFilterMode(midi::Thru::DifferentChannel);

  MIDIIN.setHandleNoteOn(handleNoteOn);
  MIDIIN.setHandleNoteOff(handleNoteOff);

  MIDIIN.setHandleControlChange(handleControlChange);

  MIDIIN.setHandleStop(handleStop);
  MIDIIN.setHandleStart(handleStart);
  MIDIIN.setHandleContinue(handleContinue);

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

#ifndef MIDIOUTPUT_INCLUDED
#define MIDIOUTPUT_INCLUDED

#include "MidiSetup.hpp"
#include "Envelopes.h"    // for access to envelope info
#include "Bass.hpp"       // for access to the bass channel info

// functions for sending MIDI out

void fire_trigger(byte t, byte v, bool internal = false) { 
    //Serial.printf("firing trigger pitch=%i, v=%i\r\n", p, v);
    // t = trigger number, p = keyboard note
    byte p = MUSO_NOTE_MINIMUM + t;
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
    } else if (p == MUSO_NOTE_MAXIMUM + NUM_ENVELOPES) {
      //Serial.printf(">> got BASS trigger!\r\n");
      //if (bass.is_note_held()) // todo: make this so that can still play bass when no DAW present...
        bass_note_on_and_next();
      //else 
      //  Serial.println("No note held? is_note_held is false");
    } else {
      Serial.printf("WARNING: fire_trigger not doing anything with pitch %i\r\n", p);
    }
#ifdef ENABLE_MIDI_ECHO
    if (internal) echo_fire_trigger(p-MUSO_NOTE_MINIMUM, v);
#endif
}

void douse_trigger(byte t, byte v, bool internal = false) {
    byte p = MUSO_NOTE_MINIMUM + t;
    if (
      p>=MUSO_NOTE_MINIMUM && 
      p<MUSO_NOTE_MAXIMUM) {
        trigger_status[p - MUSO_NOTE_MINIMUM] = TRIGGER_IS_OFF;
        MIDIOUT.sendNoteOff(p, v, MUSO_GATE_CHANNEL);   // hardcoded channel 16 for midimuso
    } else if (
      p>=MUSO_NOTE_MAXIMUM && 
      p<MUSO_NOTE_MAXIMUM + NUM_ENVELOPES) {
        update_envelope (p - (MUSO_NOTE_MAXIMUM), 0, false);
    } else if (p == MUSO_NOTE_MAXIMUM + NUM_ENVELOPES) {
      bass_note_off();
    } else {
      Serial.printf("WARNING: douse_trigger not doing anything with pitch %i\r\n", p);
    }
#ifdef ENABLE_MIDI_ECHO
    if (internal) echo_douse_trigger(p-MUSO_NOTE_MINIMUM, v);
#endif
}

void douse_all_triggers(bool internal = false) {
  for (int i = 0 ; i < NUM_TRIGGERS + NUM_ENVELOPES + 1 ; i++) {
    douse_trigger(i, 0, internal);
  }
}

void midi_send_envelope_level(byte envelope, byte level) {
  //Serial.printf("Envelope[%i] in stage %i: sending lvl %i to midi_cc %i!\r\n", envelope, envelopes[envelope].stage, level, envelopes[envelope].midi_cc);
  if (envelope==ENV_RIDE_CYMBAL) {  // hack to use the pitch bend output as an envelope, since my 'cc 74' output seems to have stopped working - could use this to add an extra envelope or LFO etc
    MIDIOUT.sendPitchBend((level<<7) + MIDI_PITCHBEND_MIN, MUSO_CV_CHANNEL);
  } else {
    MIDIOUT.sendControlChange(envelopes[envelope].midi_cc, level, MUSO_CV_CHANNEL); // send message to midimuso
  }
}

void midi_bass_send_note_on(int pitch, int velocity, int channel) {
  MIDIOUT.sendNoteOn(pitch, velocity, channel);
  // todo: echo back to host
}

void midi_bass_send_note_off(int pitch, int velocity, int channel) {
  MIDIOUT.sendNoteOff(pitch, velocity, channel);
  // todo: echo back to host
}

void midi_kill_notes() {
  MIDIOUT.sendControlChange(123,0,MUSO_GATE_CHANNEL); // todo -- check what this is actually doing/meant to do?!
  MIDIOUT.sendControlChange(123,0,MIDI_CHANNEL_BASS_OUT); // todo -- check what this is actually doing/meant to do?!
}

void kill_notes() {
  // forget which triggers are active 
  douse_all_triggers(true);
  //activeNotes = 0;

  midi_kill_notes();
}



void midi_send_clock(unsigned long received_ticks) {
  static unsigned long last_clock_ticked;
  if (received_ticks!=last_clock_ticked)
    MIDIOUT.sendClock();
  last_clock_ticked = received_ticks;
}


#endif

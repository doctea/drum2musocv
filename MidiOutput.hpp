#ifndef MIDIOUTPUT_INCLUDED
#define MIDIOUTPUT_INCLUDED

#define DEFAULT_MIDI_CHANNEL_BITBOX_OUT 11  // for the mirroring of drums
#define MIDI_CHANNEL_BITBOX_OUT (midi_channel_bitbox_out)

int midi_channel_bitbox_out = DEFAULT_MIDI_CHANNEL_BITBOX_OUT;

#include "MidiSetup.hpp"
#include "Envelopes.h"    // for access to envelope info
#include "Harmony.hpp"    // for access to the harmony channel info
#include "Euclidian.h"    // cos we need to know the the number of patterns (actually we dont but..)
#include "ClockTriggerOutput.hpp"


// get the output pitch to use for this trigger
// take into account that some outputs may be missing or moved when in different muso modes and swap them appropriately
int get_muso_pitch_for_trigger(int trigger) {
#if MUSO_MODE==MUSO_MODE_2B // pitches mode
  int gate;
  if (trigger==0) gate = 3;
  else if (trigger==1) gate = -1;
  else if (trigger==5 || trigger==6 || trigger==7) gate = -1;
  else if (trigger<=NUM_TRIGGERS) {
    if (trigger>7) 
      gate = trigger - 4; 
    else if (trigger>=2)
      gate = trigger - 2;
  }

  //Serial.printf("get_muso_pitch_for_trigger(%i) returning gate %i\r\n", trigger, gate);
  if (gate!=-1)
    return MUSO_NOTE_MINIMUM + gate;
  return -1;

#else
  return MUSO_NOTE_MINIMUM+trigger;
#endif
}

// functions for sending MIDI out

//static int i = 0;
void fire_trigger(byte t, byte v, bool internal = false) {
  //Serial.printf("firing trigger=%i, v=%i\r\n", t, v);
  // t = trigger number, p = keyboard note
  byte p = MUSO_NOTE_MINIMUM + t;
  byte b = BITBOX_NOTE_MINIMUM + t;
  if (
    p >= MUSO_NOTE_MINIMUM &&
    p < MUSO_NOTE_MAXIMUM
  ) {
    //Serial.printf("   for trigger %i sending muso gate note on pitch %i\r\n", t, p);
    trigger_status[t] = v > 0; // TRIGGER_IS_ON;
    //Serial.println("set trigger_status to ON");
    if (MUSO_GATE_CHANNEL>0 && get_muso_pitch_for_trigger(t)>=0)        {
      //Serial.printf("fire_trigger(%i) sending muso pitch %i\r\n", t, get_muso_pitch_for_trigger(t));
      MIDIOUT.sendNoteOn(get_muso_pitch_for_trigger(t)/*p*/, v, MUSO_GATE_CHANNEL);
    }
    if (MIDI_CHANNEL_BITBOX_OUT>0)  MIDIOUT.sendNoteOn(b, v, MIDI_CHANNEL_BITBOX_OUT);
    //Serial.println("sent both midi notes");
  } else if (
    p >= MUSO_NOTE_MAXIMUM &&
    p < MUSO_NOTE_MAXIMUM + NUM_ENVELOPES
  ) {
    //Serial.printf("   for trigger %i, is an envelope trigger - muso_gate_channel is %i!\r\n", t, MUSO_GATE_CHANNEL);
    if (MUSO_GATE_CHANNEL==DEFAULT_MUSO_GATE_CHANNEL)        update_envelope (p - (MUSO_NOTE_MAXIMUM), v, true);
    if (MIDI_CHANNEL_BITBOX_OUT>0)  MIDIOUT.sendNoteOn(b, v, MIDI_CHANNEL_BITBOX_OUT);  // also send trigger for the envelopes
  } else if (t>=NUM_TRIGGERS + NUM_ENVELOPES && t<NUM_PATTERNS) { //p == MUSO_NOTE_MAXIMUM + NUM_ENVELOPES) {
    //Serial.printf("   for trigger %i got HARMONY instrument %i!\r\n", t, t - (NUM_TRIGGERS + NUM_ENVELOPES));
    //if (autobass_input.is_note_held()) // todo: make this so that can still play bass when no DAW present...
    //harmony.fire_both(); //bass_note_on_and_next();
    harmony.fire_for(t - (NUM_TRIGGERS + NUM_ENVELOPES));
    //else
    //  Serial.println("No note held? is_note_held is false");
  } else {
    Serial.printf("WARNING: fire_trigger not doing anything with trigger %i (pitch %i)\r\n", t, p);
  }
  if (midiecho_enabled)
    if (internal) echo_fire_trigger(p - MUSO_NOTE_MINIMUM, v);
  //Serial.println("finishing fire_trigger");
}

void douse_trigger(byte t, byte v = 0, bool internal = false, bool tied = false) {
  //Serial.printf("dousing trigger=%i\r\n", t);
  byte p = MUSO_NOTE_MINIMUM + t;
  byte b = BITBOX_NOTE_MINIMUM + t;
  if (                                                  // a gate trigger
    p >= MUSO_NOTE_MINIMUM &&
    p < MUSO_NOTE_MAXIMUM
  ) {
    //Serial.printf("   for trigger %i sending muso gate note OFF pitch %i\r\n", t, p);
    trigger_status[t] = TRIGGER_IS_OFF;
    //Serial.println("set trigger_status to OFF");
    if (MUSO_GATE_CHANNEL>0 && get_muso_pitch_for_trigger(t)>=0)        MIDIOUT.sendNoteOff(get_muso_pitch_for_trigger(t)/*p*/, v, MUSO_GATE_CHANNEL);   // hardcoded channel 16 for midimuso
    if (MIDI_CHANNEL_BITBOX_OUT>0)  MIDIOUT.sendNoteOff(b, v, MIDI_CHANNEL_BITBOX_OUT);
    //MIDIOUT.sendNoteOff(b + 12, v, MIDI_CHANNEL_BITBOX_KEYS);
    //Serial.printf("fired a note OFF to bit box: %i\r\n", i);
  } else if (                                           // an envelope trigger
    p >= MUSO_NOTE_MAXIMUM &&
    p < MUSO_NOTE_MAXIMUM + NUM_ENVELOPES
  ) {
    //Serial.printf("   for trigger %i, dousing an envelope trigger!\r\n", p);
    if (MUSO_GATE_CHANNEL==DEFAULT_MUSO_GATE_CHANNEL)        update_envelope (p - (MUSO_NOTE_MAXIMUM), 0, false);
    if (MIDI_CHANNEL_BITBOX_OUT>0)  MIDIOUT.sendNoteOff(b, v, MIDI_CHANNEL_BITBOX_OUT);
  } else if (t>=NUM_TRIGGERS + NUM_ENVELOPES && t<NUM_PATTERNS) { //p == MUSO_NOTE_MAXIMUM + NUM_ENVELOPES) {
    //Serial.printf("   for trigger %i dousing BASS trigger!\r\n", p);
    //harmony.douse_both();
    if (tied) Serial.printf(">>>TIES: douse_trigger so STARTING TIED NOTE ON INSTRUMENT %i!\r\n", t - (NUM_TRIGGERS + NUM_ENVELOPES));
    harmony.douse_for(t - (NUM_TRIGGERS + NUM_ENVELOPES), tied);
  } else {
    //Serial.printf("WARNING: douse_trigger not doing anything with pitch %i\r\n", p);
  }
  if (midiecho_enabled)
    if (internal) echo_douse_trigger(p - MUSO_NOTE_MINIMUM, v);
}

void douse_all_triggers(bool internal = false) {
  for (int i = 0 ; i < NUM_TRIGGERS + NUM_ENVELOPES + NUM_MIDI_OUTS ; i++) {
    douse_trigger(i, 0, internal);
  }
  kill_envelopes(); // force envelopes to shush
}

void midi_send_envelope_level(byte envelope, byte level) {
  //Serial.printf("Envelope[%i] in stage %i: sending lvl %i to midi_cc %i!\r\n", envelope, envelopes[envelope].stage, level, envelopes[envelope].midi_cc);
  //if (envelope == ENV_RIDE_CYMBAL) { // hack to use the pitch bend output as an envelope, since my 'cc 74' output seems to have stopped working - could use this to add an extra envelope or LFO etc
#ifdef MUSO_USE_PITCH_FOR
  if (envelopes[envelope].midi_cc == MUSO_USE_PITCH_FOR) {
    MIDIOUT.sendPitchBend((level << 7) + MIDI_PITCHBEND_MIN, MUSO_CV_CHANNEL);
  } else {
#else
  {
#endif
    MIDIOUT.sendControlChange(envelopes[envelope].midi_cc, level, MUSO_CV_CHANNEL); // send message to midimuso
  }
}

void midi_kill_notes_bitbox() {
  if (MIDI_CHANNEL_BITBOX_OUT>0) {
    MIDIOUT.sendControlChange (MIDI_CC_ALL_NOTES_OFF, 0, MIDI_CHANNEL_BITBOX_OUT);   // 123 = kill all notes
    if (midiecho_enabled)
      MIDIIN.sendControlChange (MIDI_CC_ALL_NOTES_OFF, 0, GM_CHANNEL_DRUMS);     // 123 = kill all notes - for midiecho to host
  }
}
void midi_kill_notes_muso_drums() {
  if (MUSO_GATE_CHANNEL>0) {
    MIDIOUT.sendControlChange (MIDI_CC_ALL_NOTES_OFF, 0, MUSO_GATE_CHANNEL);   // 123 = kill all notes
    if (midiecho_enabled)
      MIDIIN.sendControlChange (MIDI_CC_ALL_NOTES_OFF, 0, GM_CHANNEL_DRUMS);     // 123 = kill all notes - for midiecho to host
  }  
}

void midi_kill_notes() {
  /*if (MUSO_GATE_CHANNEL>0) {
    MIDIOUT.sendControlChange (123, 0, MUSO_GATE_CHANNEL);   // 123 = kill all notes
    if (midiecho_enabled)
      MIDIIN.sendControlChange (123, 0, GM_CHANNEL_DRUMS);     // 123 = kill all notes - for midiecho to host
  }*/
  midi_kill_notes_muso_drums();
  midi_kill_notes_bitbox();
  harmony.kill_notes();
}

void kill_notes() {
  // forget which triggers are active
  Serial.println("!!!! kill_notes called!");
  douse_all_triggers(true);
  //activeNotes = 0;

  midi_kill_notes();
}

static unsigned long last_clock_ticked;
void midi_send_clock(unsigned long received_ticks) {
  //Serial.println("midi_send_clock()");

  if (received_ticks != last_clock_ticked) {
    MIDIOUT.sendClock();
    last_clock_ticked = received_ticks;
  } 

#ifdef CLOCK_TRIGGER_ENABLED
  process_clock_triggers(received_ticks);
#endif

}

#endif

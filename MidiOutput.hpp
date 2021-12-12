#ifndef MIDIOUTPUT_INCLUDED
#define MIDIOUTPUT_INCLUDED

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


#define DEFAULT_MIDI_CHANNEL_BITBOX_OUT 11  // for the mirroring of drums
#define MIDI_CHANNEL_BITBOX_DRUMS_OUT (midi_channel_bitbox_drums_out)
int midi_channel_bitbox_drums_out = DEFAULT_MIDI_CHANNEL_BITBOX_OUT;

//int midi_channel_bitbox_drums_out = DEFAULT_MIDI_CHANNEL_BITBOX_OUT;

#include "MidiSetup.hpp"
#include "Envelopes.h"    // for access to envelope info
#include "Harmony.hpp"    // for access to the harmony channel info
#include "Euclidian.h"    // cos we need to know the the number of patterns (actually we dont but..)
#include "ClockTriggerOutput.hpp"

int pitch_for_trigger_table[NUM_TRIGGERS];
int get_muso_pitch_for_trigger(int trigger) {
  return pitch_for_trigger_table[trigger];
}

// get the output pitch to use for this trigger
// take into account that some outputs may be missing or moved when in different muso modes and swap them appropriately
int get_muso_pitch_for_trigger_actual(int trigger) {
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

  //OUT_printf("get_muso_pitch_for_trigger(%i) returning gate %i\r\n", trigger, gate);
  if (gate!=-1)
    return MUSO_NOTE_MINIMUM + gate;
  return -1;
#elif MUSO_MODE==MUSO_MODE_0B_AND_2A  // multi-board mode -- put kick onto stick to avoid problem with muso board in 0B unexpectedly reacting on channel 1 note on/offs
  int gate = trigger;
  if (trigger==0) gate = 1;   // kick -> stick
  else if (trigger==1) gate = -1; // disable stick

  if (gate!=-1)
    return MUSO_NOTE_MINIMUM + gate;
  return -1;
#else
  return MUSO_NOTE_MINIMUM+trigger;
#endif
}

void initialise_pitch_for_trigger_table () {
  for (int i = 0 ; i < NUM_TRIGGERS ; i++) {
    pitch_for_trigger_table[i] = get_muso_pitch_for_trigger_actual(i);
  }
}

// functions for sending MIDI out

//static int i = 0;
void fire_trigger(byte trigger, byte velocity, bool internal = false) {
  //OUT_printf("firing trigger=%i, v=%i\r\n", t, v);
  // t = trigger number, p = keyboard note
  byte p = MUSO_NOTE_MINIMUM + trigger;
  byte b = BITBOX_NOTE_MINIMUM + trigger;
  if (
    p >= MUSO_NOTE_MINIMUM &&
    p < MUSO_NOTE_MAXIMUM
  ) {
    //OUT_printf("   for trigger %i sending muso gate note on pitch %i\r\n", t, p);
    trigger_status[trigger] = velocity > 0; // TRIGGER_IS_ON;
    //OUT_println("set trigger_status to ON");
    if (MUSO_GATE_CHANNEL>0 && get_muso_pitch_for_trigger(trigger)>=0)        {
      //OUT_printf("fire_trigger(%i) sending muso pitch %i\r\n", t, get_muso_pitch_for_trigger(t));
      MIDIOUT.sendNoteOn(get_muso_pitch_for_trigger(trigger)/*p*/, velocity, MUSO_GATE_CHANNEL);
    }
    if (MIDI_CHANNEL_BITBOX_DRUMS_OUT>0)  {
      OUT_printf("fire_trigger(%i) sending bitbox drum pitch %i on channel %i\r\n", b, MIDI_CHANNEL_BITBOX_DRUMS_OUT);
      MIDIOUT.sendNoteOn(b, velocity, MIDI_CHANNEL_BITBOX_DRUMS_OUT);
    }
    update_envelopes_for_trigger(trigger, velocity, true);
    //OUT_println("sent both midi notes");
  } else if (
    p >= MUSO_NOTE_MAXIMUM &&
    p < MUSO_NOTE_MAXIMUM + NUM_ENVELOPES
  ) {
    //OUT_printf("   for trigger %i, is an envelope trigger - muso_gate_channel is %i!\r\n", t, MUSO_GATE_CHANNEL);
    if (MUSO_GATE_CHANNEL==DEFAULT_MUSO_GATE_CHANNEL)        update_envelope (p - (MUSO_NOTE_MAXIMUM), velocity, true);
    if (MIDI_CHANNEL_BITBOX_DRUMS_OUT>0) MIDIOUT.sendNoteOn(b, velocity, MIDI_CHANNEL_BITBOX_DRUMS_OUT);  // also send trigger for the envelopes
  } else if (trigger>=NUM_TRIGGERS + NUM_ENVELOPES && trigger<NUM_PATTERNS) { //p == MUSO_NOTE_MAXIMUM + NUM_ENVELOPES) {
    //OUT_printf("   for trigger %i got HARMONY instrument %i!\r\n", t, t - (NUM_TRIGGERS + NUM_ENVELOPES));
    //if (autobass_input.is_note_held()) // todo: make this so that can still play bass when no DAW present...
    //harmony.fire_both(); //bass_note_on_and_next();
    harmony.fire_for(trigger - (NUM_TRIGGERS + NUM_ENVELOPES));
    update_envelopes_for_trigger(trigger, velocity, true);
    //else
    //  OUT_println("No note held? is_note_held is false");
  } else {
    OUT_printf("WARNING: fire_trigger not doing anything with trigger %i (pitch %i)\r\n", trigger, p);
  }
  if (midiecho_enabled)
    if (internal) echo_fire_trigger(p - MUSO_NOTE_MINIMUM, velocity);
  //OUT_println("finishing fire_trigger");
}

void douse_trigger(byte trigger, byte velocity = 0, bool internal = false, bool tied = false) {
  //OUT_printf("dousing trigger=%i\r\n", t);
  byte p = MUSO_NOTE_MINIMUM + trigger;
  byte b = BITBOX_NOTE_MINIMUM + trigger;
  if (                                                  // a gate trigger
    p >= MUSO_NOTE_MINIMUM &&
    p < MUSO_NOTE_MAXIMUM
  ) {
    // a drum trigger
    //OUT_printf("   for trigger %i sending muso gate note OFF pitch %i\r\n", t, p);
    trigger_status[trigger] = TRIGGER_IS_OFF;
    //OUT_println("set trigger_status to OFF");
    if (MUSO_GATE_CHANNEL>0 && get_muso_pitch_for_trigger(trigger)>=0)        
      MIDIOUT.sendNoteOff(
        get_muso_pitch_for_trigger(trigger)/*p*/, 
        velocity, MUSO_GATE_CHANNEL
      );   // hardcoded channel 16 for midimuso

    if (midi_channel_bitbox_drums_out>0) {
      OUT_printf("!!!! douse_trigger(%i) sending velocity %i on pitch %i to channel %i\r\n", trigger, velocity, b, midi_channel_bitbox_drums_out);
      //delay(10);
      //MIDIOUT.sendNoteOff(b, v, 0);
      MIDIOUT.sendNoteOff(b, velocity, midi_channel_bitbox_drums_out);
    }
    update_envelopes_for_trigger(trigger, velocity, false);
    //MIDIOUT.sendNoteOff(b + 12, v, MIDI_CHANNEL_BITBOX_KEYS);
    //OUT_printf("fired a note OFF to bit box: %i\r\n", i);
  } else if (
    // an envelope trigger
    p >= MUSO_NOTE_MAXIMUM &&
    p < MUSO_NOTE_MAXIMUM + NUM_ENVELOPES
  ) {
    //OUT_printf("   for trigger %i, dousing an envelope trigger!\r\n", p);
    if (MUSO_GATE_CHANNEL==DEFAULT_MUSO_GATE_CHANNEL)        update_envelope (p - (MUSO_NOTE_MAXIMUM), 0, false);
    if (MIDI_CHANNEL_BITBOX_DRUMS_OUT>0) MIDIOUT.sendNoteOff(b, velocity, MIDI_CHANNEL_BITBOX_DRUMS_OUT);
  } else if (trigger>=NUM_TRIGGERS + NUM_ENVELOPES && trigger<NUM_PATTERNS) { //p == MUSO_NOTE_MAXIMUM + NUM_ENVELOPES) {
    // a harmony trigger
    OUT_printf("   for trigger %i dousing instrument trigger %i!\r\n", trigger, trigger - (NUM_TRIGGERS + NUM_ENVELOPES));
    //harmony.douse_both();
    if (tied) OUT_printf(">>>TIES: douse_trigger so STARTING TIED NOTE ON INSTRUMENT %i!\r\n", trigger - (NUM_TRIGGERS + NUM_ENVELOPES));
    harmony.douse_for(trigger - (NUM_TRIGGERS + NUM_ENVELOPES), tied);
    update_envelopes_for_trigger(trigger, velocity, false);
  } else {
    //OUT_printf("WARNING: douse_trigger not doing anything with pitch %i\r\n", p);
  }
  if (midiecho_enabled)
    if (internal) echo_douse_trigger(p - MUSO_NOTE_MINIMUM, velocity);
}

void douse_all_triggers(bool internal = false) {
  for (int i = 0 ; i < NUM_TRIGGERS + NUM_ENVELOPES + NUM_MIDI_OUTS ; i++) {
#ifdef WORKAROUND_MISSED_NOTEOFFS
    if (i > NUM_TRIGGERS+NUM_ENVELOPES) {
      OUT_printf("WORKAROUND_MISSED_NOTEOFFS_DELAY: douse_all_triggers delaying trigger %i for %i ms", i, WORKAROUND_MISSED_NOTEOFFS_DELAY);
      delay(WORKAROUND_MISSED_NOTEOFFS_DELAY);
    }
#endif
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

void midi_kill_notes_bitbox_drums() {
  if (MIDI_CHANNEL_BITBOX_DRUMS_OUT>0) {
    MIDIOUT.sendControlChange (MIDI_CC_ALL_NOTES_OFF, 0, MIDI_CHANNEL_BITBOX_DRUMS_OUT);   // 123 = kill all notes
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
  midi_kill_notes_bitbox_drums();
  harmony.kill_notes();
}

void kill_notes() {
  // forget which triggers are active
  OUT_println("!!!! kill_notes called!");
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

#ifdef ENABLE_CLOCK_TRIGGER
  process_clock_triggers(received_ticks);
#endif

}

#endif

#ifndef MIDIOUTPUT_INCLUDED
#define MIDIOUTPUT_INCLUDED

#define BITBOX_NOTE_MINIMUM     36  // https://1010music.com/wp-content/uploads/2020/08/bitbox-mk2-1.0.8-user-manual.pdf "MIDI inputs for notes 36 to 51 map to the pads", "EXT1 through EXT4 are assigned notes 55 to 52 for use as Recording triggers"
#define BITBOX_KEYS_OCTAVE_OFFSET 2
#define MIDI_CHANNEL_BITBOX_OUT 11
#define MIDI_CHANNEL_BITBOX_KEYS 3 // bass output, but shifted an octave

#include "MidiSetup.hpp"
#include "Envelopes.h"    // for access to envelope info
#include "Harmony.hpp"       // for access to the harmony channel info

// functions for sending MIDI out

static int i = 0;
void fire_trigger(byte t, byte v, bool internal = false) {
  //Serial.printf("firing trigger pitch=%i, v=%i\r\n", p, v);
  // t = trigger number, p = keyboard note
  byte p = MUSO_NOTE_MINIMUM + t;
  byte b = BITBOX_NOTE_MINIMUM + t;
  if (
    p >= MUSO_NOTE_MINIMUM &&
    p < MUSO_NOTE_MAXIMUM) {
    trigger_status[p - MUSO_NOTE_MINIMUM] = v > 0; // TRIGGER_IS_ON;
    MIDIOUT.sendNoteOn(p, v, MUSO_GATE_CHANNEL);
    MIDIOUT.sendNoteOn(b, v, MIDI_CHANNEL_BITBOX_OUT);
    //MIDIOUT.sendNoteOn(b + 12, v, MIDI_CHANNEL_BITBOX_KEYS);
  } else if (
    //Serial.printf("is an envelope trigger!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    p >= MUSO_NOTE_MAXIMUM &&
    p < MUSO_NOTE_MAXIMUM + NUM_ENVELOPES) {
    update_envelope (p - (MUSO_NOTE_MAXIMUM), v, true);
    MIDIOUT.sendNoteOn(b, v, MIDI_CHANNEL_BITBOX_OUT);  // also send trigger for the envelopes
    //MIDIOUT.sendNoteOn(b + 12, v, MIDI_CHANNEL_BITBOX_KEYS);  // also send trigger for the envelopes
  } else if (p == MUSO_NOTE_MAXIMUM + NUM_ENVELOPES) {
    //Serial.printf(">> got BASS trigger!\r\n");
    //if (autobass_input.is_note_held()) // todo: make this so that can still play bass when no DAW present...
    harmony.fire_both(); //bass_note_on_and_next();
    //else
    //  Serial.println("No note held? is_note_held is false");
  } else {
    Serial.printf("WARNING: fire_trigger not doing anything with pitch %i\r\n", p);
  }
  if (midiecho_enabled)
    if (internal) echo_fire_trigger(p - MUSO_NOTE_MINIMUM, v);
}

void douse_trigger(byte t, byte v = 0, bool internal = false) {
  byte p = MUSO_NOTE_MINIMUM + t;
  byte b = BITBOX_NOTE_MINIMUM + t;
  if (
    p >= MUSO_NOTE_MINIMUM &&
    p < MUSO_NOTE_MAXIMUM) {
    trigger_status[p - MUSO_NOTE_MINIMUM] = TRIGGER_IS_OFF;
    MIDIOUT.sendNoteOff(p, v, MUSO_GATE_CHANNEL);   // hardcoded channel 16 for midimuso
    MIDIOUT.sendNoteOff(b, v, MIDI_CHANNEL_BITBOX_OUT);
    //MIDIOUT.sendNoteOff(b + 12, v, MIDI_CHANNEL_BITBOX_KEYS);
    //Serial.printf("fired a note OFF to bit box: %i\r\n", i);
  } else if (
    p >= MUSO_NOTE_MAXIMUM &&
    p < MUSO_NOTE_MAXIMUM + NUM_ENVELOPES) {
    update_envelope (p - (MUSO_NOTE_MAXIMUM), 0, false);
    MIDIOUT.sendNoteOff(b, v, MIDI_CHANNEL_BITBOX_OUT);
    //MIDIOUT.sendNoteOff(b, v, MIDI_CHANNEL_BITBOX_KEYS);
  } else if (p == MUSO_NOTE_MAXIMUM + NUM_ENVELOPES) {
    harmony.douse_both(); //bass_note_off();
  } else {
    Serial.printf("WARNING: douse_trigger not doing anything with pitch %i\r\n", p);
  }
  if (midiecho_enabled)
    if (internal) echo_douse_trigger(p - MUSO_NOTE_MINIMUM, v);
}

void douse_all_triggers(bool internal = false) {
  for (int i = 0 ; i < NUM_TRIGGERS + NUM_ENVELOPES + 1 ; i++) {
    douse_trigger(i, 0, internal);
  }
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

/*
void midi_bass_send_note_on(int pitch, int velocity, int channel) {
  MIDIOUT.sendNoteOn(pitch, velocity, channel);
  MIDIOUT.sendNoteOn(pitch + (12*BITBOX_KEYS_OCTAVE_OFFSET), velocity, MIDI_CHANNEL_BITBOX_KEYS); // output to separate channel an octave up
  if (midiecho_enabled)
    MIDIIN.sendNoteOn(pitch, velocity, MIDI_CHANNEL_BASS_OUT);  // echo back to host
  //Serial.printf("midi_bass_send_note_on(%i, %i, %i)\n", pitch, velocity, MIDI_CHANNEL_BASS_OUT);
  // todo: move echo back to host stuff into MidiEcho
}

void midi_bass_send_note_off(int pitch, int velocity, int channel) {
  MIDIOUT.sendNoteOff(pitch, velocity, channel);
  MIDIOUT.sendNoteOn(pitch + (12*BITBOX_KEYS_OCTAVE_OFFSET), velocity, MIDI_CHANNEL_BITBOX_KEYS); // output an octave up for bitbox keys
  if (midiecho_enabled)
    MIDIIN.sendNoteOff(pitch, velocity, MIDI_CHANNEL_BASS_OUT);  // echo back to host
  // todo: move echo back to host stuff into MidiEcho
}*/

void midi_kill_notes() {
  MIDIOUT.sendControlChange (123, 0, MUSO_GATE_CHANNEL);   // 123 = kill all notes
  if (midiecho_enabled)
    MIDIIN.sendControlChange (123, 0, GM_CHANNEL_DRUMS);     // 123 = kill all notes - for midiecho to host
  harmony.kill_notes();
}

void kill_notes() {
  // forget which triggers are active
  douse_all_triggers(true);
  //activeNotes = 0;

  midi_kill_notes();
}

/// clock stuff (hacky via CD74HC4067 multiplexor output module https://www.amazon.co.uk/dp/B07VF14YNG/ref=pe_3187911_185740111_TE_item)

#define CLOCK_OUT_START_PIN 2
#define CLOCK_BEAT      0
#define CLOCK_BEAT_ALT  1
#define CLOCK_BAR       2
#define CLOCK_PHRASE    3
#define CLOCK_COUNT     4
//const static byte clock_map[] = { 1, 2, 6, 10 };
//const static byte clock_map[] = { 2, 6, 4, 10 };
const static byte clock_map[] = { 4, 2, 3, 10 };  // this is the one, for some reason?  have i got my pins swapped somewhere?
//const static byte clock_map[] = { 1, 2, 6, 11 };

#define CLOCK_LOOP_COUNT  5  // X ms before moving onto next clock / stopping clock

static int should_send_clock[4] = { 0, 0, 0, 0 };

bool clock_on = false;
static unsigned long last_beat_clock_ticked;
static unsigned long last_beat_clock_millis;

void initialise_clock_outputs() {
  for (int i = 0 ; i < 4 ; i++) {
    pinMode(CLOCK_OUT_START_PIN + i, OUTPUT);
    digitalWrite(CLOCK_OUT_START_PIN + i, LOW);
  }
}

void clock_output (byte clock_number, bool on) {
  if (!on) {
    for (int i = 0 ; i < 4 ; i++) {
      digitalWrite( CLOCK_OUT_START_PIN + i, LOW );
    }
    clock_on = false;
  } else {
    Serial.printf("clock number %i on to mapped %i, beat number %i, count %i\r\n", clock_number, clock_map[clock_number], current_beat, should_send_clock[clock_number]);
    for (int i = 0 ; i < 4 ; i++) {
      digitalWrite( CLOCK_OUT_START_PIN + i, bitRead(clock_map[clock_number], i) );
    }
    clock_on = true;
  }
}


static unsigned long last_clock_ticked;
void midi_send_clock(unsigned long received_ticks) {
  //Serial.println("midi_send_clock()");


  //last_clock_ticked = received_ticks;
  if (last_beat_clock_ticked!=received_ticks) {
    if (!should_send_clock[CLOCK_BEAT_ALT] && is_bpm_on_beat && (current_beat==1 || current_beat==3)) {
      last_beat_clock_ticked = received_ticks;
      Serial.printf("setting CLOCK_BEAT_ALT to %i, was already %i\r\n", should_send_clock[CLOCK_BEAT_ALT], CLOCK_LOOP_COUNT);
      should_send_clock[CLOCK_BEAT_ALT] = CLOCK_LOOP_COUNT;
    }
    if (!should_send_clock[CLOCK_BEAT] && is_bpm_on_beat) {
      last_beat_clock_ticked = received_ticks;
      should_send_clock[CLOCK_BEAT] = CLOCK_LOOP_COUNT;
    }
    if (!should_send_clock[CLOCK_BAR] && is_bpm_on_beat && is_bpm_on_bar) {
      last_beat_clock_ticked = received_ticks;
      should_send_clock[CLOCK_BAR] = CLOCK_LOOP_COUNT;
    }
    if (!should_send_clock[CLOCK_PHRASE] && is_bpm_on_beat && is_bpm_on_phrase) {
      last_beat_clock_ticked = received_ticks;
      should_send_clock[CLOCK_PHRASE] = CLOCK_LOOP_COUNT;
    }
    /*  // for testing - trigger-per-beat-number
    if (is_bpm_on_beat && should_send_clock[current_beat]==0) {
      should_send_clock[current_beat] = CLOCK_LOOP_COUNT;
      last_beat_clock_ticked = received_ticks;
    }*/
  }

  if (received_ticks != last_clock_ticked) {
    MIDIOUT.sendClock();
    last_clock_ticked = received_ticks;
  } 
  
  for (int i = 0 ; i < CLOCK_COUNT ; i++) {
    if (should_send_clock[i]>0) {
      /*if (!clock_on)*/ 
      if (should_send_clock[i]==CLOCK_LOOP_COUNT) clock_output (i, true);
      should_send_clock[i] -= millis() - last_beat_clock_millis;
      if (should_send_clock[i]<0) should_send_clock[i] = 0;
      break;
    }
  }

  last_beat_clock_millis = millis();
  //if (received_ticks > last_beat_clock_ticked + 15) {
    if (clock_on && should_send_clock[0]==0 && should_send_clock[1]==0 && should_send_clock[2]==0 && should_send_clock[3]==0) {
      Serial.println("=== all clocks off");
      clock_output (0, false); 
    }
  //}  
}

#endif

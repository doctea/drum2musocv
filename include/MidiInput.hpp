#ifndef MIDIINPUT_INCLUDED
#define MIDIINPUT_INCLUDED

#define RELAY_PROGRAM_CHANGE false   // only enable passing through program changes when we know its safe ie when only one midimuso is connected

#include "MidiSetup.hpp"
#include "Drums.h"
#include "MidiOutput.hpp"


// GLOBALS

byte activeNotes = 0;             // tracking how many keys are held down
unsigned long last_input_at = 0;  // timestamp we last received midi from host

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

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  if (channel==GM_CHANNEL_DRUMS) {
    byte p = pitch;
    byte v = velocity;
  
    activeNotes--;
    /*if (!process_triggers_for_pitch(p, v, false)) {
      p = convert_drum_pitch(p);
      douse_trigger(p, 0);
    }*/
    douse_trigger(get_trigger_for_pitch(p), 127);
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
  if (channel==MIDI_CHANNEL_EXTENDED_ENVELOPES) {
    handle_envelope_ccs(channel, number, value);
  } else if (channel==GM_CHANNEL_DRUMS) {
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

    /*} else if (handle_bass_ccs(channel, number, value)) {
      Serial.printf("Handled bass cc channel %i number %i value %i\r\n", channel, number, value);      */
    } else if (handle_ui_ccs(channel, number, value)) {

    } else if (handle_midiecho_ccs(channel, number, value)) {

    } else if (harmony.handle_ccs(channel, number, value)) {

    } else if (number==MIDI_CC_ALL_NOTES_OFF) {
      midi_kill_notes();      
    } else {
      //MIDI.sendControlChange(number, value, 1); // pass thru unhandled CV
      if (number==0x7B) {// || // intercept 'all notes off', 
            // TODO: have i commented out the wrong lines here? ^^^
            kill_notes(); // added
            kill_envelopes();
      } else if (number==0x07) {
          //number==0x65 || // RPN MSB
          //number==0x07*/) { // intercept 'volume' messages ..  this is the fucker interfering -- used for overall volume control, so DAW sends this, interferring with our control of the CC!
          //TODO: do i need to also ignore the others (1,7,11,71,74)?
          //TODO: or... use them as offsets so can modulate...?
      }
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
  bpm_update_status(received_ticks);

  //Serial.printf("Received song position of %i received_ticks which is %i steps ", received_ticks, steps);
  //Serial.printf("(current_step=%i) (and so should be song phrase %i?)\r\n",  current_step, current_phrase); //(received_ticks/(TICKS_PER_STEP*STEPS_PER_BEAT*BEATS_PER_BAR)) / BARS_PER_PHRASE;);
  
  last_input_at = millis();  
}

void handleClock() {
  //MIDIOUT.sendClock();  // do this in clock_tick now so we can control subdivisions
  
  bpm_receive_clock_tick();
  last_input_at = millis();
}

void handleStart() {
  // TODO: start LFOs?
  MIDIOUT.sendStart();
  //playing = true;
  //Serial.println("Resetting clock - received handleStart!");
  bpm_reset_clock(0);  // -1 so next tick will be start
}
void handleContinue() {
  // TODO: continue LFOs
  MIDIOUT.sendContinue();
  kill_envelopes();
  //playing = true;
}
void handleStop() {
  MIDIOUT.sendStop();
  // TODO: stop+reset LFOs
  Serial.println("Received STOP -- killing envelopes / notes / resetting clock !");
  kill_notes();
  kill_envelopes();
  //douse_all_triggers(true); // is done in kill_notes
  harmony.kill_notes();

  Serial.println("Resetting clock - received handleStop!");
  bpm_reset_clock(0);  // -1 to make sure next tick is treated as first step of beat
  last_input_at = millis();
  //playing = false;
  
//#ifdef ENABLE_PIXELS
//  kill_notes();
//#endif

}

void handleSystemExclusive(byte* array, unsigned size) {
  // pass sysex messages through to the midimuso
  // TODO: configuration of drum2musocv via sysex?
  MIDIOUT.sendSysEx(size, array, false); // true/false means "array contains start/stop padding" -- think what we receive here is without padding..?
}

void handleProgramChange(byte channel, byte pcm) {
  if (RELAY_PROGRAM_CHANGE) {
    Serial.printf("Sending program change %i on channel %i\r\n", pcm, channel);
    MIDIOUT.sendProgramChange(pcm, channel);
  } else {
    Serial.printf("Ignoring program change %i on channel %i\r\n", pcm, channel);
  }
}

// called every loop(), to read incoming midi and route if appropriate
void process_midi() {
  if (MIDIIN.read()) {
    //Serial.printf("received message from MIDIIN, channel is %i: type is %i, ", MIDIIN.getChannel(), MIDIIN.getType()  );
    //Serial.printf("data1 is %i, data2 is %i\r\n", MIDIIN.getData1(), MIDIIN.getData2() );
    if (/*!MIDIIN.isChannelMessage() ||*/ 
      MIDIIN.getType()!=midi::MidiType::NoteOn && 
      MIDIIN.getType()!=midi::MidiType::NoteOff && 
      MIDIIN.getType()!=midi::MidiType::ControlChange)
        return; // && !MIDIIN.getType()!=midi::MidiType::PitchBend
    if (MIDIIN.getChannel()==GM_CHANNEL_DRUMS) {
      // ignore so that the handle* functions will deal with this instead
    } else if (MIDIIN.getChannel()==MIDI_CHANNEL_BASS_AUTO_IN) {
      // tell the harmony/autobass what notes we wanna play
      if (MIDIIN.getType()==midi::MidiType::NoteOn) {
        autobass_input.handle_note_on(MIDIIN.getData1(), MIDIIN.getData2());
      } else if (MIDIIN.getType()==midi::MidiType::NoteOff) {
        autobass_input.handle_note_off(MIDIIN.getData1());
        if (!autobass_input.is_note_held()) { // if this has meant all nodes have turned off, kill the existing note
          harmony.douse_all(); //bass_note_off();
        }
      }
    } else if (MIDIIN.getChannel()==MIDI_CHANNEL_BASS_IN) {            // forward bass input unchanged
      // relay all incoming messages for the Neutron/bass
      //Serial.println("Received a message targeted at the neutron directly: %0x, %0x, %0x\n", MIDIIN.getType(), MIDIIN.getData1(), MIDIIN.getData2());
      if (MIDIIN.getType()==midi::MidiType::NoteOn) {
        //autobass_input.handle_note_on(MIDIIN.getData1(), MIDIIN.getData2());
        harmony.send_note_on_for_channel(MIDI_CHANNEL_BASS_OUT, MIDIIN.getData1(), MIDIIN.getData2());
        update_envelopes_for_trigger(TRIGGER_HARMONY_BASS, MIDIIN.getData2(), true);
      } else if (MIDIIN.getType()==midi::MidiType::NoteOff) {
        harmony.send_note_off_for_channel(MIDI_CHANNEL_BASS_OUT, MIDIIN.getData1(), MIDIIN.getData2());
        if(!harmony.is_note_held(MIDI_CHANNEL_BASS_OUT))
          update_envelopes_for_trigger(TRIGGER_HARMONY_BASS, MIDIIN.getData2(), false);
      } else {
        MIDIOUT.send(MIDIIN.getType(),
                    MIDIIN.getData1(),
                    MIDIIN.getData2(),
                    MIDI_CHANNEL_BASS_OUT
        );
      }
    } else if (MIDIIN.getChannel()==MIDI_CHANNEL_MELODY_IN) {       // forward channel 3 unchanged
      // direct melody playing - echo all messages straight through to eg bitbox
      if (MIDIIN.getType()==midi::MidiType::NoteOn) {
        //autobass_input.handle_note_on(MIDIIN.getData1(), MIDIIN.getData2());
        harmony.send_note_on_for_channel(MIDI_CHANNEL_BITBOX_KEYS, MIDIIN.getData1(), MIDIIN.getData2());
        update_envelopes_for_trigger(TRIGGER_HARMONY_MELODY, MIDIIN.getData2(), true);
      } else if (MIDIIN.getType()==midi::MidiType::NoteOff) {
        harmony.send_note_off_for_channel(MIDI_CHANNEL_BITBOX_KEYS, MIDIIN.getData1(), MIDIIN.getData2());
        if(!harmony.is_note_held(MIDI_CHANNEL_BITBOX_KEYS))
          update_envelopes_for_trigger(TRIGGER_HARMONY_MELODY, MIDIIN.getData2(), false);
      } else {
        MIDIOUT.send(MIDIIN.getType(),
                   MIDIIN.getData1(),
                   MIDIIN.getData2(),
                   MIDI_CHANNEL_BITBOX_KEYS
        );
      }
    } else if (MIDIIN.getChannel()==MIDI_CHANNEL_PAD_ROOT_IN) {     // forward channel 1 unchanged
      // for ensemble - send notes to muso on channel 1
      if (MIDIIN.getType()==midi::MidiType::NoteOn) {
        //autobass_input.handle_note_on(MIDIIN.getData1(), MIDIIN.getData2());
        //Serial.println("MIDI_CHANNEL_PAD_ROOT_IN note ON");
        harmony.send_note_on_for_channel(MIDI_CHANNEL_PAD_ROOT_OUT, MIDIIN.getData1(), MIDIIN.getData2());
        update_envelopes_for_trigger(TRIGGER_HARMONY_PAD_ROOT, MIDIIN.getData2(), true);
      } else if (MIDIIN.getType()==midi::MidiType::NoteOff) {
        harmony.send_note_off_for_channel(MIDI_CHANNEL_PAD_ROOT_OUT, MIDIIN.getData1(), MIDIIN.getData2());
        if(!harmony.is_note_held(MIDI_CHANNEL_PAD_ROOT_OUT))
          update_envelopes_for_trigger(TRIGGER_HARMONY_PAD_ROOT, MIDIIN.getData2(), false);
      } else {
        MIDIOUT.send(MIDIIN.getType(),
                    MIDIIN.getData1(),
                    MIDIIN.getData2(),
                    MIDI_CHANNEL_PAD_ROOT_OUT
        );
      }
    } else if (MIDIIN.getChannel()==MIDI_CHANNEL_PAD_PITCH_IN) {    // forward channel 2 unchanged
      // for ensemble - send notes to muso on channel 1
      if (MIDIIN.getType()==midi::MidiType::NoteOn) {
        //autobass_input.handle_note_on(MIDIIN.getData1(), MIDIIN.getData2());
        harmony.send_note_on_for_channel(MIDI_CHANNEL_PAD_PITCH_OUT, MIDIIN.getData1(), MIDIIN.getData2());
        update_envelopes_for_trigger(TRIGGER_HARMONY_PAD_PITCH, MIDIIN.getData2(), true);
      } else if (MIDIIN.getType()==midi::MidiType::NoteOff) {
        harmony.send_note_off_for_channel(MIDI_CHANNEL_PAD_PITCH_OUT, MIDIIN.getData1(), MIDIIN.getData2());
        if(!harmony.is_note_held(MIDI_CHANNEL_PAD_PITCH_OUT))
          update_envelopes_for_trigger(TRIGGER_HARMONY_PAD_PITCH, MIDIIN.getData2(), false);
      } else {
        MIDIOUT.send(MIDIIN.getType(),
                    MIDIIN.getData1(),
                    MIDIIN.getData2(),
                    MIDI_CHANNEL_PAD_PITCH_OUT
        );      
      }
    } else {
      // catch all other channels
      //Serial.printf("received message from MIDIIN, channel is %i: type is %i, ", MIDIIN.getChannel(), MIDIIN.getType()  );
      //Serial.printf("data1 is %i, data2 is %i\r\n", MIDIIN.getData1(), MIDIIN.getData2() );
      /*if (MIDIIN.getChannel()==1) { // || MIDIIN.getChannel()==2 || MIDIIN.getChannel()==16) {
        MIDIOUT.send(MIDIIN.getType(),
                   MIDIIN.getData1(),
                   MIDIIN.getData2(),
                   MIDIIN.getChannel());
      }*/
    }
    //Serial.printf("received message from MIDIIN, channel is %i: type is %i, ", MIDIIN.getChannel(), MIDIIN.getType()  );
    //Serial.printf("data1 is %i, data2 is %i\r\n", MIDIIN.getData1(), MIDIIN.getData2() );
  }
}

void setup_midi() {
  
#ifdef USB_NATIVE 
  MIDIOUT.begin(MIDI_CHANNEL_OMNI); //GM_CHANNEL_DRUMS);
#endif
  MIDIIN.begin(MIDI_CHANNEL_OMNI); //GM_CHANNEL_DRUMS);

  MIDIIN.turnThruOff();
  //MIDIIN.setThruFilterMode(midi::Thru::DifferentChannel);

  MIDIIN.setHandleNoteOn(handleNoteOn);
  MIDIIN.setHandleNoteOff(handleNoteOff);

  MIDIIN.setHandleControlChange(handleControlChange);

  MIDIIN.setHandleSystemExclusive(handleSystemExclusive);

  MIDIIN.setHandleProgramChange(handleProgramChange);

  MIDIIN.setHandleStop(handleStop);
  MIDIIN.setHandleStart(handleStart);
  MIDIIN.setHandleContinue(handleContinue);

  MIDIIN.setHandleClock(handleClock);

  MIDIIN.setHandleSongPosition(handleSongPosition);

}

#endif

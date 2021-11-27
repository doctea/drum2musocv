#ifndef MIDIKEYS_INCLUDED
#define MIDIKEYS_INCLUDED

#define DEBUG_HARMONY false

#define TIE_PORTA_TICK_LENGTH   TICKS_PER_STEP

// the range of the midimuso pitch, apparently
#define MINIMUM_PITCH 21    
#define MAXIMUM_PITCH 101   

#include <USB-MIDI.h> 
#include "Harmony.hpp"
#include "ChannelState.hpp"
#include "MidiSetup.hpp"
#include "Envelopes.h"

// for providing multiple MIDI key outputs ie bass and harmony
//  todo: properly handle multiple notes being on at the same time for tracking chords
class MidiKeysOutput : public ChannelState {
  private:
    //int currently_playing = -1;
    //int currently_playing[10] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
    int octave_offset = 0;
    int melody_mode = HARMONY::MELODY_MODE::SINGLE;
    int arp_counter = 0;

  public:
    byte channel = 0;
    
    MidiKeysOutput(int chan, int octave_off = 0) {
      channel = chan;
      octave_offset = octave_off;
    }

    MidiKeysOutput set_melody_mode(int mm) {
      melody_mode = mm % HARMONY::MELODY_MODE::MELODY_MODE_MAX;
      return *this;
    }

    bool tied_status = false;
    bool tied_started = false;
    int stored_tied_notes[10];
    int tied_ttl = 0;
    void process_tick_ties() {
      //if (tied_status) Serial.printf("process_tick_ties on a tied note for channel %i, ttl is %i\r\n", channel, tied_ttl);
      if (tied_status && tied_started && --tied_ttl<=0) {
        //Serial.printf("<<<<TIES: Processing a finished tie, latched note to kill==%i!\r\n", stored_tied_notes[0]);
        send_note_off(stored_tied_notes);
        tied_status = false;
        tied_started = false;
        for (int i = 0 ; i < 10 ; i++) {
          stored_tied_notes[i] = -1;
        }
        tied_status = false;
      }
    }

    void fire_notes(int pitch, int *pitches, int velocity = 127) {
      //Serial.printf("fire_notes for channel %i with melody_mode %i: pitch %i\r\n", channel, melody_mode, pitch);

      if (is_note_held() && !tied_status) {
        douse_notes();
      }
      if (tied_status) {
        //Serial.printf("TIES: fire_notes with tied_status - latched note is %i - STARTING countdown\r\n", stored_tied_notes[0]);
        // there is a tye pending, so set ttl
        if (stored_tied_notes[0]==pitch) { // same note, so kill old one first
          //Serial.printf("TIES: fire_notes told to tie same note as already tied, so sending note off for %i first\r\n", stored_tied_notes[0]);
          send_note_off(stored_tied_notes);
          tied_started = tied_status = false;
          tied_ttl = 0;
        } else {
          tied_ttl = TIE_PORTA_TICK_LENGTH; //TICKS_PER_STEP/2; //PPQN/2;
          tied_started = true;
        }
      }

      if (melody_mode==HARMONY::MELODY_MODE::MELODY_MODE_NONE) {
        // do nothing
      } else if (melody_mode==HARMONY::MELODY_MODE::SINGLE) {
        arp_counter = 0;
        send_note_on(pitch, velocity);
        //if (DEBUG_HARMONY) 
        if (DEBUG_HARMONY) Serial.printf("    using single mode - sending pitch %s [%i]\r\n", get_note_name(pitch).c_str(), pitch);
      } else if (melody_mode==HARMONY::MELODY_MODE::CHORD || HARMONY::MELODY_MODE::ARPEGGIATE) {
        //if (DEBUG_HARMONY) 
        
        //memcpy(&last_melody_pitches, get_chord_for_pitch(pitch), 10 * sizeof(int));
        //Serial.printf("about to play chord is %i\r\n", );

          //memcpy(&last_melody_pitches, pitches, 10 * sizeof(int) )

        if (melody_mode==HARMONY::MELODY_MODE::ARPEGGIATE) {
          if (pitches[arp_counter]==-1 || arp_counter>=10) {
            arp_counter = 0;
          }
          /*pitches[0] = pitches[arp_counter];
          for (int i = 1 ; i < 10 ; i++) {
            pitches[i] = -1;
          }*/
          if (DEBUG_HARMONY) Serial.printf("    using arp mode for melody - arp_counter %i so pitch %s!\r\n", arp_counter, get_note_name(pitches[arp_counter]).c_str());
          send_note_on(pitches[arp_counter], velocity);
          arp_counter++;    // TODO: test arp mode as appropriate
        } else {
          if (DEBUG_HARMONY) Serial.println("    using chord mode for melody!");

          send_note_on(pitches, velocity);  // send all notes of generated chord_type
        }
      }
    }

    void douse_notes(bool tied = false) {
      //Serial.printf("douse_notes for channel %i with melody_mode %i: pitch %i\r\n", channel, melody_mode, get_held_notes()[0]);
      if (!tied && is_note_held()) {
        HARM_printf("\tNot tied, dousing lowest %i [%s]\r\n", get_held_notes()[0], get_note_name(get_held_notes()[0]).c_str());
        send_note_off(get_held_notes());
        //send_all_notes_off();
      } else if (tied && is_note_held()) {
        //Serial.printf("TIED: not dousing");
        //Serial.printf("TIES: latching tie -- tying current held note %i\r\n", get_held_notes()[0]);
        //Serial.printf(" - storing note %i\r\n", get_held_notes()[0]);

        // note should be tied with previous, so set a flag and wait for next trigger before killing previous notes
        tied_status = true;
        //tied_ttl = 6;
        tied_started = false;
        memcpy(&stored_tied_notes, get_held_notes(), 10*sizeof(int)); //get_held_notes()[0]));
        HARM_printf("TIES: latching tie -- stored held note %i\r\n", stored_tied_notes[0]);
      }
    }

    // send_note_on/off multiples, expects array of size 10
    // todo: make this handle velocity per note
    void send_note_on(int *pitches, int velocity = 127) { //velocity[10] = { 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 }) {
      //Serial.printf("send_note_on!\r\n");

      HARM_printf(">>> Start ON for multiples on channel %i, held: %s\r\n", channel, get_debug_notes_held());
      if (DEBUG_HARMONY) {
        Serial.printf("   channel %i HarmonyOutput  playing chord: [ ", channel);
        for (int i = 0 ; i < 10 ; i++) {
          if (pitches[i]>-1) Serial.printf("%s\t", get_note_name(pitches[i]).c_str());
        }
        Serial.println("]");
      }
      for (int i = 0 ; i < 10 ; i++) {
        if (pitches[i]>=0) {
          if (DEBUG_HARMONY) Serial.printf("   >about to call send_note_on [%i] from list sending %i\r\n", i, pitches[i]);
          send_note_on(pitches[i], velocity); //velocity[i]);
          if (DEBUG_HARMONY) Serial.printf("   <did send_note_on %i with list sending %i\r\n", i, pitches[i]);
        } else {
          break;
        }
      }
      if (DEBUG_HARMONY)Serial.printf("<<< After ON for multiples on channel %i, notes held: %s\r\n", channel, get_debug_notes_held());
    }

    // send note off for multiples, expects array of size 10
    void send_note_off(int *pitches, int velocity = 0) {
      //Serial.println("--- starting send_note_off [multiple]");
      HARM_printf("Begin OFF for channel %i, notes held: %s\r\n", channel, get_debug_notes_held());

      if (DEBUG_HARMONY) {
        Serial.printf("   channel %i HarmonyOutput stopping chord: [ ", channel);
        for (int i = 0 ; i < 10 ; i++) {
          if (pitches[i]>-1) Serial.printf("%s\t", get_note_name(pitches[i]).c_str());
        }
        Serial.println("]");
      }
      
      // output debug info about multiple notes off sent
      if (DEBUG_HARMONY) {
        Serial.printf("   channel %i told to turn off notes: [", channel);
        for (int i = 0 ; i < 10 ; i++) {
          if (pitches[i]>-1) Serial.printf("%i, ", pitches[i]);
        }
        Serial.println("]");
  
        Serial.printf("   channel %i HarmonyOutput held notes: [", channel);
        for (int i = 0 ; i < 10 ; i++) {
          if (held_notes[i]>-1) Serial.printf("%i, ", held_notes[i]);
        }
        Serial.println("]");
      }

      for (int i = 0 ; i < 10 ; i++) {
        if (pitches[i]>=0) {
          HARM_printf("\tsend_note_off %i from list, sending %i [%s]\r\n", i, pitches[i], get_note_name(pitches[i]).c_str());
          if (is_note_held(pitches[i])) {
            send_note_off(pitches[i], velocity); //velocity[i]);
            i--;
          }
        } else {
          //Serial.printf("\tbreaking at %i cos pitch is %i\r\n", i, pitches[i]);
          break;
        }
      }
      //Serial.println("--- finished send_note_off [multiple]");
      HARM_printf("== After OFF for multiples on channel %i, notes held: %s\r\n", channel, get_debug_notes_held());
    }

    // actually send note on/off midi for one specified pitch
    void send_note_on(int pitch, int velocity = 127) {
      /*if (currently_playing>=0) {
        Serial.printf("Harmony #%i WARNING: asked to play note %i, but already playing %i!\r\n", channel, pitch, currently_playing);
      }*/
      // from midi_send_note_on(bass_currently_playing, velocity);

      if (pitch==-1) return;
      if (channel==0) return;

      int offset = 12*octave_offset;

      if (offset + pitch < MINIMUM_PITCH || offset + pitch > MAXIMUM_PITCH)
        return;
        
      handle_note_on((byte)pitch);
      
      pitch += offset; //(12*octave_offset);

      // Serial.printf("actual send_note_ON for pitch %i on channel %i\r\n", pitch, channel); 
      HARM_printf("\t\tactual send_note_on  on channel %i for pitch %i [%s] at velocity %i\r\n", (byte)channel, pitch, get_note_name(pitch).c_str(), velocity);
      MIDIOUT.sendNoteOn((byte)pitch, (byte)velocity, (byte)channel);

      // todo - possibly put the fire_envelope_for_channel here instead?
      // but how do we tell when its a chord we're playing rather than just a single note so we dont just retrigger? :/
      //      do we even need to care about that actually?
      fire_envelope_for_channel(channel, velocity);

      if (midiecho_enabled)
        MIDIIN.sendNoteOn((byte)pitch, (byte)velocity, (byte)channel);  // echo back to host
    }
 
    void send_note_off(int pitch, int velocity = 0) {
      //BASS_printf("bass_note_off: bass pitch offset is %i\r\n", bass_currently_playing);
      // from bass_note_off()
      //if (!bass_currently_playing && (!bpm_internal_mode && bass_only_note_held && !autobass_input.is_note_held()))
      //  return;

      if (channel==0) return;
      if (pitch==-1) return;
      if (!is_note_held((int)pitch)) {
        Serial.printf("WARN: send_note_off told to OFF for pitch %i, but we think it isn't held\r\n", pitch);
        return;
      }

      int offset = (12*octave_offset);

      if (offset + pitch < MINIMUM_PITCH || offset + pitch > MAXIMUM_PITCH)
        return;

      //Serial.printf("actual send_note_off, pitch %i is indeed held\r\n", pitch);
      handle_note_off((byte)pitch);

      pitch += offset;

      //midi_bass_send_note_off(bass_currently_playing, 0, MIDI_CHANNEL_BASS_OUT);
      if (DEBUG_HARMONY) Serial.printf("\t\tactual send_note_off on channel %i for pitch %i [%s] at velocity %i\r\n", channel, pitch, get_note_name(pitch).c_str(), velocity);
      MIDIOUT.sendNoteOff(pitch, velocity, channel);

      if (!is_note_held())
        douse_envelope_for_channel(channel);

      if (midiecho_enabled)
        MIDIIN.sendNoteOff(pitch, velocity, channel);  // echo back to host 
    }

    // turn off all the held notes on this output
    void send_all_notes_off() {
      if (channel==0) return;
      
      if (is_note_held()) {
        if (DEBUG_HARMONY) Serial.printf("send_all_notes_off on channel %i! ", channel);
        debug_notes_held();
        send_note_off(get_held_notes());
      } else {
        OUT_printf("WARNING: send_all_notes_off but nothing held on channel %i!\r\n", channel);
      }
      
      //MIDIOUT.sendControlChange (MIDI_CC_ALL_NOTES_OFF, 0, channel);   // 123 = kill all notes
      /*for (int i = 0 ; i < 127 ; i++) {
        MIDIOUT.sendNoteOff(i, 0, channel);
      }*/
      if (midiecho_enabled)
        MIDIIN.sendControlChange (MIDI_CC_ALL_NOTES_OFF, 0, channel);   // 123 = kill all notes - for midiecho back to host
      
      handle_all_notes_off();
    }

    void set_octave_offset(int offset) {
      octave_offset = offset;
      //send_note_off(held_notes);
      if (octave_offset==offset) return;
      
      send_all_notes_off();
      //handle_all_notes_off();
    }

    // set the midi channel for this output
    void set_midi_channel(int chan) {
      //send_note_off(held_notes);
      if (chan==channel) return;
      
      if (DEBUG_HARMONY) Serial.printf("#### set_midi_channel was %i, now %i\r\n", channel, chan);
      send_all_notes_off();
      //handle_all_notes_off();
      channel = chan;
    }
};

#endif

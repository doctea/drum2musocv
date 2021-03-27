#ifndef MIDIKEYS_INCLUDED
#define MIDIKEYS_INCLUDED

#define DEBUG_HARMONY false

#include "ChannelState.hpp"

// for providing multiple MIDI key outputs ie bass and harmony
//  todo: properly handle multiple notes being on at the same time for tracking chords
class MidiKeysOutput : public ChannelState {
  public:

    int channel = 0;
    
    MidiKeysOutput(int chan, int octave_off = 0) {
      channel = chan;
      octave_offset = octave_off;
    }

    // send_note_on/off multiples, expects array of size 10
    // todo: make this handle velocity per note
    void send_note_on(int *pitches, int velocity = 127) { //velocity[10] = { 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 }) {
      //Serial.printf("send_note_on!\r\n");
      if (DEBUG_HARMONY) Serial.printf(">>> Start ON for multiples on c%i, held: %s\r\n", channel, get_debug_notes_held());

      Serial.printf("   channel %i HarmonyOutput  playing chord: [ ", channel);
      for (int i = 0 ; i < 10 ; i++) {
        if (pitches[i]>-1) Serial.printf("%s\t", get_note_name(pitches[i]).c_str());
      }
      Serial.println("]");
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
      if (DEBUG_HARMONY) Serial.printf("Begin OFF for channel %i, notes held: %s\r\n", channel, get_debug_notes_held());

      Serial.printf("   channel %i HarmonyOutput stopping chord: [ ", channel);
      for (int i = 0 ; i < 10 ; i++) {
        if (pitches[i]>-1) Serial.printf("%s\t", get_note_name(pitches[i]).c_str());
      }
      Serial.println("]");
      
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
          if (DEBUG_HARMONY)  Serial.printf("send_note_off %i with list, sending %i\r\n", i, pitches[i]);
          send_note_off(pitches[i], velocity); //velocity[i]);
        } else {
          break;
        }
      }
      //Serial.println("--- finished send_note_off [multiple]");
      if (DEBUG_HARMONY) Serial.printf("== After OFF for multiples on channel %i, notes held: %s\r\n", channel, get_debug_notes_held());
    }

    // actually send note on/off midi for one specified pitch
    void send_note_on(int pitch, int velocity = 127) {
      /*if (currently_playing>=0) {
        Serial.printf("Harmony #%i WARNING: asked to play note %i, but already playing %i!\r\n", channel, pitch, currently_playing);
      }*/
      // from midi_send_note_on(bass_currently_playing, velocity);

      if (pitch==-1) 
        return;
      if (channel>0) {
        handle_note_on((byte)pitch);
        
        pitch += (12*octave_offset);

        //Serial.printf("actual send_note_ON for pitch %i on channel %i\r\n", pitch, channel); 
        MIDIOUT.sendNoteOn(pitch, velocity, channel);
        if (midiecho_enabled)
          MIDIIN.sendNoteOn(pitch, velocity, channel);  // echo back to host
      }
    }
 
    void send_note_off(int pitch, int velocity = 0) {
      //BASS_printf("bass_note_off: bass pitch offset is %i\r\n", bass_currently_playing);
      // from bass_note_off()
      //if (!bass_currently_playing && (!bpm_internal_mode && bass_only_note_held && !autobass_input.is_note_held()))
      //  return;

      if (channel==0) return;

      if (pitch==-1) 
        return;

      if (is_note_held((int)pitch)) {
        //Serial.printf("actual send_note_off, pitch %i is indeed held\r\n", pitch);
        handle_note_off((byte)pitch);

        pitch += (12*octave_offset);

        //midi_bass_send_note_off(bass_currently_playing, 0, MIDI_CHANNEL_BASS_OUT);
        //Serial.printf("actual send_note_off for pitch %i on channel %i\r\n", pitch, channel);
        MIDIOUT.sendNoteOff(pitch, velocity, channel);
        if (midiecho_enabled)
          MIDIIN.sendNoteOff(pitch, velocity, channel);  // echo back to host 
      } else {
        Serial.printf("WARN: send_note_off told to OFF for pitch %i, but we think it isn't held\r\n", pitch);
      }
    }

    void send_all_notes_off() {
      if (channel==0) return;
      
      if (is_note_held())
        send_note_off(held_notes);
      Serial.printf("send_all_notes_off sending off on channel %i!\r\n", channel);
      MIDIOUT.sendControlChange (MIDI_CC_ALL_NOTES_OFF, 0, channel);   // 123 = kill all notes
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
      handle_all_notes_off();
    }

    void set_midi_channel(int chan) {
      //send_note_off(held_notes);
      if (chan==channel) return;
      
      send_all_notes_off();
      handle_all_notes_off();
      channel = chan;
    }


    
  private:

    //int currently_playing = -1;
    //int currently_playing[10] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
    int octave_offset = 0;

};

#endif

#ifndef MIDIKEYS_INCLUDED
#define MIDIKEYS_INCLUDED

#include "ChannelState.hpp"

// for providing multiple MIDI key outputs ie bass and harmony
//  todo: properly handle multiple notes being on at the same time for tracking chords
class MidiKeysOutput : public ChannelState {
  public:
    MidiKeysOutput(int chan, int octave_off = 0) {
      channel = chan;
      octave_offset = octave_off;
    }

    // send_note_on/off multiples
    // todo: make this handle velocity per note
    void send_note_on(int *pitches, int velocity = 127) { //velocity[10] = { 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 }) {
      //Serial.printf("send_note_on!\r\n");
      for (int i = 0 ; i < 10 ; i++) {
        if (pitches[i]>=0) {
          //Serial.printf("send_note_on %i with list sending %i\r\n", i, pitches[i]);
          send_note_on(pitches[i], velocity); //velocity[i]);
        } else {
          break;
        }
      }
      Serial.printf("== After ON for channel %i, notes held: %s\r\n", channel, get_debug_notes_held());
    }

    // expects array of size 10
    void send_note_off(int *pitches, int velocity = 0) {
      //Serial.println("--- starting send_note_off [multiple]");
      //Serial.printf("After OFF for channel %i, notes held: %s\r\n", channel, get_debug_notes_held());
      for (int i = 0 ; i < 10 ; i++) {
        if (pitches[i]>=0) {
          //Serial.printf("send_note_off %i with list sending %i\r\n", i, pitches[i]);
          send_note_off(pitches[i], velocity); //velocity[i]);
        } else {
          break;
        }
      }
      //Serial.println("--- finished send_note_off [multiple]");
      Serial.printf("== After OFF for channel %i, notes held: %s\r\n", channel, get_debug_notes_held());
    }

    // actually send note on/off midi for one specified pitch
    void send_note_on(int pitch, int velocity = 127) {
      /*if (currently_playing>=0) {
        Serial.printf("Harmony #%i WARNING: asked to play note %i, but already playing %i!\r\n", channel, pitch, currently_playing);
      }*/
      // from midi_send_note_on(bass_currently_playing, velocity);

      handle_note_on((byte)pitch);
      
      pitch += (12*octave_offset);

      MIDIOUT.sendNoteOn(pitch, velocity, channel);
      if (midiecho_enabled)
        MIDIIN.sendNoteOn(pitch, velocity, channel);  // echo back to host
    }
 
    void send_note_off(int pitch, int velocity = 0) {
      //BASS_printf("bass_note_off: bass pitch offset is %i\r\n", bass_currently_playing);
      // from bass_note_off()
      //if (!bass_currently_playing && (!bpm_internal_mode && bass_only_note_held && !autobass_input.is_note_held()))
      //  return;

      if (is_note_held((byte)pitch)) {
        handle_note_off((byte)pitch);

        pitch += (12*octave_offset);

        //midi_bass_send_note_off(bass_currently_playing, 0, MIDI_CHANNEL_BASS_OUT);
        MIDIOUT.sendNoteOff(pitch, velocity, channel);
        if (midiecho_enabled)
          MIDIIN.sendNoteOff(pitch, velocity, channel);  // echo back to host 
      }    
    }

    void send_all_notes_off() {
      handle_all_notes_off();
      
      MIDIOUT.sendControlChange (123, 0, channel);   // 123 = kill all notes
      if (midiecho_enabled)
        MIDIIN.sendControlChange (123, 0, channel);   // 123 = kill all notes - for midiecho back to host
    }

    
  private:
    int channel = 0;
    //int currently_playing = -1;
    //int currently_playing[10] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
    int octave_offset = 0;

};

#endif

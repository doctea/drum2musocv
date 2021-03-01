#ifndef HARMONY_INCLUDED
#define HARMONY_INCLUDED

#include "bass.hpp"       // for access to the bass channel info  -- to be deprecated ? 

// for providing multiple MIDI key outputs ie bass and harmony
//  todo: properly handle multiple notes being on at the same time for tracking chords
class MidiKeysOutput {
  public:
    MidiKeysOutput(int chan, int octave_off = 0) {
      channel = chan;
      octave_offset = octave_off;
    }

    // send_note_on/off multiplexors
    // todo: make this handle velocity per note
    void send_note_on(int pitch[10], int velocity = 127) { //velocity[10] = { 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 }) {
      //Serial.printf("send_note_on!\r\n");
      for (int i = 0 ; i < 10 ; i++) {
        if (pitch[i]>=0) {
          //Serial.printf("send_note_on %i with list sending %i\r\n", i, pitch[i]);
          send_note_on(pitch[i], velocity); //velocity[i]);
        }
      }
    }

    void send_note_off(int pitch[10], int velocity = 0) {
      for (int i = 0 ; i < 10 ; i++) {
        if (pitch[i]>=0) {
          //Serial.printf("send_note_off %i with list sending %i\r\n", i, pitch[i]);
          send_note_off(pitch[i], velocity); //velocity[i]);
        }
      }
    }

    // actually send note on/off midi for one specified pitch
    void send_note_on(int pitch, int velocity = 127) {
      if (currently_playing>=0) {
        Serial.printf("Harmony #i WARNING: asked to play note %i, but already playing %i!\r\n", channel, pitch, currently_playing);
      }
      // from midi_send_note_on(bass_currently_playing, velocity);
      pitch += (12*octave_offset);
      currently_playing = pitch;

      MIDIOUT.sendNoteOn(pitch, velocity, channel);
      if (midiecho_enabled)
        MIDIIN.sendNoteOn(pitch, velocity, channel);  // echo back to host
    }
 
    void send_note_off(int pitch, int velocity = 0) {
      //BASS_printf("bass_note_off: bass pitch offset is %i\r\n", bass_currently_playing);
      // from bass_note_off()
      //if (!bass_currently_playing && (!bpm_internal_mode && bass_only_note_held && !autobass_input.is_note_held()))
      //  return;
      pitch += (12*octave_offset);
    
      if (currently_playing >= 0) {
        //midi_bass_send_note_off(bass_currently_playing, 0, MIDI_CHANNEL_BASS_OUT);
        MIDIOUT.sendNoteOff(pitch, velocity, channel);
        if (midiecho_enabled)
          MIDIIN.sendNoteOff(pitch, velocity, channel);  // echo back to host 
      }
    
      currently_playing = -1;
    }

    void send_all_notes_off() {
      MIDIOUT.sendControlChange (123, 0, channel);   // 123 = kill all notes
    }
    
  private:
    int channel = 0;
    int currently_playing = -1;
    int octave_offset = 0;

};

// track the current harmony state and send MIDI as appropriate when triggered
class Harmony {

  private:
    int last_root = -1;
    int last_melody_root = -1;
    ChannelState&   channel_state;
    MidiKeysOutput& mko_bass;
    MidiKeysOutput& mko_keys;

  public:  
    Harmony(ChannelState& channel_state_, MidiKeysOutput& mko_bass_, MidiKeysOutput& mko_keys_): channel_state(channel_state_), mko_bass(mko_bass_), mko_keys(mko_keys_)
    { // todo: pass in config settings
      channel_state = channel_state_;
      mko_bass = mko_bass_;
      mko_keys = mko_keys_;
    }

    void fire_melody () {
      // send notes to melody appropriate for current status
      //  to include options for playing root, full chords or arpeggiation

      if (bass_only_note_held && !channel_state.is_note_held()) {
        return;
      }
      
      int pitch = MIDI_BASS_ROOT_PITCH;
      pitch = channel_state.get_root_note();

      // get the current chord number
      if (!channel_state.is_note_held()) {
        pitch = channel_state.get_root_note() + bass_get_scale_note();
      }
    
      last_melody_root = pitch;   

      if (channel_state.is_note_held())
        mko_keys.send_note_on(channel_state.get_held_notes(), 127);  // send all held notes
      else
        mko_keys.send_note_on(pitch, 127);    // send a single pitch  
    }
    
    void douse_melody () {
      // send notes to douse playing melody
      int pitch = last_melody_root;
      //mko_keys.send_note_off(pitch);
      mko_keys.send_note_off(channel_state.get_held_notes(), 127);
      last_melody_root = -1;
    }

    
    void fire_bass () {
      // send notes to bass appropriate for current status
      
      if (bass_only_note_held && !channel_state.is_note_held()) {
        return;
      }

      int pitch = MIDI_BASS_ROOT_PITCH;       // TODO: adjust pitch if required by the mode
      pitch = channel_state.get_root_note();

      // todo: adjust if arping / progressioning / etc

      // get the current chord number
      if (!channel_state.is_note_held()) {
        pitch = channel_state.get_root_note() + bass_get_scale_note();
      }
      Serial.printf("harmony.fire_bass() told to fire pitch %i?\r\n", pitch);

      last_root = pitch;
      //Serial.printf("mko_bass channel is %i\r\n", mko_bass.channel);
      mko_bass.send_note_on(pitch, 127);
    }

    void douse_bass () {
      // send notes to douse bass
      //bass_note_off();
      int pitch = last_root;
      mko_bass.send_note_off(pitch);
      last_root = -1;
    }

    // todo: checking/configure which should fire
    void fire_both () {
      fire_bass();
      fire_melody();
    }
    void douse_both () {
      douse_bass();
      douse_melody();
    }
};


MidiKeysOutput mkob = MidiKeysOutput(MIDI_CHANNEL_BASS_OUT);
MidiKeysOutput mkok = MidiKeysOutput(MIDI_CHANNEL_BITBOX_KEYS, 2);  // +2 octave offset

// for use globally

Harmony harmony = Harmony(
  autobass_input, 
  mkob,
  mkok
);

#endif

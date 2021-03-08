#ifndef HARMONY_INCLUDED
#define HARMONY_INCLUDED

#include "bass.hpp"       // for access to the bass channel info  -- to be deprecated ? 

#define CC_HARMONY_MELODY_MODE  29
#define CC_HARMONY_MUTATE_MODE  30

namespace HARMONY {
  enum MUTATION_MODE { 
    MUTATION_MODE_NONE,
    RANDOMISE,
    MUTATION_MODE_MAX
  };

  enum MELODY_MODE {
    MELODY_MODE_NONE,
    SINGLE,
    CHORD,
    ARPEGGIATE,
    MELODY_MODE_MAX
  };
}

// for providing multiple MIDI key outputs ie bass and harmony
//  todo: properly handle multiple notes being on at the same time for tracking chords
class MidiKeysOutput {
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
          Serial.printf("send_note_on %i with list sending %i\r\n", i, pitches[i]);
          send_note_on(pitches[i], velocity); //velocity[i]);
        }
      }
    }

    // expects array of size 10
    void send_note_off(int *pitches, int velocity = 0) {
      for (int i = 0 ; i < 10 ; i++) {
        if (pitches[i]>=0) {
          //Serial.printf("send_note_off %i with list sending %i\r\n", i, pitch[i]);
          send_note_off(pitches[i], velocity); //velocity[i]);
        }
      }
    }

    // actually send note on/off midi for one specified pitch
    void send_note_on(int pitch, int velocity = 127) {
      if (currently_playing>=0) {
        Serial.printf("Harmony #%i WARNING: asked to play note %i, but already playing %i!\r\n", channel, pitch, currently_playing);
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
      if (midiecho_enabled)
        MIDIIN.sendControlChange (123, 0, channel);   // 123 = kill all notes - for midiecho back to host
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
    int mutation_mode = HARMONY::MUTATION_MODE::RANDOMISE;
    int melody_mode =   HARMONY::MELODY_MODE::CHORD;

  public:  
    Harmony(ChannelState& channel_state_, MidiKeysOutput& mko_bass_, MidiKeysOutput& mko_keys_): channel_state(channel_state_), mko_bass(mko_bass_), mko_keys(mko_keys_)
    { // todo: pass in config settings
      channel_state = channel_state_;
      mko_bass = mko_bass_;
      mko_keys = mko_keys_;
    }

    // MELODY
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
        //pitch = channel_state.get_root_note() + bass_get_scale_note();
        pitch = bass_get_sequence_pitch(bass_counter);
      }
    
      last_melody_root = pitch;   

      if (channel_state.is_note_held()) {
        mko_keys.send_note_on(channel_state.get_held_notes(), 127);  // send all held notes
      } else {
        if (melody_mode==HARMONY::MELODY_MODE::MELODY_MODE_NONE) {
          // do nothing
        } else if (melody_mode==HARMONY::MELODY_MODE::SINGLE) {
          mko_keys.send_note_on(pitch, 127);    // send a single pitch  
        } else if (melody_mode==HARMONY::MELODY_MODE::CHORD) {
          Serial.println("using chord mode for melody!");
          mko_keys.send_note_on(get_chord_for_pitch(pitch), 127);
        } else if (melody_mode==HARMONY::MELODY_MODE::ARPEGGIATE) {
          // TODO
        }
      }
    }

    // return pointer to array
    int *get_chord_for_pitch (int pitch) {
      /*static int pitches[10] = { 
        pitch, 
        pitch + bass_scale_offset[scale_number][2], 
        pitch + bass_scale_offset[scale_number][4], 
        //pitch + bass_scale_offset[scale_number][6], // 7th chord
        -1, -1, -1, -1, -1, -1, -1
      };  // send a triad based on the root*/

      // todo: different chord shapes, inversions etc...
      
      static int pitches[10] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } ;
      pitches[0] = pitch;
      pitches[1] = pitch + bass_scale_offset[scale_number][2];
      pitches[2] = pitch + bass_scale_offset[scale_number][4];
      //pitches[3] = pitch + bass_scale_offset[scale_number][6]; // add 7th chord
      //pitches[3] = pitch + bass_scale_offset[scale_number][8]; // add 9th chord
      //-1, -1, -1, -1, -1, -1, -1
      
      return pitches;
    }
    
    void douse_melody () {
      // send notes to douse playing melody
      int pitch = last_melody_root;
      //mko_keys.send_note_off(pitch);
      mko_keys.send_note_off(channel_state.get_held_notes(), 127);
      last_melody_root = -1;
    }

    // BASS
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
        //pitch = bass_get_sequence_pitch(bass_counter);
      }
      Serial.printf("harmony.fire_bass() told to fire pitch %i?\r\n", pitch);

      last_root = pitch;
      //Serial.printf("mko_bass channel is %i\r\n", mko_bass.channel);
      mko_bass.send_note_on(pitch, 127);

      bass_counter++;
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

    void kill_notes() {
        mko_bass.send_all_notes_off();
        mko_keys.send_all_notes_off();
    }

    void mutate() {
      if (mutation_mode==HARMONY::MUTATION_MODE::RANDOMISE) {
        for (int i = 0 ; i < 4 ; i++) {
          if (random(10)<2)
            chord_progression[i] = random(0,8);
            
          for (int x = 0 ; x < 4 ; x++) {
            if (random(10)<5)
              bass_sequence[i][x] = random(0,8);
          }
        }
      }
    }


    bool handle_ccs(int channel, int number, int value) {
      if (channel!=GM_CHANNEL_DRUMS) return false;
    
      if (number==CC_HARMONY_MUTATE_MODE) {
        mutation_mode = value % HARMONY::MUTATION_MODE::MUTATION_MODE_MAX;
        Serial.printf("Setting harmony mutation_mode to %i\n", mutation_mode);
        return true;
      } else if (number==CC_HARMONY_MELODY_MODE) {
        melody_mode = value % HARMONY::MELODY_MODE::MELODY_MODE_MAX;
        Serial.printf("Setting melody mode to %i\r\n", melody_mode);
        //Serial.printf("Sizeof harmony modes is %i\r\n", sizeof(HARMONY::MELODY_MODE));
        return true;
      }
      return false;
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

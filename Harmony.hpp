#ifndef HARMONY_INCLUDED
#define HARMONY_INCLUDED

#include "Profiler.hpp"

#define DEBUG_HARMONY false
//#define HARM_DEBUG
// handling debugging output - pattern from https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c/1644898#1644898
#ifdef HARM_DEBUG
#define HARM_DEBUG 1
#else
#define HARM_DEBUG 0
#endif
#define HARM_printf(fmt, ...)   do { if (HARM_DEBUG) Serial.printf((fmt), ##__VA_ARGS__); } while (0)
#define HARM_println(fmt, ...)  do { if (HARM_DEBUG) Serial.println((fmt), ##__VA_ARGS__); } while (0)
//debug handling

#define DEFAULT_MELODY_OFFSET   1
#define DEFAULT_PAD_ROOT_OUT_OFFSET   0 //-2

#define DEFAULT_AUTO_PROGRESSION_ENABLED  true   // automatically play chords in progression order?
#define DEFAULT_BASS_ONLY_WHEN_NOTE_HELD  false  // 
#define DEFAULT_SCALE                     0      // 0 = major, 1 = minor...
#define DEFAULT_AUTO_SCALE_ENABLED        false  // true


// CONFIGURATION: messages targeted to channel _IN will be relayed on channel _OUT -- for passing through messages to Neutron (TODO: probably move this to a dedicated config file)
#define MIDI_CHANNEL_BASS_IN        8     // channel to receive direct bass playing
#define MIDI_CHANNEL_BASS_AUTO_IN   9     // channel to receive automatic bass notes
#define MIDI_CHANNEL_MELODY_IN      3     // channel to receive direct melody/chords
#define DEFAULT_MIDI_CHANNEL_BASS_OUT       4   //2     // channel to output bass notes on
#define DEFAULT_MIDI_CHANNEL_BITBOX_KEYS 3 // bass output, but shifted an octave
#define MIDI_CHANNEL_PAD_ROOT_IN    1
#define MIDI_CHANNEL_PAD_PITCH_IN   2
#define DEFAULT_MIDI_CHANNEL_PAD_ROOT_OUT   1
#define DEFAULT_MIDI_CHANNEL_PAD_PITCH_OUT  2

// channels that the outputs use
#define MIDI_CHANNEL_BASS_OUT       (harmony.get_midi_channel_bass_out())       // output 0
#define MIDI_CHANNEL_BITBOX_KEYS    (harmony.get_midi_channel_bitbox_keys())    // output 1
#define MIDI_CHANNEL_PAD_ROOT_OUT   (harmony.get_midi_channel_pads_root())      // output 2
#define MIDI_CHANNEL_PAD_PITCH_OUT  (harmony.get_midi_channel_pads_pitch())     // output 3

#define TRIGGER_HARMONY_BASS      16
#define TRIGGER_HARMONY_MELODY    17
#define TRIGGER_HARMONY_PAD_ROOT  18
#define TRIGGER_HARMONY_PAD_PITCH 19

//BITBOX/melody settings
#define BITBOX_NOTE_MINIMUM         36  // https://1010music.com/wp-content/uploads/2020/08/bitbox-mk2-1.0.8-user-manual.pdf "MIDI inputs for notes 36 to 51 map to the pads", "EXT1 through EXT4 are assigned notes 55 to 52 for use as Recording triggers"
#define BITBOX_KEYS_OCTAVE_OFFSET   2

/*
// deprecated arpegiation settings
#define ARP_MODE_NONE         0
#define ARP_MODE_PER_BEAT     1
#define ARP_MODE_NEXT_ON_NOTE 2
#define ARP_MODE_MAX          3
#define DEFAULT_ARP_MODE          ARP_MODE_NONE   // choose notes to play from the current sequence (eg incrementing through them)?
#define CC_BASS_SET_ARP_MODE        17    // cc to set the bass arp mode
*/

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

  enum CHORD_TYPE {
    TRIAD,
    SEVENTH,
    EIGHTH,
    NINETH,
    TENTH,
    ELEVENTH,
    TWELVETH,
    THIRTEENTH,
    CHORD_TYPE_MAX
  };
}

//#include "bass.hpp"       // for access to the bass channel info  -- to be deprecated ? 
#include "Drums.h"
#include "HarmonyOutput.hpp"
#include "BPM.hpp"
#include "ChannelState.hpp"

ChannelState autobass_input = ChannelState();   // global tracking notes that are held on incoming bass channel


///////////////////// overall harmony settings /////////////////////
#define CC_HARMONY_MUTATE_MODE      30    // overall harmony mutation mode, 0=None, 1=Randomise
#define CC_AUTO_PROGRESSION         31    // enable/disable playing auto chord progression
#define CC_AUTO_CHORD_TYPE          105   // enable/disable playing automatic chord types (ie stacking triads)
#define CC_AUTO_CHORD_INVERSION     106   // enable/disable playing automatic chord inversions
#define CC_MELODY_SCALE             108   // choose scale to use, 0=major, 1=natural minor, 2=melodic minor, 3=harmonic minor, 4=lydian, 5=whole tone, 6=blues
#define CC_MELODY_AUTO_SCALE        109   // enable/disable automatic changing of scale every phrase

#define CC_MELODY_ROOT              11    // set the MIDI note to use as the root pitch, 48=C4
#define CC_HARMONY_ONLY_NOTE_HELD      18    // cc to set bass to only play in external mode if note is held

///////////////////// output-specific harmony settings /////////////////////
//// output 0 (bass)
#define CC_CHANNEL_BASS_OUT         12    // output 0: set the MIDI channel to output the bass on (default 4)
#define CC_BASS_MELODY_MODE         2     // output 0: set mode to use for the bass output - 0=None, 1=Single note, 2=Chord, 3=Arpeggiate chord
#define CC_BASS_SET_TIE_ON          6     // output 0: set which steps the Euclidian bass should tie on
// not implemented: output 0 octave offset

//// output 1 (bitbox)
#define CC_CHANNEL_BITBOX_KEYS      13    // output 1: set the MIDI channel to output the chords on (default 3)
#define CC_HARMONY_MELODY_MODE      29    // output 1: set mode to use for the chords output - 0=None, 1=Single note, 2=Chord, 3=Arpeggiate chord
#define CC_MELODY_OCTAVE_OFFSET     107   // output 1: octave offset for melody, 0=-2, 1=-1, 2=0, 3=+1, 4=+2, 5=+3
// not implemented: output 1 tie-on 

//// output 2 (muso/ensemble root)
#define CC_CHANNEL_PAD_ROOT         10    // output 2: set the MIDI channel to output the pad root on (default 1)
#define CC_PAD_ROOT_MELODY_MODE     112   // output 2: set mode to use for the chords output - 0=None, 1=Single note, 2=Chord, 3=Arpeggiate chord
#define CC_PAD_ROOT_OCTAVE_OFFSET        113   // output 2: octave offset for melody, 0=-2, 1=-1, 2=0, 3=+1, 4=+2, 5=+3
// not implemented: output 2 melody mode
// not implemented: output 2 octave offset
// not implemented: output 2 tie-on 

//// output 3 (muso/ensemble pitch)
#define CC_CHANNEL_PAD_PITCH        5     // output 3: set the MIDI channel to output the pads pitch on (default 2)
#define CC_PAD_PITCH_MELODY_MODE    3     // output 3: set mode to use for the pitch output - 0=None, 1=Single note, 2=Chord, 3=Arpeggiate chord
#define CC_PAD_PITCH_OCTAVE_OFFSET  4     // output 3: set octave offset (-2 to +3)
// not implemented: output 3 tie-on 



#define CHORD_PROGRESSION_LENGTH  ((int)(sizeof(chord_progression)/sizeof(chord_progression[0])))      // how many chords in progression
#define NUM_SCALES                (sizeof(scale_offset) / sizeof(scale_offset[0]))      // how many scales we know about in total
#define NUM_SEQUENCES             (sizeof(sequence) / sizeof(sequence[0]))              // how many sequences we know about in total
#define HARM_SEQUENCE_LENGTH      ((int)(sizeof(sequence[0])/sizeof(sequence[0][0])))   // how many notes in arps

#define SCALE_SIZE  7

int scale_offset[][SCALE_SIZE] = {
  { 0, 2, 4, 5, 7, 9, 11 },     // major scale
  { 0, 2, 3, 5, 7, 8, 10 },     // natural minor scale
  { 0, 2, 3, 5, 7, 9, 11 },     // melodic minor scale 
  { 0, 2, 3, 5, 7, 8, 11 },     // harmonic minor scale
  { 0, 2, 4, 6, 7, 9, 11 },     // lydian
  { 0, 2, 4, 6, 8, 10, (12) },  // whole tone - 6 note scale - flavours for matching melody to chords
  { 0, 3, 5, 6, 7, 10, (12) },  // blues - flavours for matching melody to chords
  // minor pent = natural minor but miss out 2nd and 8th
  // major pent = major but miss out 5th and 11th

  // mode of C - use chord but use the scale of
  // dorian D E F G A B C D
  //         2 1 2 2 2 2 2 
  // 6 modes per scale

  // relative major/minor are modes of each other
  //    C maj is also A minor
};

// for use by qsort
int sort_pitch(const void *cmp1, const void *cmp2)
{
  // Need to cast the void * to int *
  int a = *((int *)cmp1);
  int b = *((int *)cmp2);
  // The comparison
  //if (a==b==-1) return 0;
  if (b==-1 && a>b) return -1;
  if (a==-1 && b>a) return 1;
  
  return a < b ? 
          -1 : // a > b
          (a > b ? 
              1 : // a < b
              0); // equal
}

void sort_pitches(int pitches[], int len) {
  qsort(pitches, len, sizeof(pitches[0]), sort_pitch);
}


// track the current harmony state and send MIDI as appropriate when triggered
class Harmony {

  private:

    static int const NUM_MKO = 4;
    MidiKeysOutput mko[NUM_MKO] = {
        MidiKeysOutput(DEFAULT_MIDI_CHANNEL_BASS_OUT),
	      MidiKeysOutput(DEFAULT_MIDI_CHANNEL_BITBOX_KEYS,   BITBOX_KEYS_OCTAVE_OFFSET).set_melody_mode(HARMONY::MELODY_MODE::CHORD),  // with octave offset
        MidiKeysOutput(DEFAULT_MIDI_CHANNEL_PAD_ROOT_OUT,  DEFAULT_PAD_ROOT_OUT_OFFSET),  // with octave offset
        MidiKeysOutput(DEFAULT_MIDI_CHANNEL_PAD_PITCH_OUT, DEFAULT_MELODY_OFFSET).set_melody_mode(HARMONY::MELODY_MODE::ARPEGGIATE)  // with octave offset
    }; 
#define mko_bass        mko[0]
#define mko_keys        mko[1]
#define mko_pads_root   mko[2]
#define mko_pads_pitch  mko[3]

    int scale_number = DEFAULT_SCALE;   // index of the current scale we're in
    int chord_number = 0;               // index of the current chord degree that we're playing (0-6, well actually can be negative or go beyond that to access lower&higher octaves)
    int sequence_number = 0;            // index of the arp sequence that we're currently playing
    int sequence_counter = 0;           // track current position in arp sequence

    //int arp_counter = 0;  // todo, probably move this into the MidiKeysOutput..?
    
    int mutation_mode = HARMONY::MUTATION_MODE::RANDOMISE;
    //int melody_mode   = HARMONY::MELODY_MODE::CHORD;  // deprecated, is now per-output
    //int arp_mode      = DEFAULT_ARP_MODE; //ARP_MODE_NEXT_ON_NOTE;  // deprecated, is now per-output
    //bool auto_arp         = DEFAULT_AUTO_ARP_ENABLED;   // deprecated. choose notes to play from the current sequence (eg incrementing through them)

    bool auto_progression = DEFAULT_AUTO_PROGRESSION_ENABLED;   // automatically play chords in progression order
    bool auto_scale       = DEFAULT_AUTO_SCALE_ENABLED;   // automatically switch scales every phrase
    bool auto_chord_type  = true;
    bool auto_chord_inversion = true;
    bool only_note_held   = DEFAULT_BASS_ONLY_WHEN_NOTE_HELD;

    int default_chord_progression[4]    =   { 
      0, 5, 1, 4 
      //0, 5, 1, 5
    };     // default chord progression

    int default_sequence[4][4]     =   { // degrees of scale to play per chord -- ie, arp patterns .. deprecated now?
      { 0, 0, 0, 0 },
      { 0, 0, 0, 1 },
      { 0, 1, 1, 2 },
      { 0, 1, 0, 4 },
      //{ 0, 2, 4, 6 },
      /*{ 0, 2, 0, 4 },
        { 0, 3, 6, 4 }*/
    };
    int chord_progression[4];
    int sequence[4][4];   // deprecated now?

  public:  
 
    int last_note_on;

    ChannelState&   channel_state;
    Harmony(ChannelState& channel_state_): channel_state(channel_state_) {
      reset_progression();
      reset_sequence_pattern();
    }

    void process_ties() {
      for (int i = 0 ; i < NUM_MKO ; i++) {
        mko[i].process_tick_ties();
      }
    }

    void mutate_midi_root_pitch() {
      //Serial.printf("mutating root pitch from %i to %i\n", harmony.r, channel_state.last_note_on);
      kill_notes();
      set_midi_root_pitch(36 + scale_offset[scale_number][random(0,SCALE_SIZE)]);
      Serial.printf("mutating root pitch -- setting to %i\n", 36 + scale_offset[scale_number][random(0,SCALE_SIZE)]);// from %i to %i\n", harmony.r, channel_state.last_note_on);
      
      //channel_state.set_midi_root_pitch(channel_state.last_note_on);
    }

    void set_midi_root_pitch(int pitch) {
      channel_state.set_midi_root_pitch(pitch);
    }

    void set_progression(int source[4]) {
      for (int i = 0 ; i < CHORD_PROGRESSION_LENGTH ; i++) {
        chord_progression[i] = source[i];
      }
    }
    void reset_progression() { 
      //chord_progression[4]    =   { 0, 5, 1, 4 };
      set_progression(default_chord_progression);
    }

    // may be deprecated?
    void reset_sequence_pattern( int source[4][4]) {
      for (int i = 0 ; i < NUM_SEQUENCES ; i++) {
        for (int x = 0 ; x < HARM_SEQUENCE_LENGTH ; x++) {
          sequence[i][x] = source[i][x];
        }
      }
    }
    void reset_sequence_pattern () {
      reset_sequence_pattern(default_sequence);
    }

    // get the root pitch of the current chord.. TODO: may need to look at this to see if this is doing the right thing?
    int get_current_chord_root_pitch() {
     return channel_state.get_root_note() + get_scale_note(0, get_chord_number());
    }

    // to be used when relaying played notes from midi in..
    // this way envelopes will trigger too
    void send_note_on_for_channel(int channel, int pitch, int velocity) {
      HARM_printf("send_note_on_for_channel(chan%i, pitc%i, velo%i)\n", channel, pitch, velocity);
      for (int i = 0 ; i < NUM_MKO ; i++) {
        // translate i to trigger number ?
        if (mko[i].channel==channel) {
          mko[i].send_note_on(pitch, velocity);
        }
      }      
    }
    // to be used when relaying played notes from midi in..
    // this way envelopes will trigger too
    void send_note_off_for_channel(int channel, int pitch, int velocity = 0) {
      for (int i = 0 ; i < NUM_MKO ; i++) {
        if (mko[i].channel==channel) {
          mko[i].send_note_off(pitch, velocity);
        }
      }      
    }

    /*// kill only envelope output, to be used when an envelope changes its assigned trigger
    void fire_envelope_for_channel(int channel, int velocity = 127) {
      for (int i = 0 ; i < NUM_MKO ; i++) {
        if (mko[i].channel==channel) {
          mko[i].fire_envelope(velocity);
        }
      }
    }
    void douse_envelope_for_channel(int channel) {
      for (int i = 0 ; i < NUM_MKO ; i++) {
        if (mko[i].channel==channel) {
          mko[i].douse_envelope();
        }
      }
    }*/

    // trigger current pitch/chord on the specified harmony output
    void fire_for(int output_number) {
      unsigned long time = millis();
      //Serial.printf("fire_for output number %i\r\n", output_number);
   
      if (only_note_held && !channel_state.is_note_held()) {
        return;
      }

      // find the pitch to play
      int pitch = MIDI_BASS_ROOT_PITCH;
      pitch = channel_state.get_root_note();
      int *notes;

      // get the current chord number
      if (channel_state.is_note_held()) {
        // incoming keys held, so use lowest as our current pitch
        notes = channel_state.get_held_notes();
        pitch = notes[0];
      } else {
        // no notes held so find current notes to use based on autoharmony chord
        int chord_number = get_chord_number();
        int chord_type = get_chord_type();
        int inversion = get_chord_inversion();

        pitch = get_current_chord_root_pitch();

        notes = get_notes_for_chord(
            get_chord_number(),
            chord_type,
            inversion
          );
      } 

      last_note_on = pitch;

      bool fired = mko[output_number].fire_notes(pitch, notes);
      pf.l(PF::PF_HARMONY, millis()-time);
      if (fired) {
        update_envelopes_for_trigger(output_number + NUM_TRIGGERS + NUM_ENVELOPES, 127, true);
      }      
    }

    void douse_for(int output_number, bool tied = false) {
      unsigned long time = millis();
      HARM_printf("douse_for output number %i - channel %i\r\n", output_number, mko[output_number].channel);
      mko[output_number].douse_notes(tied);   // erm this doesnt seem to affect anything?!
      pf.l(PF::PF_HARMONY, millis()-time);
      if (!mko[output_number].is_note_held()) {
        update_envelopes_for_trigger(output_number + NUM_TRIGGERS + NUM_ENVELOPES, 0, false);
      }      
    }

    // douse all harmony output notes by calling douse_for
    void douse_all() {
      for (int i = 0 ; i < NUM_MKO ; i++) {
        douse_for(i);
      }
    }

    // kill all harmony output notes by calling send_all_notes_off
    void kill_notes() {
        HARM_println(">>>>>> harmony.kill_notes() called");
        for (int i = 0 ; i < NUM_MKO ; i++) {
          mko[i].send_all_notes_off();
        }
    }

    // mutate harmony chord progression / sequence
    void mutate() {
      unsigned long time = millis();

      if (mutation_mode==HARMONY::MUTATION_MODE::RANDOMISE) {
        HARM_println("## HARMONY MUTATE - randomise!");
        randomSeed(get_euclidian_seed());
        for (int i = 0 ; i < 4/*CHORD_PROGRESSION_LENGTH*/ ; i++) {
          if (random(0,10)<4) {
            chord_progression[i] = random(0,8);
            //Serial.printf ("randomised prog %i to %i !\r\n", i, chord_progression[i]);
          }

          Serial.printf(" sequence %i now [ ", i);
          for (int x = 0 ; x < HARM_SEQUENCE_LENGTH ; x++) {
            if (random(0,10)<5) {
              sequence[i][x] = random(0,4);
            }
            Serial.printf("%i ", sequence[i][x]);
          }
          Serial.println("]");
        }
        Serial.print("  chords are now [ ");
        for (int x = 0 ; x < HARM_SEQUENCE_LENGTH ; x++) {
          Serial.printf("%i ", chord_progression[x]);
        }
        Serial.println("]");
      }
      pf.l(PF::PF_HARMONY, millis()-time);
    }

    /////// chord choosing, note choosing, etc

    // might be deprecated now?
    int get_sequence_note(int position = 0) { 
      if (position % HARM_SEQUENCE_LENGTH == 0) HARM_printf("----- at bass arp sequence start (position %i)\r\n", position);
      int sequence_number = get_sequence_number();
    
      return get_scale_note(
        sequence [sequence_number] [position % HARM_SEQUENCE_LENGTH]
      );
    }

    bool is_note_held(int channel) {
      for (int i = 0 ; i < NUM_MKO ; i++) {
        if (mko[i].channel==channel) {
          if (mko[i].is_note_held()) return true;
          //mko[i].send_note_off(pitch, velocity);
        }
      } 
      return false;     
    }

    // might be deprecated now?
    int get_sequence_pitch(int position = 0) {  
      //bass_root = MIDI_BASS_ROOT_PITCH;// + current_phrase;
      if (channel_state.is_note_held()) {
        // bass is autoplaying, so ask it for what note it recommends for this position
        return channel_state.get_sequence_held_note(position);
      }
      // else base it on the root note and the sequence's position
      return channel_state.get_root_note() + get_sequence_note(position);
    }
    
    // get the currently active scale
    int get_scale_number() {
      if (auto_scale) {
        // automatically change scales based on our current song phrase position
        return scale_number = (0 + current_phrase) % NUM_SCALES; // todo: make this switchable ..
      }
      
      return scale_number; // default to major
    }

    /*int get_currently_playing_root() {
      return last_melody_root;
    }*/

    void reset_sequence() {
      sequence_counter = 0;
    }
    
    // get the root note for the scale chord -- can go negative/higher than 6 to access other octaves
    // part of harmony
    int get_scale_note(int scale_degree = 0, int chord_number = -100) {
      // todo: move this elsewhere to make this changeable..
      //int scale_number = 0;
      int scale_number = get_scale_number();
      if (chord_number==-100) 
        chord_number = get_chord_number();
    
      // temporary step through chord number based on the current_bar (so resets for each new phrase)
      //scale_degree += chord_number;
    
      int sd = (chord_number + scale_degree);
      int oct = sd / SCALE_SIZE;

//#define HARM_DEBUG 1
      HARM_printf("bass_get_scale: in phrase:bar:beat %i:%i:%i, using scale %i ", current_phrase, current_bar, current_beat, scale_number );
      HARM_printf(" || chord %i, scale degree %i -> got scale offset %i and oct %i -> ",  chord_number, scale_degree, sd, oct);
      if (sd < 0) {
        sd = SCALE_SIZE + sd;  // convert to the actual scale degree
        oct--;                      // account for needing to be one octave below
      } /*else if (sd>SCALE_SIZE) {
        oct = 1;
      }*/
      HARM_printf("final oct:sd is %i:%i, sdMscale_size %i", oct, sd, sd % SCALE_SIZE);
   
      int r = scale_offset[scale_number][sd % SCALE_SIZE]
              +
              (oct * 12)
              ;
    
      HARM_printf(" -- final pitch is %i (%s)\r\n", r, get_note_name(channel_state.get_root_note()+r).c_str());
//#define HARM_DEBUG 0
      return r;
    }

    // get the currently active arp sequence index - might be deprecated now?
    int get_sequence_number() {
      /*if (arp_mode>0) { //auto_arp) {
        // choose the arp sequence automatically based on the current song position
        sequence_number = current_bar % NUM_SEQUENCES;
      } else {*/
        sequence_number = 0;
      /*}*/
      
      return sequence_number;
    }

    // get the currently active chord number in the scale based on autoharmony progression, or 0
    int get_chord_number() {
      if (auto_progression) {
        // automatically change chord based on our current song bar position
        int c = chord_progression[current_bar % CHORD_PROGRESSION_LENGTH] % SCALE_SIZE;
        HARM_printf("get_chord_number returning %i for chord progression on bar %i!\r\n", c, current_bar%CHORD_PROGRESSION_LENGTH);
        return c; // todo: make this select from lowest held note?
      }
      
      return 0; // default to root (default or whatever is held)
    }

    // todo: can probably do away with all this by qsorting the pitches after adding/removal
    // todo: move this to ChannelState
    void replace_pitch(int pitches[], int pitch_search, int pitch_replace) {
      remove_pitch(pitches, pitch_search);
      insert_pitch(pitches, pitch_replace);
    }
    void insert_pitch(int pitches[], int pitch) {
      for (int i = 0 ; i < 10 ; i++) {
        if (pitches[i]==pitch) {
          // duplicate, ignore
          break;
        } else if (pitches[i]==-1) {
          // empty, add
          //Serial.printf("\tinsert_pitch: adding pitch %s [%i] at position %i\r\n", get_note_name(pitch).c_str(), pitch, i);
          pitches[i] = pitch;
          break;
        } else if (pitches[i]>pitch) {
          // higher found, insert at i 
          //Serial.printf("\tinsert_pitch: inserting pitch %s [%i] at position %i\r\n", get_note_name(pitch).c_str(), pitch, i);
          //for (int x = i+1 ; x < 10 ; x++) {
          for (int x = 8 ; x > i+1 ; x--) { // 8 because highest usable index is 9 and we use index+1 below
            //Serial.printf("\t\tshifting pitch at %i to %i (%i overwrites %i)\r\n", x, x+1, pitches[x-1], pitches[x]);
            pitches[x+1] = pitches[x]; 
          }
          pitches[i] = pitch;
          break;
        } else if (pitches[i]==-1) {
          break;
        }
      }
    }
    void remove_pitch(int pitches[], int pitch) {
      for (int i = 0 ; i < 10 ; i++) {
        if (pitches[i]==pitch) {
          //Serial.printf("\tremove_pitch: removing pitch %s [%i] from position %i\r\n", get_note_name(pitch).c_str(), pitch, i);
          // found, remove and shift
          for (int x = i+1 ; x < 10 ; x++) {
            pitches[x-1] = pitches[x];
            if (pitches[x]==-1) 
              break;
          }
          pitches[9] = -1;
          break;
        } else if (pitches[i]==-1) {
          break;
        }
      }
    }

    // determine current chord type to use, ie any extensions, otherwise triad
    int get_chord_type() {
      int type = 0;
      if (auto_chord_type) {
        type = HARMONY::CHORD_TYPE::TRIAD;
        if (current_bar%2==1) {   // second or fourth bar of a phrase
          type = HARMONY::CHORD_TYPE::SEVENTH;
          if (current_phrase%2==0)  // every other phrase 
            type = HARMONY::CHORD_TYPE::NINETH;
        }
        if (current_bar==3) { // if its the last bar of a phrase
          type += (current_phrase+current_beat)%HARMONY::CHORD_TYPE::CHORD_TYPE_MAX;  // mutate the chord type based on current phrase & current beat
        }
        type %= HARMONY::CHORD_TYPE::CHORD_TYPE_MAX;   
      } else {                     // default
        type = HARMONY::CHORD_TYPE::TRIAD;
      }
        
      return type;
    }

    // what chord inversion we should use right now
    int get_chord_inversion() {
      if (auto_chord_inversion) 
        return (current_bar%2==0 && current_beat+1%2==0) || current_bar==BARS_PER_PHRASE-1 
          ? current_beat : 0;
      else
        return 0;
    }

    // return pointer to array of the notes to use in specified chord number+type+inversion based on current root
    int *get_notes_for_chord (int chord, int chord_type = HARMONY::CHORD_TYPE::TRIAD, int inversion = 0) {
      /*static int pitches[10] = { 
        pitch, 
        pitch + scale_offset[scale_number][2], 
        pitch + scale_offset[scale_number][4], 
        //pitch + scale_offset[scale_number][6], // 7th chord
        -1, -1, -1, -1, -1, -1, -1
      };  // send a triad based on the root*/

      // todo: different chord shapes, inversions etc...

      static int pitches[10] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } ;

      int p = 0;
      pitches[p++] = channel_state.get_root_note() + get_scale_note(0, chord);
      /*pitches[1] = pitch + scale_offset[scale_number][2];
      pitches[2] = pitch + scale_offset[scale_number][4];
      pitches[3] = current_bar==BARS_PER_PHRASE-1 ? pitch + scale_offset[scale_number][6] : -1; // add 7th chord*/
      pitches[p++] = channel_state.get_root_note() + get_scale_note(2, chord);
      pitches[p++] = channel_state.get_root_note() + get_scale_note(4, chord);


      if (chord_type==HARMONY::CHORD_TYPE::SEVENTH) {
        if (DEBUG_HARMONY) Serial.println("!!! seventh chord?");
        //pitches[p++] = /*current_bar==BARS_PER_PHRASE-1 ? */
        insert_pitch(pitches, 
                     channel_state.get_root_note() + get_scale_note(6, chord));
                     //: -1; // add SEVENTH 
        p++;
      }
      if (chord_type==HARMONY::CHORD_TYPE::EIGHTH) {
        if (DEBUG_HARMONY) Serial.println("!!! eighth chord?");

        //pitches[p++] = /*current_bar==BARS_PER_PHRASE-1 ? */
        insert_pitch(pitches, 
                   channel_state.get_root_note() + get_scale_note(7, chord));
                   //: -1; // add NINTH
                   p++;
      }
      if (chord_type==HARMONY::CHORD_TYPE::NINETH) {
        if (DEBUG_HARMONY) Serial.println("!!! ninth chord?");

        //pitches[p++] = /*current_bar==BARS_PER_PHRASE-1 ? */
        insert_pitch(pitches, 
                   channel_state.get_root_note() + get_scale_note(8, chord));
                   //: -1; // add NINTH
                   p++;
      }
      if (chord_type==HARMONY::CHORD_TYPE::TENTH) {
        if (DEBUG_HARMONY) Serial.println("!!! tenth chord?");

        //pitches[p++] = /*current_bar==BARS_PER_PHRASE-1 ? */
        insert_pitch(pitches, 
                   channel_state.get_root_note() + get_scale_note(9, chord));
                   //: -1; // add NINTH
                   p++;
      }
      if (chord_type==HARMONY::CHORD_TYPE::ELEVENTH) {
        if (DEBUG_HARMONY) Serial.println("!!! eleventh chord?");
        //pitches[p++] = //current_bar==BARS_PER_PHRASE-1 ? 
        insert_pitch(pitches, 
                   channel_state.get_root_note() + get_scale_note(10, chord));
                   p++;
      }
      if (chord_type==HARMONY::CHORD_TYPE::TWELVETH) {
        Serial.println("!!! twelveth chord?");
        //pitches[p++] = //current_bar==BARS_PER_PHRASE-1 ? 
        insert_pitch(pitches, 
                   channel_state.get_root_note() + get_scale_note(11, chord));
                   p++;
      }
      if (chord_type==HARMONY::CHORD_TYPE::THIRTEENTH) {
        if (DEBUG_HARMONY) Serial.println("!!! thirteenth chord?");
        insert_pitch(pitches, 
        //pitches[p++] = //current_bar==BARS_PER_PHRASE-1 ? 
                   channel_state.get_root_note() + get_scale_note(12, chord) );
                   p++;
      }

      // fill the unused notes
      for (int i = p ; i < 10 ; i++) {
        pitches[i] = -1;
      }

      // do inversions
      do_inversion(pitches, inversion);
      
      if (DEBUG_HARMONY) {
        Serial.printf("get_chord_notes for rn %i : scale %i : chord %i : inversion %i : chord_type %i : [ ", channel_state.get_root_note(), get_scale_number(), chord, inversion, chord_type);
        for (int i = 0 ; i < 10 ; i++) {
          if (pitches[i]>-1) Serial.printf("%s ", get_note_name(pitches[i]).c_str());
        }
        Serial.println("] ");
      }
      
      return pitches;
    }

    // invert a chord
    void do_inversion(int pitches[10], int i) {
        int lowestpitch = pitches[0];
        
        pitches[i%3] -= 12;
        for (int x = 0 ; x < i/3 ; x++) {
          pitches[(i+3-x)%3] += ((i/3)+1)*12;
        }

        sort_pitches(pitches, 10);

        for (int x = 0 ; x < 10 ; x++) {
          if ((x==9 || pitches[x+1]==-1)) {
            if (pitches[x] > pitches[x-1]+12)
              pitches[x] -= 12;
            break;
          } else {
            if (pitches[x] < pitches[x+1]-12)
              pitches[x] += 12;
          }
          //if (pitches[x]!=-1 && pitches[x]<=pitches[x+1]-24) 
            //pitches[x] += 12;
        }
        
    }

    void debug_inversions() {
      for (int i = 0 ; i < 12 ; i++) {
        int pitches[] = { 60, 64, 67, -1, -1, -1, -1, -1, -1, -1 };
        int lowestpitch = pitches[0];
        /*for (int x = 0 ; x < 3 ; x++) {
          if (bitRead(i, x)) {
            // mutate pitch x
            //replace_pitch(pitches, pitches[x], 
            //  (pitches[x]>lowestpitch+12) ? pitches[x]-12 : pitches[x]+12
            //);
            pitches[x] += 12;
            //if (x==2 && pitches[x]>lowestpitch+12) {
            //  pitches[x] -= 24;
            //} 
            //else 
            if (x>0 && !bitRead(i,x-1) && pitches[x]>lowestpitch+12) { //pitches[x]>pitches[x-1]+12) { 
              pitches[x] -= 24;
            }
          }
        }
        */
        /*for (int x = i ; x > 0 ; x--) {
          pitches[x%3] -= 12;
          if (pitches[x%3]<lowestpitch-12)
          //if (pitches[x%3]>pitches[x-1%3]+12)
            pitches[x%3] += 12;
        }*/
        /*pitches[i%3] -= 12;
        for (int x = 0 ; x < i/3 ; x++) {
          pitches[(i+3-x)%3] -= 12;
        }

        // perhaps we collapse the pitches after calculating the above - ie loop from top to bottom, and if el is more than an octave above el-1, we el-=12 ?
        
        sort_pitches(pitches, 10);

        for (int x = 0 ; x < 10 ; x++) {
          if (x!=0 && (x==9 || pitches[x+1]==-1)) {
            if (pitches[x] > pitches[x-1]+12)
              pitches[x] -= 12;
          } else {
            if (pitches[x] < pitches[x+1]-12)
              pitches[x] += 12;
          }
          //if (pitches[x]!=-1 && pitches[x]<=pitches[x+1]-24) 
            //pitches[x] += 12;
        }*/
        do_inversion(pitches, i);
        
        Serial.printf("Inversion %i: [ ", i);
        for (int d = 0 ; d < 10 ; d++) {
          //if (pitches[d]>-1) 
            Serial.printf("%i ", pitches[d]);
        }
        Serial.print("] [ ");
        for (int d = 0 ; d < 10 ; d++) {
          //if (pitches[d]>-1) 
            Serial.printf("%s ", get_note_name(pitches[d]).c_str());
        }
        Serial.println("]");
        //Serial.println("----");
      }
    }

    // handle CCs for harmony options
    bool handle_ccs(int channel, int number, int value) {
      if (channel!=GM_CHANNEL_DRUMS) return false;
    
      if (number==CC_HARMONY_MUTATE_MODE) {
        mutation_mode = value % HARMONY::MUTATION_MODE::MUTATION_MODE_MAX;
        Serial.printf("Setting harmony mutation_mode to %i\n", mutation_mode);
        return true;
      } else if (number==CC_HARMONY_MELODY_MODE) {
        mko_keys.douse_notes();
        mko_keys.set_melody_mode(value); //melody_mode = value % HARMONY::MELODY_MODE::MELODY_MODE_MAX;
        //melody_mode = value % HARMONY::MELODY_MODE::MELODY_MODE_MAX;
        Serial.printf("Setting melody mode to %i\r\n", value);
        //Serial.printf("Sizeof harmony modes is %i\r\n", sizeof(HARMONY::MELODY_MODE));
        //if (melody_mode==0)
        return true;
      } else if (number==CC_BASS_MELODY_MODE) {
        mko_bass.douse_notes();
        mko_bass.set_melody_mode(value);
        return true;
      } else if (number==CC_PAD_PITCH_MELODY_MODE) {
        mko_pads_pitch.douse_notes();
        mko_pads_pitch.set_melody_mode(value);
        return true;
      } else if (number==CC_PAD_ROOT_MELODY_MODE) {
        mko_pads_root.douse_notes();
        mko_pads_root.set_melody_mode(value);
        return true;
      /*} else if (number==CC_BASS_SET_ARP_MODE) {
        set_arp_mode(value);
        return true;*/
      } else if (number==CC_HARMONY_ONLY_NOTE_HELD) {
        set_only_note_held(value>0);
        return true;
      } else if (number==CC_AUTO_PROGRESSION) {
        auto_progression = value>0;
        return true;
      } else if (number==CC_AUTO_CHORD_TYPE) {
        auto_chord_type = value>0;
        return true;
      } else if (number==CC_AUTO_CHORD_INVERSION) {
        auto_chord_inversion = value>0;
        return true;
      } else if (number==CC_MELODY_OCTAVE_OFFSET) {
        mko_keys.douse_notes();
        mko_keys.set_octave_offset(constrain(value-2, -2, 3));
        return true;
      } else if (number==CC_PAD_PITCH_OCTAVE_OFFSET) {
        mko_pads_pitch.douse_notes();
        mko_pads_pitch.set_octave_offset(constrain(value-2, -2, 3));
        return true;
      } else if (number==CC_PAD_ROOT_OCTAVE_OFFSET) {
        mko_pads_root.douse_notes();
        mko_pads_root.set_octave_offset(constrain(value-2, -2, 3));
        return true;
      } else if (number==CC_MELODY_SCALE) {
        //mko_keys.set_octave_offset(constrain(value-3, -3, 3));
        scale_number = value % NUM_SCALES;
        Serial.printf("set scale to %i\n", scale_number);
        douse_all(); //douse_melody();
        return true;
      } else if (number==CC_MELODY_AUTO_SCALE) {
        //mko_keys.set_octave_offset(constrain(value-3, -3, 3));
        auto_scale = value>0;
        return true;
      } else if (number==CC_CHANNEL_BASS_OUT) {
        mko_bass.set_midi_channel(value);
        return true;
      } else if (number==CC_CHANNEL_BITBOX_KEYS) {
        mko_keys.set_midi_channel(value);
        return true;
      } else if (number==CC_CHANNEL_PAD_ROOT) {
        mko_pads_root.set_midi_channel(value);
        return true;
      } else if (number==CC_CHANNEL_PAD_PITCH) {
        //Serial.println(">>> received CC_CHANNEL_PAD_PITCH ");
        mko_pads_pitch.set_midi_channel(value);
        return true;
      } else if (number==CC_MELODY_ROOT) {
        if (channel_state.set_midi_root_pitch(value)) // sets but returns false if no change
          kill_notes();
        return true;
      } else if (number==CC_BASS_SET_TIE_ON) {
        //Serial.printf("### TIE: Setting tie_on for pattern %i to %i\r\n", PATTERN_BASS, value);
        patterns[PATTERN_BASS].tie_on = value;
        return true;
      }

      return false;
    }    

    // only used by MIDI_CHANNEL_* defines - todo: refactor
    int get_midi_channel_bass_out() {
      return mko_bass.channel;
    }
    int get_midi_channel_bitbox_keys() {
      return mko_keys.channel;
    }
    int get_midi_channel_pads_pitch() {
      return mko_pads_pitch.channel;
    }
    int get_midi_channel_pads_root() {
      return mko_pads_root.channel;
    }
    
    // enable/disable restricting harmony to only playing when notes are held on input
    void set_only_note_held(int value) {
      only_note_held = value; //= mode % ARP_MODE_MAX;
      // todo: kill notes that otherwise won't end when this mode changes
    }

    // get strings about the current scale/sequence/chords settings
    char *get_bass_info() {
      static char output[20];
    
      sprintf(output, "s%i %i %i:%3s", scale_number, sequence_number, chord_number, get_note_name(get_current_chord_root_pitch()).c_str());
      return output;
    }

    /*char *get_bass_info_2() {
      static char output[32];
    
      // auto scale, auto arp, auto progression, bass counter
      sprintf(output, "as aa ap bc\n%c  %c  %c  %2i\n", bass_auto_scale ? 'Y' : 'N', bass_auto_arp ? 'Y' : 'N', bass_auto_progression ? 'Y' : 'N', bass_counter);
      return output;
    }*/
    
};

// for use globally
Harmony harmony = Harmony(autobass_input);

#endif

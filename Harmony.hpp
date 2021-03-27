#ifndef HARMONY_INCLUDED
#define HARMONY_INCLUDED

#define DEBUG_HARMONY false

#define DEFAULT_MELODY_OFFSET   1

#define DEFAULT_AUTO_PROGRESSION_ENABLED  true   // automatically play chords in progression order?
#define DEFAULT_BASS_ONLY_WHEN_NOTE_HELD  false  // 
#define DEFAULT_SCALE                     0      // 0 = major, 1 = minor...
#define DEFAULT_AUTO_SCALE_ENABLED        true


// CONFIGURATION: messages targeted to channel _IN will be relayed on channel _OUT -- for passing through messages to Neutron (TODO: probably move this to a dedicated config file)
#define MIDI_CHANNEL_BASS_IN        8     // channel to receive direct bass playing
#define MIDI_CHANNEL_BASS_AUTO_IN   9     // channel to receive automatic bass notes
#define MIDI_CHANNEL_MELODY_IN      3     // channel to receive direct melody/chords
#define DEFAULT_MIDI_CHANNEL_BASS_OUT       2     // channel to output bass notes on
#define DEFAULT_MIDI_CHANNEL_BITBOX_KEYS 3 // bass output, but shifted an octave

#define MIDI_CHANNEL_BASS_OUT     (harmony.get_midi_channel_bass_out())
#define MIDI_CHANNEL_BITBOX_KEYS  (harmony.get_midi_channel_bitbox_keys())

//BITBOX/melody settings
#define BITBOX_NOTE_MINIMUM     36  // https://1010music.com/wp-content/uploads/2020/08/bitbox-mk2-1.0.8-user-manual.pdf "MIDI inputs for notes 36 to 51 map to the pads", "EXT1 through EXT4 are assigned notes 55 to 52 for use as Recording triggers"
#define BITBOX_KEYS_OCTAVE_OFFSET 2

#define CC_CHANNEL_BASS_OUT 12
#define CC_CHANNEL_BITBOX_KEYS 13
#define CC_MELODY_ROOT 11

#define CC_BASS_SET_ARP_MODE        17    // cc to set the bass arp mode
#define CC_BASS_ONLY_NOTE_HELD      18    // cc to set bass to only play in external mode if note is held

#define ARP_MODE_NONE         0
#define ARP_MODE_PER_BEAT     1
#define ARP_MODE_NEXT_ON_NOTE 2
#define ARP_MODE_MAX          3
#define DEFAULT_ARP_MODE          ARP_MODE_NONE   // choose notes to play from the current sequence (eg incrementing through them)?

//#define BASS_DEBUG
// handling debugging output - pattern from https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c/1644898#1644898
#ifdef HARM_DEBUG
#define HARM_DEBUG 1
#else
#define HARM_DEBUG 0
#endif
#define HARM_printf(fmt, ...)   do { if (HARM_DEBUG) Serial.printf((fmt), ##__VA_ARGS__); } while (0)
#define HARM_println(fmt, ...)  do { if (HARM_DEBUG) Serial.println((fmt), ##__VA_ARGS__); } while (0)
//debug handling

//#include "bass.hpp"       // for access to the bass channel info  -- to be deprecated ? 
#include "HarmonyOutput.hpp"
#include "BPM.hpp"
#include "ChannelState.hpp"

ChannelState autobass_input = ChannelState();   // tracking notes that are held

#define CC_HARMONY_MELODY_MODE  29
#define CC_HARMONY_MUTATE_MODE  30

#define CC_AUTO_PROGRESSION   31
#define CC_AUTO_CHORD_TYPE    105
#define CC_AUTO_CHORD_INVERSION    106
#define CC_MELODY_OCTAVE_OFFSET   107
#define CC_MELODY_SCALE   108
#define CC_MELODY_AUTO_SCALE   109

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




// qsort requires you to create a sort function
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
  // A simpler, probably faster way:
  //return b - a;
}


void sort_pitches(int pitches[], int len) {
  qsort(pitches, len, sizeof(pitches[0]), sort_pitch);
}


// track the current harmony state and send MIDI as appropriate when triggered
class Harmony {

  private:
    int last_root = -1;
    int last_melody_root = -1;
    int last_melody_pitches[10];
    ChannelState&   channel_state;
    MidiKeysOutput& mko_bass;
    MidiKeysOutput& mko_keys;

    int scale_number = DEFAULT_SCALE;               // index of the current scale we're in
    int chord_number = 0;               // index of the current chord degree that we're playing (0-6, well actually can be negative or go beyond that to access lower&higher octaves)
    int sequence_number = 0;            // index of the arp sequence that we're currently playing
    int sequence_counter = 0;               // track current position in arp sequence

    int arp_counter = 0;  // todo, probably move this into the MidiKeysOutput..?
    
    int mutation_mode = HARMONY::MUTATION_MODE::RANDOMISE;
    int melody_mode   = HARMONY::MELODY_MODE::CHORD;
    int arp_mode      = DEFAULT_ARP_MODE; //ARP_MODE_NEXT_ON_NOTE;

    //bool auto_arp         = DEFAULT_AUTO_ARP_ENABLED;   // choose notes to play from the current sequence (eg incrementing through them)
    bool auto_progression = DEFAULT_AUTO_PROGRESSION_ENABLED;   // automatically play chords in progression order
    bool auto_scale       = DEFAULT_AUTO_SCALE_ENABLED;   // automatically switch scales every phrase
    bool auto_chord_type  = true;
    bool auto_chord_inversion = true;
    bool only_note_held   = DEFAULT_BASS_ONLY_WHEN_NOTE_HELD;

    int default_chord_progression[4]    =   { 
      0, 5, 1, 4 
    };     // chord progression
    int default_sequence[4][4]     =   { // degrees of scale to play per chord -- ie, arp patterns
      { 0, 0, 0, 0 },
      { 0, 0, 0, 1 },
      { 0, 1, 1, 2 },
      { 0, 1, 0, 4 },
      //{ 0, 2, 4, 6 },
      /*{ 0, 2, 0, 4 },
        { 0, 3, 6, 4 }*/
    };
    int chord_progression[4];
    int sequence[4][4];

  public:  
    Harmony(ChannelState& channel_state_, MidiKeysOutput& mko_bass_, MidiKeysOutput& mko_keys_): channel_state(channel_state_), mko_bass(mko_bass_), mko_keys(mko_keys_)
    { // todo: pass in config settings
      channel_state = channel_state_;
      mko_bass = mko_bass_;
      mko_keys = mko_keys_;

      reset_progression();
      reset_sequence_pattern();

      //debug_inversions();
    }

    void reset_progression(int source[4]) {
      for (int i = 0 ; i < CHORD_PROGRESSION_LENGTH ; i++) {
        chord_progression[i] = source[i];
      }
    }
    void reset_progression() { //&default_chord_progression) {
      //chord_progression[4]    =   { 0, 5, 1, 4 };
      reset_progression(default_chord_progression);
    }

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

    // MELODY
    void fire_melody () {
      // send notes to melody appropriate for current status
      //  to include options for playing root, full chords or arpeggiation

      //if (!bpm_internal_mode && bass_only_note_held && !autobass_input.is_note_held())
      //return;
      
      if (only_note_held && !channel_state.is_note_held()) {
        return;
      }

      // find the pitch to play
      int pitch = MIDI_BASS_ROOT_PITCH;
      pitch = channel_state.get_root_note();

      // get the current chord number
      if (!channel_state.is_note_held()) {
        //pitch = channel_state.get_root_note() + get_scale_note(get_chord_number());
        //pitch = bass_get_sequence_pitch(bass_counter);
        pitch = channel_state.get_root_note() + get_scale_note(
                        arp_mode>0 ? sequence[ get_sequence_number() ][ sequence_counter%HARM_SEQUENCE_LENGTH ] : 0, 
                        get_chord_number());
        /*if (arp_mode>0) { // select from the current chord
          pitch = get_notes_for_chord(get_chord_number()) [ sequence_counter%HARM_SEQUENCE_LENGTH ];
        } else {
          pitch = channel_state.get_root_note() + get_scale_note(get_chord_number());
        }*/
      }
    
      last_melody_root = pitch;

      if (DEBUG_HARMONY) 
        Serial.printf("beat %i: harmony.fire_melody()  told to fire at root pitch %s? [%i]\r\n", current_beat, get_note_name(pitch).c_str(), pitch);

      if (melody_mode==HARMONY::MELODY_MODE::MELODY_MODE_NONE) {
        // do nothing
      }/* else if (melody_mode==HARMONY::MELODY_MODE::ARPEGGIATE) {
        //if (DEBUG_HARMONY)
        pitch = channel_state.get_root_note() + get_scale_note(sequence[sequence_number][sequence_counter%HARM_SEQUENCE_LENGTH], get_chord_number());
        last_melody_pitches[0] = pitch;

        Serial.printf("    using arpeggiate mode - sending pitch %s [%i]\r\n", get_note_name(pitch).c_str(), pitch);
        mko_keys.send_note_on(pitch, 127);    // send a single pitch
      } */ else if (melody_mode==HARMONY::MELODY_MODE::SINGLE) {
        pitch = channel_state.get_root_note() 
                + get_scale_note(
                        arp_mode>0 ? sequence[ get_sequence_number() ][ sequence_counter%HARM_SEQUENCE_LENGTH ] : 0, 
                        get_chord_number()
                );// + get_scale_note(0, get_chord_number());
        last_melody_pitches[0] = pitch;

        if (DEBUG_HARMONY) Serial.printf("    using single mode - sending pitch %s [%i]\r\n", get_note_name(pitch).c_str(), pitch);
        mko_keys.send_note_on(pitch, 127);    // send a single pitch
      } else if (melody_mode==HARMONY::MELODY_MODE::CHORD || HARMONY::MELODY_MODE::ARPEGGIATE) {
        if (DEBUG_HARMONY) Serial.println("    using chord mode for melody!");
        
        //memcpy(&last_melody_pitches, get_chord_for_pitch(pitch), 10 * sizeof(int));
        //Serial.printf("about to play chord is %i\r\n", );

        if (channel_state.is_note_held()) {
          memcpy(&last_melody_pitches, channel_state.get_held_notes(), 10*sizeof(int));
          for (int i = 0 ; i < get_chord_inversion() ; i++) {
            if (last_melody_pitches[i]>-1) last_melody_pitches[i] += 12;
          }
          mko_keys.send_note_on(last_melody_pitches, 127);  // send all held notes
        } else {
          if (DEBUG_HARMONY) Serial.printf("   phrase is %i, beat %i, scale num %i: ", current_phrase, current_beat, get_scale_number()); 
          if (DEBUG_HARMONY) Serial.printf("   about to play chord number %i(+%i sequence offset)\r\n", get_chord_number(), sequence[get_sequence_number()][sequence_counter]);

          //int sequence_offset = 0; //sequence[current_phrase%4][current_beat];
          int chord_number = get_chord_number();
          int chord_type = get_chord_type();
          int inversion = get_chord_inversion();
          //int chord_type = 0;
          //int inversion = 0;          

          // todo: move get_chord_number here (all it does is test mode we're in and return chord_progression number or 0?
          // todo: move arping logic here (currently hardcoded to arp thru sequence)
          memcpy(&last_melody_pitches, 
            get_notes_for_chord(
              arp_mode>0 ?
                sequence[get_sequence_number()%NUM_SEQUENCES][sequence_counter%HARM_SEQUENCE_LENGTH] + get_chord_number() :
                get_chord_number(),
              chord_type,
              inversion
            ), 
            10 * sizeof(int)
          );

          if (melody_mode==HARMONY::MELODY_MODE::ARPEGGIATE) {
            if (last_melody_pitches[arp_counter]==-1) {
              arp_counter = 0;
            }
            last_melody_pitches[0] = last_melody_pitches[arp_counter];
            for (int i = 1 ; i < 10 ; i++) {
              last_melody_pitches[i] = -1;
            }
            mko_keys.send_note_on(last_melody_pitches[0], 127);
            
          } else {
            mko_keys.send_note_on(last_melody_pitches, 127);  // send all notes of generated chord
          }
        }

        // debug melody turn on notes
        if (DEBUG_HARMONY) {
          Serial.printf("   storing last_melody_pitches as: [");
          for (int i = 0 ; i < 10 ; i++) {
            if (last_melody_pitches[i]>-1) Serial.printf("%i, ", last_melody_pitches[i]);
          }
          Serial.println("]");
        }
        
        //mko_keys.send_note_on(get_chord_for_pitch(pitch), 127);
        //mko_keys.send_note_on(last_melody_pitches, 127);
      } /*else if (melody_mode==HARMONY::MELODY_MODE::ARPEGGIATE) {
        // TODO
      }*/
    }
    
    void douse_melody () {
      // send notes to douse playing melody
      int pitch = last_melody_root;
      //mko_keys.send_note_off(pitch);
      
      // debug melody turn on notes
      if (DEBUG_HARMONY) {
        Serial.printf("recalling last_melody_pitches as: [");
        for (int i = 0 ; i < 10 ; i++) {
          if (last_melody_pitches[i]>-1) Serial.printf("%i, ", last_melody_pitches[i]);
        }
        Serial.println("]");
      }
      
      if (melody_mode==HARMONY::MELODY_MODE::CHORD) {
        mko_keys.send_note_off(last_melody_pitches, 127);
      } else /* if (melody_mode==HARMONY::MELODY_MODE::SINGLE || melody_mode==HARMONY::MELODY_MODE::MELODY_MODE_NONE ) */{
        //if (channel_state.is_note_held()) {
          //mko_keys.send_note_off(channel_state.get_held_notes(), 0);
          mko_keys.send_note_off(last_melody_pitches, 0);

          if (melody_mode==HARMONY::MELODY_MODE::ARPEGGIATE) {
            arp_counter++;
            if (arp_counter>=10) arp_counter = 0;
          }
          
        /*} else {
          if (pitch>-1)
            mko_keys.send_note_off(pitch, 127);
        }*/
      } 
      for (int i = 0 ; i < 10 ; i++)  // clear last played memory
        last_melody_pitches[i] = -1;

      last_melody_root = -1;
    }

    // BASS
    void fire_bass () {
      // send notes to bass appropriate for current status
      
      if (only_note_held && !channel_state.is_note_held()) {
        return;
      }


      // todo: make this actually work again.. 
      if (arp_mode==ARP_MODE_PER_BEAT)
        sequence_counter = current_beat;
      else if (arp_mode==ARP_MODE_NONE)// dont arp at all
        sequence_counter = 0;
        
      int pitch = MIDI_BASS_ROOT_PITCH;       // TODO: adjust pitch if required by the mode
      pitch = channel_state.get_root_note();
      //pitch = channel_state.get_root_note() + bass_get_scale_note(get_chord_number());

      // todo: adjust if arping / progressioning / etc

      // get the current chord number
      if (!channel_state.is_note_held()) {
        //pitch = channel_state.get_root_note() + bass_get_scale_note();
        // TODO: obey arp mode and adjust note to send accordingly
        //if (DEBUG_HARMONY) Serial.printf("fire_bass > current beat is %i: sequence_number is %i, counter is %i, ", current_beat, get_sequence_number(), sequence_counter);
        //if (DEBUG_HARMONY) Serial.printf("   chord number is %i, sequence result is %i, ", get_chord_number(), sequence[get_sequence_number()%NUM_SEQUENCES][sequence_counter%HARM_SEQUENCE_LENGTH]);
        
        pitch = channel_state.get_root_note() + 
                    get_scale_note(
                        arp_mode>0 ? sequence[ get_sequence_number() ][ sequence_counter%HARM_SEQUENCE_LENGTH ] : 0, 
                        get_chord_number()
                    );
        /*if (arp_mode>0) { // select from the current chord
          pitch = get_notes_for_chord(get_chord_number()) [ sequence_counter%HARM_SEQUENCE_LENGTH ];
        } else {
          pitch = channel_state.get_root_note() + get_scale_note(get_chord_number());
        }*/
        
        if (DEBUG_HARMONY) Serial.printf(" fire_bass : root %i : seq %i : seq_cnt %i : chordnum %i, ", channel_state.get_root_note(), get_sequence_number(), sequence_counter, get_chord_number()  );
        if (DEBUG_HARMONY) Serial.printf("  : scalenote %i : finds pitch %i\r\n", get_scale_note(sequence[get_sequence_number()][sequence_counter%HARM_SEQUENCE_LENGTH]), pitch);
        //if (DEBUG_HARMONY) Serial.printf("   resulting pitch is %i aka %s\r\n", pitch, get_note_name(pitch).c_str());
        //pitch = bass_get_sequence_pitch(bass_counter);
      } else {
        pitch = channel_state.get_sequence_held_note(sequence_counter);
      }
      if (DEBUG_HARMONY) 
        Serial.printf("beat %i: harmony.fire_bass()  told to fire pitch %s [%i]\r\n", current_beat, get_note_name(pitch).c_str(), pitch);

      //Serial.printf("mko_bass channel is %i\r\n", mko_bass.channel);

      if (pitch>-1) {
        last_root = pitch;
        mko_bass.send_note_on(pitch, 127);
      }
    }

    void douse_bass () {
      // send notes to douse bass
      //bass_note_off();
      int pitch = last_root;
      if (pitch>-1)
        mko_bass.send_note_off(pitch);
      last_root = -1;

      if (pitch>-1) {
        //last_root = pitch;
        //mko_bass.send_note_on(pitch, 127);
        if (arp_mode==ARP_MODE_NEXT_ON_NOTE)
          sequence_counter++;
      }       
    }

    // todo: checking/configure which should fire
    void fire_both () {
      if (DEBUG_HARMONY) Serial.println("fire_both>>>>>");
      fire_bass();
      fire_melody();
      if (DEBUG_HARMONY) Serial.println("fire_both<<<<<");
    }
    void douse_both () {
      if (DEBUG_HARMONY) Serial.println("douse_both>>>>>");
      douse_bass();
      douse_melody();
      if (DEBUG_HARMONY) Serial.println("douse_both<<<<<");
    }

    void kill_notes() {
        Serial.println("harmony.kill_notes() called!");
        mko_bass.send_all_notes_off();
        mko_keys.send_all_notes_off();
    }

    void mutate() {
      if (mutation_mode==HARMONY::MUTATION_MODE::RANDOMISE) {
        Serial.println("## HARMONY MUTATE - randomise!");
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
    }

//// chord choosing, note choosing, etc

    int get_sequence_note(int position = 0) {
      if (position % HARM_SEQUENCE_LENGTH == 0) HARM_printf("----- at bass arp sequence start (position %i)\r\n", position);
      int sequence_number = get_sequence_number();
    
      return get_scale_note(
        sequence [sequence_number] [position % HARM_SEQUENCE_LENGTH]
      );
    }

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
      HARM_printf("final oct:sd is %i:%i", oct, sd);
   
      int r = scale_offset[scale_number][sd % SCALE_SIZE]
              +
              (oct * 12)
              ;
    
      HARM_printf(" -- final note is %i\r\n", r);
//#define HARM_DEBUG 0
      return r;
    }

    // get the currently active arp sequence index
    int get_sequence_number() {
      if (arp_mode>0) { //auto_arp) {
        // choose the arp sequence automatically based on the current song position
        sequence_number = current_bar % NUM_SEQUENCES;
      } else {
        sequence_number = 0;
      }
      
      return sequence_number;
    }

    // get the currently active chord in the scale
    int get_chord_number() {
      if (auto_progression) {
        // automatically change chord based on our current song bar position
        int c = chord_progression[current_bar % CHORD_PROGRESSION_LENGTH] % SCALE_SIZE;
        HARM_printf("get_chord_number returning %i for chord progression on bar %i!\r\n", c, current_bar%CHORD_PROGRESSION_LENGTH);
        return c; // todo: make this select from lowest held note?
      }
      
      return 0; // default to root (default or whatever is held)
    }

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
          Serial.printf("\tinsert_pitch: adding pitch %s [%i] at position %i\r\n", get_note_name(pitch).c_str(), pitch, i);
          pitches[i] = pitch;
          break;
        } else if (pitches[i]>pitch) {
          // higher found, insert at i 
          Serial.printf("\tinsert_pitch: inserting pitch %s [%i] at position %i\r\n", get_note_name(pitch).c_str(), pitch, i);
          //for (int x = i+1 ; x < 10 ; x++) {
          for (int x = 8 ; x > i+1 ; x--) {
            Serial.printf("\t\tshifting pitch at %i to %i (%i overwrites %i)\r\n", x, x+1, pitches[x-1], pitches[x]);
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
          Serial.printf("\tremove_pitch: removing pitch %s [%i] from position %i\r\n", get_note_name(pitch).c_str(), pitch, i);
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

    int get_chord_type() {
      int type = 0;
      if (auto_chord_type) {
        type = HARMONY::CHORD_TYPE::TRIAD;
        if (current_bar%2==1) {
          type = HARMONY::CHORD_TYPE::SEVENTH;
          if (current_phrase%2==0)
            type = HARMONY::CHORD_TYPE::NINETH;
        }
        if (current_bar==3) { 
          type += (current_phrase+current_beat)%HARMONY::CHORD_TYPE::CHORD_TYPE_MAX;
        }
        type %= HARMONY::CHORD_TYPE::CHORD_TYPE_MAX;   
      } else
        type = HARMONY::CHORD_TYPE::TRIAD;
        
      return type;
    }

    int get_chord_inversion() {
      if (auto_chord_inversion) 
        return (current_bar%2==0 && current_beat+1%2==0) || current_bar==BARS_PER_PHRASE-1 
          ? current_beat : 0;
      else
        return 0;
    }

    // return pointer to array
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
        Serial.println("!!! seventh chord?");
        //pitches[p++] = /*current_bar==BARS_PER_PHRASE-1 ? */
        insert_pitch(pitches, 
                     channel_state.get_root_note() + get_scale_note(6, chord));
                     //: -1; // add SEVENTH 
        p++;
      }
      if (chord_type==HARMONY::CHORD_TYPE::EIGHTH) {
        Serial.println("!!! eighth chord?");

        //pitches[p++] = /*current_bar==BARS_PER_PHRASE-1 ? */
        insert_pitch(pitches, 
                   channel_state.get_root_note() + get_scale_note(7, chord));
                   //: -1; // add NINTH
                   p++;
      }
      if (chord_type==HARMONY::CHORD_TYPE::NINETH) {
        Serial.println("!!! ninth chord?");

        //pitches[p++] = /*current_bar==BARS_PER_PHRASE-1 ? */
        insert_pitch(pitches, 
                   channel_state.get_root_note() + get_scale_note(8, chord));
                   //: -1; // add NINTH
                   p++;
      }
      if (chord_type==HARMONY::CHORD_TYPE::TENTH) {
        Serial.println("!!! tenth chord?");

        //pitches[p++] = /*current_bar==BARS_PER_PHRASE-1 ? */
        insert_pitch(pitches, 
                   channel_state.get_root_note() + get_scale_note(9, chord));
                   //: -1; // add NINTH
                   p++;
      }
      if (chord_type==HARMONY::CHORD_TYPE::ELEVENTH) {
        Serial.println("!!! eleventh chord?");
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
        Serial.println("!!! thirteenth chord?");
        insert_pitch(pitches, 
        //pitches[p++] = //current_bar==BARS_PER_PHRASE-1 ? 
                   channel_state.get_root_note() + get_scale_note(12, chord) );
                   p++;
      }

      // do inversions
      /*for (int i = 0 ; i < inversion ; i++) {
        if (pitches[i]>-1) {
          //pitches[i] += 12;
          int p = pitches[i];
          remove_pitch(pitches, p);
          insert_pitch(pitches, p + 12);
        }
      }*/
      //pitches[3] = pitch + scale_offset[scale_number][8]; // add 9th chord
      //-1, -1, -1, -1, -1, -1, -1
      for (int i = p ; i < 10 ; i++) {
        pitches[i] = -1;
      }

      do_inversion(pitches, inversion);
      
      Serial.printf("get_chord_notes for scale %i : chord %i : inversion %i : chord_type %i : [ ", get_scale_number(), chord, inversion, chord_type);
      for (int i = 0 ; i < 10 ; i++) {
        if (pitches[i]>-1) Serial.printf("%s ", get_note_name(pitches[i]).c_str());
      }
      Serial.println("] ");
      
      return pitches;
    }

    int get_currently_playing_root() {
      return last_melody_root;
    }

    void reset_sequence() {
      sequence_counter = 0;
    }

    /*int inversions[][] = {
      { 0, 1, 2 },
    }*/

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


    bool handle_ccs(int channel, int number, int value) {
      if (channel!=GM_CHANNEL_DRUMS) return false;
    
      if (number==CC_HARMONY_MUTATE_MODE) {
        mutation_mode = value % HARMONY::MUTATION_MODE::MUTATION_MODE_MAX;
        Serial.printf("Setting harmony mutation_mode to %i\n", mutation_mode);
        return true;
      } else if (number==CC_HARMONY_MELODY_MODE) {
        douse_melody();
        melody_mode = value % HARMONY::MELODY_MODE::MELODY_MODE_MAX;
        Serial.printf("Setting melody mode to %i\r\n", melody_mode);
        //Serial.printf("Sizeof harmony modes is %i\r\n", sizeof(HARMONY::MELODY_MODE));
        //if (melody_mode==0)
        return true;
      } else if (number==CC_BASS_SET_ARP_MODE) {
        set_arp_mode(value);
        return true;
      } else if (number==CC_BASS_ONLY_NOTE_HELD) {
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
        mko_keys.set_octave_offset(constrain(value-2, -2, 3));
        douse_melody();
        return true;
      } else if (number==CC_MELODY_SCALE) {
        //mko_keys.set_octave_offset(constrain(value-3, -3, 3));
        scale_number = value % NUM_SCALES;
        Serial.printf("set scale to %i", scale_number);
        douse_melody();
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
      } else if (number==CC_MELODY_ROOT) {
        if (channel_state.set_midi_root_pitch(value)) // sets but returns false if no change
          kill_notes();
        return true;
      }

      return false;
    }    

    int get_midi_channel_bass_out() {
      return mko_bass.channel;
    }
    int get_midi_channel_bitbox_keys() {
      return mko_keys.channel;
    }
    
    void set_arp_mode(int mode) {
      arp_mode = mode % ARP_MODE_MAX;
      // todo: kill notes that otherwise won't end when this mode changes
    }
    
    void set_only_note_held(int value) {
      only_note_held = value; //= mode % ARP_MODE_MAX;
      // todo: kill notes that otherwise won't end when this mode changes
    }


    // get strings about the current scale/sequence/chords settings
    char *get_bass_info() {
      static char output[20];
    
      sprintf(output, "s%i %i %i:%3s", scale_number, sequence_number, chord_number, get_note_name(get_currently_playing_root()).c_str());
      return output;
    }

    /*char *get_bass_info_2() {
      static char output[32];
    
      // auto scale, auto arp, auto progression, bass counter
      sprintf(output, "as aa ap bc\n%c  %c  %c  %2i\n", bass_auto_scale ? 'Y' : 'N', bass_auto_arp ? 'Y' : 'N', bass_auto_progression ? 'Y' : 'N', bass_counter);
      return output;
    }*/
    
};


MidiKeysOutput mkob = MidiKeysOutput(DEFAULT_MIDI_CHANNEL_BASS_OUT);
MidiKeysOutput mkok = MidiKeysOutput(DEFAULT_MIDI_CHANNEL_BITBOX_KEYS, DEFAULT_MELODY_OFFSET);  // with octave offset

// for use globally

Harmony harmony = Harmony(
  autobass_input, 
  mkob,
  mkok
);

#endif

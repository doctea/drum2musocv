// based on example code/pseudocode from https://www.computermusicdesign.com/simplest-euclidean-rhythm-algorithm-explained/

#include "Euclidian.h"
#include "MidiOutput.hpp" // because we need to send MIDI
#include "BPM.hpp"  // cos we need to know lengths

//#define EUC_DEBUG

#ifdef EUC_DEBUG
#define EUC_DEBUG 1
#else
#define EUC_DEBUG 0
#endif

// https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c/1644898#1644898
#define EUC_printf(fmt, ...) do { if (EUC_DEBUG) Serial.printf((fmt), ##__VA_ARGS__); } while (0)
#define EUC_println(fmt, ...) do { if (EUC_DEBUG) Serial.println((fmt), ##__VA_ARGS__); } while (0)


bpm_status bpm_statuses[NUM_PATTERNS];

void make_euclid(pattern_t *p, int steps = 0, int pulses = 0, int rotation = -1, int duration = -1, int trigger = -1, int tie_on = -1) {
  // fill pattern_t according to parameters

  if (trigger>=0)   p->trigger = trigger;
  if (tie_on>=0)    p->tie_on = tie_on;

  if (steps > 0)    p->steps = steps;
  if (pulses > 0)   p->pulses = pulses;
  if (rotation >= 0) p->rotation = rotation;
  if (duration >= 0) p->duration = duration;

  EUC_printf("in make_euclid (steps = %2i, pulses = %2i, rotation = %2i, duration = %i)\r\n", p->steps, p->pulses, p->rotation, p->duration);

  int bucket = 0;
  for (int i = 0 ; i < p->steps ; i++) {
    bucket += p->pulses;
    if (bucket >= p->steps) {
      bucket -= p->steps;
      p->stored[i] = true;
    } else {
      p->stored[i] = false;
    }
  }
  p->original_steps = p->steps;

  if (p->rotation > 0) {
    rotate_pattern(p, p->rotation);
  }

}

// should note be played this step?
bool query_pattern(pattern_t *p, int step, int offset = 0, int bar = 0) {
  step += bar * STEPS_PER_BAR;
  int curStep = (step + offset) % p->steps; //wraps beat around if it is higher than the number of steps
  if (curStep < 0) curStep = (p->steps) + curStep; // wrap around if result passes sequence boundary
  //EUC_printf("\r\nquery_pattern querying step %i\r\n", curStep);
  return p->stored[curStep];
}

// find out if note should be killed this step
bool query_pattern_note_off(pattern_t *p, int step, int bar = 0) { //, int offset = -2) {
  step += bar * STEPS_PER_BAR;
  //EUC_printf("\r\nnote_off querying beat %i with duration-based offset %i\r\n", beat, (int)p->duration*-1);
  // TODO: so this looks "back in time" in the pattern to see if a note was played, but i think there is a better way of doing this somehow
  // should we cut before retriggering?
  return query_pattern(p, step, (int)p->duration * -1);
}

// rotate the pattern around specifed number of steps -- could actually not change the pattern and just use the rotation in addition to offset in the query_patterns
void rotate_pattern(pattern_t *p, int rotate) {
  bool stored[p->steps];
  int offset = p->steps - rotate;
  for (int i = 0 ; i < p->steps ; i++) {
    stored[i] = p->stored[abs( (i + offset) % p->steps )];
  }
  memcpy(p->stored, stored, sizeof(stored));
}

// set whether pattern should play
void set_pattern_active_status(int pattern, bool active) {
  patterns[pattern].active_status = active;
  
  if (!active) {
    douse_trigger(patterns[pattern].trigger);
    Serial.printf("set_pattern_active_status dousing trigger %i for pattern %i\r\n", patterns[pattern].trigger, pattern);
  }
}

double randomDouble(double minf, double maxf)
{
  return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);  // use 1ULL<<63 for max double values)
}

// mutate the pattern according to mode
void mutate(int pattern){
  if (euclidian_mutate_mode == EUCLIDIAN_MUTATE_MODE_TOTAL)
    mutate_euclidian_total(pattern);
  else if (euclidian_mutate_mode == EUCLIDIAN_MUTATE_MODE_SUBTLE)
    mutate_euclidian(pattern);
  else if (euclidian_mutate_mode == EUCLIDIAN_MUTATE_MODE_ACIDBANGER)
    mutate_euclidian_acidbanger(pattern);
  else if (euclidian_mutate_mode == EUCLIDIAN_MUTATE_MODE_NONE) {
    // do nothing
  } else if (euclidian_mutate_mode == EUCLIDIAN_MUTATE_MODE_MASKED) {
    mutate_euclidian_masked(pattern);
  } else { //and there's room for more modes too...

  }
}

void mutate_euclidian_masked(int pattern) {
  mutate_euclidian(pattern);
  int mask_by_pattern = 0;
  do { 
    mask_by_pattern = random(NUM_PATTERNS) % NUM_PATTERNS;  // todo: cut groups?
  } while (mask_by_pattern==pattern);
  
  Serial.printf("[EUC] Mutated pattern %i (and masked against pattern %i!)!\r\n", pattern, mask_by_pattern);
  mask_patterns(&patterns[pattern], &patterns[mask_by_pattern]);
}

void mutate_euclidian_acidbanger(int pattern) {
  // based on code from https://github.com/vitling/acid-banger/blob/main/src/pattern.ts
  float density = 1.0;
  if (pattern>=NUM_TRIGGERS+NUM_ENVELOPES) {
    // melodic track so use melodic generator                                     
    for (int i = 0 ; i < patterns[pattern].steps ; i++) {
      int chance = 100.0 * (density * (i % 4 == 0 ? 0.6 : (i % 3 == 0 ? 0.5 : (i % 2 == 0 ? 0.3 : 0.1))));
      if (random(0,100) < chance) {
        patterns[pattern].stored[i] = true;
      } else {
        patterns[pattern].stored[i] = false;
      }
    }
  } else {
    // rhythm track so use rhythmic generator
    mutate_euclidian_total(pattern);
  }
}

void mutate_euclidian_total(int pattern) {
  int steps = random(2, SEQUENCE_LENGTH_STEPS);   // limit the other random results to max steps so that we don't frequently saturate
  int pulses = random(1, steps);
  if (pattern >= NUM_TRIGGERS && pattern < NUM_PATTERNS) steps *= 2;
  int rotation = random(1, steps+1);
  int duration = random (0, STEPS_PER_BEAT * 2);

  //void make_euclid(pattern_t *p, int steps = 0, int pulses = 0, int rotation = 0, int duration = 0) {
  make_euclid(&patterns[pattern], steps, pulses, rotation, duration);
}

void mutate_euclidian(int pattern) {
  int r = random(0, 100);
  if (r > 50) {
    if (r > 75) {
      patterns[pattern].pulses += 1;
    } else {
      patterns[pattern].pulses -= 1;
    }
  } else if (r < 25) {
    patterns[pattern].rotation += 1;
  } else if (r > 25) {
    patterns[pattern].pulses *= 2;
  } else {
    patterns[pattern].pulses /= 2;
  }
  if (patterns[pattern].pulses >= patterns[pattern].original_steps || patterns[pattern].pulses <= 0) {
    patterns[pattern].pulses = 1;
  }
  make_euclid(&patterns[pattern]); //, patterns[pattern].steps, patterns[pattern].pulses, patterns[pattern].rotation);
  // TODO: expose mask_enabled as an option you can select
  /*if (mask_enabled && r < random(0, 100)) {
    mask_patterns(&patterns[pattern], &patterns[(pattern - 1) % NUM_PATTERNS]);
    EUC_printf("[EUC] Mutated pattern %i (and masked against pattern %i!)!\r\n", pattern, (pattern - 1) % NUM_PATTERNS);
  } else {
    EUC_printf("[EUC] Mutated pattern %i\r\n", pattern);
  }*/

  //randomise_envelopes();  // todo: enable this
}

void mask_patterns (pattern_t *target, pattern_t *op_pattern) {
  for (int i = 0 ; i < target->steps ; i++ ) {
    target->stored[i] = target->stored[i] && !op_pattern->stored[i % op_pattern->steps];
  }
}

void process_euclidian(int ticks) {

  static int last_processed = 0;
  if (ticks == last_processed) return;

  if (!euclidian_auto_play && bpm_internal_mode) return;    // dont play if not set to auto play and running off internal bpm

  // TODO: configurable mutation frequency
  // start of phrase or middle of phrase
  if (is_bpm_on_beat && is_bpm_on_step && (is_bpm_on_phrase || (is_bpm_on_bar && current_bar == (BARS_PER_PHRASE / 2)))) { //(received_ticks / PPQN) % (SEQUENCE_LENGTH_STEPS / 2) == 0) { // commented part mutates every 2 bars

    // always reset even if not in mutate mode, so that fills work
    if (euclidian_reset_before_mutate && (mutate_enabled || euclidian_fills_enabled)) {
      EUC_println("Resetting euclidian before mutation!");
      initialise_euclidian();
    }
    
    if (euclidian_reset_before_mutate) {
      harmony.reset_progression();
      harmony.reset_sequence();
    }

    bool should_mutate = mutate_enabled;

    if (should_mutate) {
      harmony.mutate();

      unsigned long seed = get_euclidian_seed();
      EUC_printf("Euclidian seed: %i\n", seed);
      randomSeed(seed);

      for (int i = 0 ; i < 3 ; i++) { // TODO: make the number of mutations configurable
        EUC_printf("Picking random mutation pattern between %i (incl) and %i (excl).. ", euclidian_mutate_minimum_pattern % NUM_PATTERNS, constrain(1 + euclidian_mutate_maximum_pattern,0,NUM_PATTERNS));
        int ran = random(euclidian_mutate_minimum_pattern % NUM_PATTERNS, constrain(1 + euclidian_mutate_maximum_pattern,0,NUM_PATTERNS));
        EUC_printf("chose pattern %i\r\n", ran);
        randomSeed(seed + ran);
        mutate(ran);

        //debug_patterns();
      }
    }
  }

  // fills on last bar of phrase if enabled
  if (euclidian_fills_enabled && /*is_bpm_on_phrase &&*/ is_bpm_on_beat && is_bpm_on_step && ((is_bpm_on_bar && current_bar == (BARS_PER_PHRASE - 1)))) { //(received_ticks / PPQN) % (SEQUENCE_LENGTH_STEPS / 2) == 0) { // commented part mutates every 2 bars
    EUC_printf("FILLING at bar %i!\r\n", current_bar);
    randomSeed(current_phrase);
    for (int i = 0 ; i < 3 ; i++) {
      int ran = random(0/*euclidian_mutate_minimum_pattern % NUM_PATTERNS*/, constrain(1 + euclidian_mutate_maximum_pattern,0,NUM_PATTERNS));
      patterns[ran].rotation += 2;
      make_euclid(&patterns[ran]);
    }
    for (int i = 0 ; i < 3 ; i++) {
      int ran = random(euclidian_mutate_minimum_pattern % NUM_PATTERNS, constrain(1 + euclidian_mutate_maximum_pattern,0,NUM_PATTERNS));
      patterns[ran].pulses *= 2;
      if (patterns[ran].pulses > patterns[ran].steps) patterns[ran].pulses /= 8;
      make_euclid(&patterns[ran]);
    }
  }

  // old euclidian trigger loop went here

  harmony.process_ties();

  for (int i = 0 ; i < NUM_PATTERNS ; i++) {
    //EUC_printf("\r\n>>>>>>>>>>>about to query current_step %i\r\n", current_step);
    if (!patterns[i].active_status || patterns[i].trigger==-1) continue;
    
    int stutter = 0; //i-NUM_PATTERNS/2;
    if (euclidian_shuffle_hats) {
      if (i==10||i==9) { // hats
        //if (!is_bpm_on_beat && is_bpm_on_step) { // && current_step%2==0) {
          //stutter = random(-2,2); //1+current_step%STEPS_PER_BEAT;
          //stutter = current_step%STEPS_PER_BEAT;
        //}
        if (!is_bpm_on_beat)
          stutter = -1 + (current_step%3);
          //stutter = -2 + (received_ticks-2%(PPQN/TICKS_PER_STEP/2));  // hyperfast !
          //stutter = (-1*(TICKS_PER_STEP/2)) + (ticks%(TICKS_PER_STEP));  // hyperfast !
          //stutter = -(TICKS_PER_STEP/2) + (ticks%(TICKS_PER_STEP));  
          //stutter = (received_ticks%(TICKS_PER_STEP)) - (TICKS_PER_STEP);
          //stutter = -TICKS_PER_STEP/2 + (received_ticks%(TICKS_PER_STEP));
      }
    }
    if (euclidian_flam_clap) {
      if (i==2) { // clap
        if (!is_bpm_on_beat && current_beat>=2) {
          stutter = +3;
        }
      }
    }
    bpm_status bs = bpm_status();
    bs.update(ticks + stutter);

    // handle duration of 0 as duration of half a step
    bool should_douse = false;
    if (patterns[i].duration==0 && (stutter+ticks)%(TICKS_PER_STEP/2)==0) {
      douse_trigger(patterns[i].trigger, 0, true);
      //should_douse = true;
      //continue
    }
    
    if(bs.is_bpm_on_step) {
      //if (i==10||i==9||i==2) { //stutter!=0) {
      //  Serial.printf("stuttering %i for pattern %i !\r\n", stutter, i);
      //}
      if (query_pattern(&patterns[i], bs.current_step, 0 , bs.current_bar)) {  // step trigger
        //douse_trigger(i, 127, true);
        //Serial.printf(">> for pattern %i, using trigger number %i\r\n", i, patterns[i].trigger);
        fire_trigger(patterns[i].trigger, 127, true);
        if (i < 16) {
          //EUC_printf("%01X", i); // print as hex
        } else {
          //EUC_printf("{%01i}", bass_currently_playing); // for bass note indicator
          //EUC_printf("%3s ", get_note_name(harmony.get_currently_playing_root()).c_str()); // for bass note indicator
        }
        //EUC_printf("%c", 97 + i); // print a...q (65 for uppercase)
        //EUC_printf(" ");
      } else if (/*should_douse ||*/ query_pattern_note_off(&patterns[i], bs.current_step, bs.current_bar)) {  // step kill
        bool tied = patterns[i].tie_on>0 && bs.current_step%patterns[i].tie_on==0;
        if (tied) {
          EUC_printf("==== pattern %i at step %i with tie_on %i, result %c\r\n", i, bs.current_step, patterns[i].tie_on, bs.current_step%patterns[i].tie_on==0?'Y':'N');
          //Serial.printf("   Found tied note for beat %i\r\n", bs.current_beat);
        }
        douse_trigger(patterns[i].trigger, 0, true, tied);
        // TODO: turn off according to some other thing.. eg cut groups?
        if (i == 16) EUC_printf("..."); // add extra dots for bass note indicator
        //EUC_printf(".", i); EUC_printf(" ");
      } else {
        //EUC_printf("  ");
        if (i == 16) EUC_printf("   "); // add extra spaces for bass note indicator
      }
    }

    //delete bs;
    
    //Serial.printf(">>> finished checking pattern %i for ticks %i\r\n", i, ticks);
  }
  
  //EUC_printf("ticks is %i, ticks_per_step/2 is %i, result of mod is %i\n", ticks, TICKS_PER_STEP/2, ticks%TICKS_PER_STEP);
  last_processed = ticks;
}

void initialise_euclidian() {

  EUC_println("initialise_euclidian():");

  EUC_println("resetting all:-");
  const int LEN = SEQUENCE_LENGTH_STEPS;
  for (int i = 0 ; i < NUM_PATTERNS ; i++) {
    make_euclid(&patterns[i], LEN, 0, 1, DEFAULT_DURATION, i); // initialise patterns to default length, zero pulses, default rotation of '1' and default duration of 1 step
    // if we don't do this then this does a bad job of resetting the patterns
  }

  // definition: make_euclid( pattern_t *p,  int steps = 0,  int pulses = 0,   int rotation = 0,   int duration = STEPS_PER_BEAT/4   ) {

  EUC_println("initialising:-");
  //     length, pulses, rotation, duration
  int i = 0;
  make_euclid(&patterns[i++],  LEN,    4, 1,   DEFAULT_DURATION, get_trigger_for_pitch(GM_NOTE_ELECTRIC_BASS_DRUM));    // kick
  make_euclid(&patterns[i++],  LEN,    5, 1,   DEFAULT_DURATION, get_trigger_for_pitch(GM_NOTE_SIDE_STICK));    // stick
  make_euclid(&patterns[i++],  LEN,    2, 5,   DEFAULT_DURATION, get_trigger_for_pitch(GM_NOTE_HAND_CLAP));    // clap
  make_euclid(&patterns[i++],  LEN,    3, 1,   DEFAULT_DURATION, get_trigger_for_pitch(GM_NOTE_ELECTRIC_SNARE));   // snare
  make_euclid(&patterns[i++],  LEN,    3, 3,   DEFAULT_DURATION, get_trigger_for_pitch(GM_NOTE_CRASH_CYMBAL_1));    // crash 1
  make_euclid(&patterns[i++],  LEN,    7, 1,   DEFAULT_DURATION, get_trigger_for_pitch(GM_NOTE_TAMBOURINE));    // tamb
  make_euclid(&patterns[i++],  LEN,    9, 1,   DEFAULT_DURATION, get_trigger_for_pitch(GM_NOTE_HIGH_TOM));    // hi tom!
  make_euclid(&patterns[i++],  LEN/4,  2, 3,   DEFAULT_DURATION, get_trigger_for_pitch(GM_NOTE_LOW_TOM));    // low tom
  make_euclid(&patterns[i++],  LEN/2,  2, 3,   DEFAULT_DURATION, get_trigger_for_pitch(GM_NOTE_PEDAL_HI_HAT));    // pedal hat
  make_euclid(&patterns[i++],  LEN,    4, 3,   DEFAULT_DURATION, get_trigger_for_pitch(GM_NOTE_OPEN_HI_HAT));    // open hat
  make_euclid(&patterns[i++],  LEN,    16, 0,  0,                get_trigger_for_pitch(GM_NOTE_CLOSED_HI_HAT)); //DEFAULT_DURATION);   // closed hat
  make_euclid(&patterns[i++],  LEN*2,  1, 1,   DEFAULT_DURATION, get_trigger_for_pitch(GM_NOTE_CRASH_CYMBAL_2));   // crash 2
  make_euclid(&patterns[i++],  LEN*2,  1, 5,   DEFAULT_DURATION, get_trigger_for_pitch(GM_NOTE_SPLASH_CYMBAL));   // splash
  make_euclid(&patterns[i++],  LEN*2,  1, 9,   DEFAULT_DURATION, get_trigger_for_pitch(GM_NOTE_VIBRA_SLAP));    // vibra
  make_euclid(&patterns[i++],  LEN*2,  1, 13,  DEFAULT_DURATION, get_trigger_for_pitch(GM_NOTE_RIDE_BELL));   // bell
  make_euclid(&patterns[i++],  LEN*2,  5, 13,  DEFAULT_DURATION, get_trigger_for_pitch(GM_NOTE_RIDE_CYMBAL_1));   // cymbal
  make_euclid(&patterns[i++],  LEN,    4, 3,   STEPS_PER_BEAT/2, PATTERN_BASS);  // bass (neutron) offbeat
  make_euclid(&patterns[i++],  LEN,    4, 3,   STEPS_PER_BEAT-1, PATTERN_MELODY); //NUM_TRIGGERS+NUM_ENVELOPES);  // melody as above
  make_euclid(&patterns[i++],  LEN,    1, 1,   STEPS_PER_BEAT*2, PATTERN_PAD_ROOT); // root pad
  make_euclid(&patterns[i++],  LEN,    1, 5,   STEPS_PER_BEAT,   PATTERN_PAD_PITCH); // root pad
  Serial.printf(" initialised %i Euclidian patterns\r\n", i-1);
  //make_euclid(&patterns[16],  LEN,    16, 0);    // bass (neutron)  sixteenth notes
  //make_euclid(&patterns[16],  LEN,    12, 4); //STEPS_PER_BEAT/2);    // bass (neutron)  rolling
  //make_euclid(&patterns[16],  LEN,    12, 4, STEPS_PER_BEAT/2);    // bass (neutron)  rolling*/
}


void debug_patterns() {
  EUC_printf("                 [");
  for (int i = 0 ; i < 16 ; i++) {
    EUC_printf("%01X ", i);
  }
  EUC_println("]");

  for (int x = 0 ; x < NUM_PATTERNS ; x++) {
    EUC_printf("Sequences are: "); EUC_printf("%01X ", x); EUC_printf("[");
    //for (int i = 0 ; i < patterns[x].steps ; i++) {
    for (int i = 0 ; i < 16 ; i++) {
      EUC_printf(patterns[x].stored[i % patterns[x].steps] ? "#" : (i >= patterns[x].steps ? "_" : "."));
      EUC_printf(" ");
    }
    EUC_println("]");
  }
}


// returns true if it means we should kill what's playing
bool euclidian_set_auto_play (bool enable) {
  bool current_mode = euclidian_auto_play;
  euclidian_auto_play = enable; //!euclidian_auto_play;
  if (bpm_internal_mode && !euclidian_auto_play && current_mode != euclidian_auto_play) {
    Serial.println("euclidian_set_auto_play returning TRUE!");
    return true;
  }
  Serial.println("euclidian_set_auto_play returning FALSE!");
  return false;
}

// returns true if mode changed
/*bool euclidian_set_mutate_mode(int mutate_mode) {
  bool current_mode = euclidian_mutate_mode;
  euclidian_mutate_mode = mutate_mode % EUCLIDIAN_MUTATE_MODE_MAX;
  return current_mode != euclidian_mutate_mode;
  }*/

// change to an euclidian setting; returns true if this CC was handled here
bool handle_euclidian_ccs(byte channel, byte number, byte value) {
  //NOISY_DEBUG(1000, number);
  if (channel != GM_CHANNEL_DRUMS) return false;

  if (number >= CC_EUCLIDIAN_ACTIVE_STATUS_START && number <= CC_EUCLIDIAN_ACTIVE_STATUS_END) { // + (ENV_CC_SPAN*NUM_ENVELOPES)) {
    EUC_printf("handle_euclidian_ccs(%i, %i) setting pattern number %i to %c\r\n", channel, number, number - CC_EUCLIDIAN_ACTIVE_STATUS_START, value>1?'Y':'N');
    set_pattern_active_status(number - CC_EUCLIDIAN_ACTIVE_STATUS_START, value > 1);
    return true;
  } else if (number == CC_EUCLIDIAN_SET_AUTO_PLAY) {
    if (euclidian_set_auto_play (value > 0)) {
      kill_notes();
      kill_envelopes();
      harmony.douse_all();
    }
    return true;
  } else if (number == CC_EUCLIDIAN_SEED_MODIFIER) {
    euclidian_seed_modifier = value;
    return true;
  } else if (number == CC_EUCLIDIAN_SEED_MODIFIER_2) {
    euclidian_seed_modifier_2 = value;
    return true;
  } else if (number == CC_EUCLIDIAN_RESET_BEFORE_MUTATE) {
    euclidian_reset_before_mutate = value > 0;
    return true;
  } else if (number == CC_EUCLIDIAN_SET_MINIMUM_PATTERN) {
    euclidian_mutate_minimum_pattern = value % NUM_PATTERNS;
    return true;
  } else if (number == CC_EUCLIDIAN_SET_MAXIMUM_PATTERN) {
    euclidian_mutate_maximum_pattern = value % NUM_PATTERNS;
    return true;
  } else if (number == CC_EUCLIDIAN_SEED_USE_PHRASE) {
    euclidian_seed_use_phrase = value;
    return true;
  } else if (number == CC_EUCLIDIAN_SET_MUTATE_MODE) {
    euclidian_mutate_mode = value % EUCLIDIAN_MUTATE_MODE_MAX;
    return true;
  } else if (number == CC_EUCLIDIAN_FILLS) {
    euclidian_fills_enabled = value > 0;
    return true;
  } else if (number == CC_EUCLIDIAN_HIHAT_SHUFF) {
    euclidian_shuffle_hats = value > 0;
    return true;
  } else if (number == CC_EUCLIDIAN_CLAP_FLAM) {
    euclidian_flam_clap = value >0;
    return true;
  }
  return false;
}


/* the original javascript from computermusicdesign
   //calculate a euclidean rhythm
  function euclid(steps,  pulses, rotation){
    storedRhythm = []; //empty array which stores the rhythm.
    //the length of the array is equal to the number of steps
    //a value of 1 for each array element indicates a pulse

    var bucket = 0; //out variable to add pulses together for each step

    //fill array with rhythm
    for( var i=0 ; i < steps ; i++){
        bucket += pulses;
            if(bucket >= steps) {
            bucket -= steps;
            storedRhythm.push(1); //'1' indicates a pulse on this beat
        } else {
            storedRhythm.push(0); //'0' indicates no pulse on this beat
        }
    }
  }
  The second function carries out the rotation:


  function rotateSeq(seq, rotate){
    var output = new Array(seq.length); //new array to store shifted rhythm
    var val = seq.length - rotate;

    for( var i=0; i < seq.length ; i++){
        output[i] = seq[ Math.abs( (i+val) % seq.length) ];
    }

    return output;
  }
  Querying the current beat

  Finally, we have a function for querying the storedRhythm array to find out if there is a pulse on the current beat:

  //send triggers
    function query_beat(curBeat){
    var curStep = curBeat % curSteps; //wraps beat around if it is higher than the number of steps
    return storedRhythm[curStep];
  }
*/

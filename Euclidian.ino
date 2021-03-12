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

void make_euclid(pattern_t *p, int steps = 0, int pulses = 0, int rotation = -1, int duration = -1) {
  // fill pattern_t according to parameters

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
  return query_pattern(p, step, (int)p->duration * -1);
}

void rotate_pattern(pattern_t *p, int rotate) {
  bool stored[p->steps];
  int offset = p->steps - rotate;
  for (int i = 0 ; i < p->steps ; i++) {
    stored[i] = p->stored[abs( (i + offset) % p->steps )];
  }
  memcpy(p->stored, stored, sizeof(stored));
}

void set_pattern_active_status(int pattern, bool active) {
  patterns[pattern].active_status = active;
  douse_trigger(pattern);
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
  if (mask_enabled && r < random(0, 100)) {
    mask_patterns(&patterns[pattern], &patterns[(pattern - 1) % NUM_PATTERNS]);
    EUC_printf("[EUC] Mutated pattern %i (and masked against pattern %i!)!\r\n", pattern, (pattern - 1) % NUM_PATTERNS);
  } else {
    EUC_printf("[EUC] Mutated pattern %i\r\n", pattern);
  }
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

  if (is_bpm_on_step) { 
    // TODO: configurable mutation frequency
    // start of phrase or middle of phrase
    if (is_bpm_on_beat && is_bpm_on_step && (is_bpm_on_phrase || (is_bpm_on_bar && current_bar == (BARS_PER_PHRASE / 2)))) { //(received_ticks / PPQN) % (SEQUENCE_LENGTH_STEPS / 2) == 0) { // commented part mutates every 2 bars

      // always reset even if not in mutate mode, so that fills work
      if (euclidian_reset_before_mutate && (mutate_enabled || euclidian_fills_enabled)) {
        EUC_println("Resetting euclidian before mutation!");
        initialise_euclidian();
      }

      bool should_mutate = mutate_enabled;

      if (should_mutate) {
        harmony.mutate();
        
        unsigned long seed = euclidian_seed_modifier;
        if (euclidian_seed_modifier_2 > 0) seed *= (256 + euclidian_seed_modifier_2 * 2);
        if (euclidian_seed_use_phrase) seed += current_phrase + 1;
        if (seed == 0) seed = 1;
        EUC_printf("Euclidian seed: %i\n", seed);
        randomSeed(seed);
  
        for (int i = 0 ; i < 3 ; i++) { // TODO: make the number of mutations configurable
          EUC_printf("Picking random mutation pattern between %i (incl) and %i (excl).. ", euclidian_mutate_minimum_pattern % NUM_PATTERNS, 1 + euclidian_mutate_maximum_pattern % NUM_PATTERNS);
          int ran = random(euclidian_mutate_minimum_pattern % NUM_PATTERNS, 1 + euclidian_mutate_maximum_pattern);
          EUC_printf("chose pattern %i\r\n", ran);
          randomSeed(seed + ran);
          if (euclidian_mutate_mode == EUCLIDIAN_MUTATE_MODE_TOTAL)
            mutate_euclidian_total(ran);
          else
            mutate_euclidian(ran);
          //debug_patterns();
        }
      }
    }

    // fills on last bar of phrase if enabled
    if (euclidian_fills_enabled && /*is_bpm_on_phrase &&*/ is_bpm_on_beat && is_bpm_on_step && ((is_bpm_on_bar && current_bar == (BARS_PER_PHRASE - 1)))) { //(received_ticks / PPQN) % (SEQUENCE_LENGTH_STEPS / 2) == 0) { // commented part mutates every 2 bars
      EUC_printf("FILLING at bar %i!\r\n", current_bar);
      randomSeed(current_phrase);
      for (int i = 0 ; i < 3 ; i++) {
        int ran = random(0/*euclidian_mutate_minimum_pattern % NUM_PATTERNS*/, 1 + euclidian_mutate_maximum_pattern);
        patterns[ran].rotation += 2;
        make_euclid(&patterns[ran]);
      }
      for (int i = 0 ; i < 3 ; i++) {
        int ran = random(euclidian_mutate_minimum_pattern % NUM_PATTERNS, 1 + euclidian_mutate_maximum_pattern);
        patterns[ran].pulses *= 2;
        if (patterns[ran].pulses > patterns[ran].steps) patterns[ran].pulses /= 8;
        make_euclid(&patterns[ran]);
      }
    }

    // its a beat!
    //EUC_printf(" >>STEP %2.2u", current_step);
    //EUC_printf(" >>BEAT %1.1u", current_beat);
    EUC_printf("[EUC] [mode %i] ", demo_mode );
    EUC_printf(" (ticks = %5u", ticks); EUC_printf(") ");
    EUC_printf(" >>BPM %3.3f >>STEP %i:%i:%2.2u.%1.2u ", bpm_current, current_phrase, current_bar, current_beat, current_step);
    EUC_printf("[ ");
    for (int i = 0 ; i < NUM_PATTERNS ; i++) {
      //EUC_printf("\r\n>>>>>>>>>>>about to query current_step %i\r\n", current_step);
      if (!patterns[i].active_status) continue;

      if (query_pattern(&patterns[i], current_step, 0 /*offset*/, current_bar)) {  // step trigger
        douse_trigger(i, 127, true);
        fire_trigger(i, 127, true);
        if (i < 16) {
          EUC_printf("%01X", i); // print as hex
        } else {
          //EUC_printf("{%01i}", bass_currently_playing); // for bass note indicator
          EUC_printf("%3s ", get_note_name(harmony.get_currently_playing_root()).c_str()); // for bass note indicator
        }
        //EUC_printf("%c", 97 + i); // print a...q (65 for uppercase)
        EUC_printf(" ");
      } else if (query_pattern_note_off(&patterns[i], current_step, current_bar)) {  // step kill
        douse_trigger(i, 127, true);
        // TODO: turn off according to some other thing.. eg cut groups?
        if (i == 16) EUC_printf("..."); // add extra dots for bass note indicator
        EUC_printf(".", i); EUC_printf(" ");
      } else {
        EUC_printf("  ");
        if (i == 16) EUC_printf("   "); // add extra spaces for bass note indicator
      }
    }
    EUC_printf("]  ");
    //EUC_printf("bass playing pitch %2i ", bass_currently_playing);
    EUC_printf (is_bpm_on_beat ? "<<<<BEAT!" : "<<  STEP");
    if (current_beat == 0) {
      EUC_printf(" (first beat of bar)");
    }
    if (is_bpm_on_beat && is_bpm_on_bar && is_bpm_on_phrase) {
      EUC_printf(" (first beat of phrase!)");
    }

    if (autobass_input.is_note_held()) {
      EUC_printf(" [%s]", autobass_input.get_debug_notes_held());
    }

    EUC_println("");
  }
  //EUC_printf("ticks is %i, ticks_per_step/2 is %i, result of mod is %i\n", ticks, TICKS_PER_STEP/2, ticks%TICKS_PER_STEP);
  last_processed = ticks;
}

void initialise_euclidian() {

  EUC_println("initialise_euclidian():");

  EUC_println("resetting all:-");
  const int LEN = SEQUENCE_LENGTH_STEPS;
  for (int i = 0 ; i < NUM_PATTERNS ; i++) {
    make_euclid(&patterns[i], LEN, 0, 1, DEFAULT_DURATION); // initialise patterns to default length, zero pulses, default rotation of '1' and default duration of 1 step
    // if we don't do this then this does a bad job of resetting the patterns
  }

  // definition: make_euclid( pattern_t *p,  int steps = 0,  int pulses = 0,   int rotation = 0,   int duration = STEPS_PER_BEAT/4   ) {

  EUC_println("initialising:-");
  //     length, pulses, rotation, duration
  make_euclid(&patterns[0],   LEN,    4, 1,   DEFAULT_DURATION);    // kick
  make_euclid(&patterns[1],   LEN,    5, 1,   DEFAULT_DURATION);    // stick
  make_euclid(&patterns[2],   LEN,    2, 5,   DEFAULT_DURATION);    // clap
  make_euclid(&patterns[3],   LEN/4,  16,1,   DEFAULT_DURATION);   // snare
  make_euclid(&patterns[4],   LEN,    3, 3,   DEFAULT_DURATION);    // crash 1
  make_euclid(&patterns[5],   LEN,    7, 1,   DEFAULT_DURATION);    // tamb
  make_euclid(&patterns[6],   LEN,    9, 1,   DEFAULT_DURATION);    // hi tom!
  make_euclid(&patterns[7],   LEN/4,  2, 3,   DEFAULT_DURATION);    // low tom
  make_euclid(&patterns[8],   LEN/2,  2, 3,   DEFAULT_DURATION);    // pedal hat
  make_euclid(&patterns[9],   LEN,    4, 3,   DEFAULT_DURATION);    // open hat
  make_euclid(&patterns[10],  LEN,    16, 1,  DEFAULT_DURATION);   // closed hat
  make_euclid(&patterns[11],  LEN*2,  1 , 1,  DEFAULT_DURATION);   // crash 2
  make_euclid(&patterns[12],  LEN*2,  1 , 5,  DEFAULT_DURATION);   // splash
  make_euclid(&patterns[13],  LEN*2,  1, 9,   DEFAULT_DURATION);    // vibra
  make_euclid(&patterns[14],  LEN*2,  1, 13,  DEFAULT_DURATION);   // bell
  make_euclid(&patterns[15],  LEN*2,  5, 13,  DEFAULT_DURATION);   // cymbal
  make_euclid(&patterns[16],  LEN,    4, 3, STEPS_PER_BEAT / 2);  // bass (neutron) offbeat
  //make_euclid(&patterns[16],  LEN,    16, 0);    // bass (neutron)  sixteenth notes
  //make_euclid(&patterns[16],  LEN,    12, 4); //STEPS_PER_BEAT/2);    // bass (neutron)  rolling
  //make_euclid(&patterns[16],  LEN,    12, 4, STEPS_PER_BEAT/2);    // bass (neutron)  rolling*/

  /*make_euclid(&patterns[0], 16, 16, 0);
    make_euclid(&patterns[1], 13, 8, 0);
    make_euclid(&patterns[2], 16, 4, 0);
    make_euclid(&patterns[3], 16, 2, 0);
    make_euclid(&patterns[4], 16, 3, 1);*/
  /*make_euclid(&patterns[5], 16, 5, 0);
    make_euclid(&patterns[6], 13, 6, 5);
    make_euclid(&patterns[7], 16, 7, 0);
    make_euclid(&patterns[8], 12, 9, 3);
    make_euclid(&patterns[9], 16, 10, 0);
    make_euclid(&patterns[10], 16, 11, 1);

    make_euclid(&patterns[11], 16, 1, 1);
    make_euclid(&patterns[12], 16, 1, 5);
    make_euclid(&patterns[13], 16, 1, 9);
    make_euclid(&patterns[14], 16, 1, 13);
    make_euclid(&patterns[15], 16, 1, 3);*/
  //delay(100);
  //debug_patterns();
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
    return true;
  }
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

  if (number >= CC_EUCLIDIAN_ACTIVE_STATUS_START && number <= CC_EUCLIDIAN_ACTIVE_STATUS_START + NUM_PATTERNS) { // + (ENV_CC_SPAN*NUM_ENVELOPES)) {
    set_pattern_active_status(number - CC_EUCLIDIAN_ACTIVE_STATUS_START, value > 1);
    return true;
  } else if (number == CC_EUCLIDIAN_SET_AUTO_PLAY) {
    if (euclidian_set_auto_play (value > 0)) {
      kill_notes();
      kill_envelopes();
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
  }
  /*else if (number==CC_EUCLIDIAN_SET_MUTATE_MODE) {
    euclidian_set_mutate_mode(value % EUCLIDIAN_MUTATE_MODE_MAX);
    return true;
    }*/
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

// based on example code/pseudocode from https://www.computermusicdesign.com/simplest-euclidean-rhythm-algorithm-explained/

#include "MidiOutput.hpp" // because we need to send MIDI

#define EUC_DEBUG

#ifdef EUC_DEBUG
#define EUC_DEBUG 1
#else
#define EUC_DEBUG 0
#endif

// https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c/1644898#1644898
#define EUC_printf(fmt, ...) do { if (EUC_DEBUG) Serial.printf((fmt), ##__VA_ARGS__); } while (0)
#define EUC_println(fmt, ...) do { if (EUC_DEBUG) Serial.println((fmt), ##__VA_ARGS__); } while (0)

void make_euclid(pattern_t *p, int steps = 0, int pulses = 0, int rotation = 0, int duration = 0) {
  // fill pattern_t according to parameters

  if (steps > 0)    p->steps = steps;
  if (pulses > 0)   p->pulses = pulses;
  if (rotation > 0) p->rotation = rotation;
  if (duration > 0) p->duration = duration;

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

bool query_pattern(pattern_t *p, int beat, int offset = 0) {
  int curStep = (beat + offset) % p->steps; //wraps beat around if it is higher than the number of steps
  if (curStep<0) curStep = (p->steps) + curStep;  // wrap around if result passes sequence boundary
  //EUC_printf("\r\nquery_pattern querying step %i\r\n", curStep);
  return p->stored[curStep];
}

// find out if note should be killed this step
bool query_pattern_note_off(pattern_t *p, int beat) { //, int offset = -2) {
  //EUC_printf("\r\nnote_off querying beat %i with duration-based offset %i\r\n", beat, (int)p->duration*-1);
  return query_pattern(p, beat, (int)p->duration*-1);
}

void rotate_pattern(pattern_t *p, int rotate) {
  bool stored[p->steps];
  int offset = p->steps - rotate;
  for (int i = 0 ; i < p->steps ; i++) {
    stored[i] = p->stored[abs( (i + offset) % p->steps )];
  }
  memcpy(p->stored, stored, sizeof(stored));
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

  if (is_bpm_on_step) { //0==ticks%TICKS_PER_STEP) {
    if (mutate_enabled && /*is_bpm_on_phrase &&*/ is_bpm_on_beat && is_bpm_on_step && (received_ticks / PPQN) % (SEQUENCE_LENGTH_STEPS / 2) == 0) { //==current_song_position%SEQUENCE_LENGTH_BEATS) {
      for (int i = 0 ; i < 3 ; i++) {
        int ran = random(1, NUM_PATTERNS);
        mutate_euclidian(ran);
        //debug_patterns();
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
      if (query_pattern(&patterns[i], current_step)) {  // step trigger
        douse_trigger(i, 127, true);
        fire_trigger(i, 127, true);
        if (i<16) {
          EUC_printf("%01X", i); // print as hex 
        } else {
          //EUC_printf("{%01i}", bass_currently_playing); // for bass note indicator
          EUC_printf("%3s ", get_note_name(bass_currently_playing).c_str()); // for bass note indicator
        }
        //EUC_printf("%c", 97 + i); // print a...q (65 for uppercase)
        EUC_printf(" ");   
      } else if (query_pattern_note_off(&patterns[i], current_step)) {  // step kill
        // TODO: turn off according to some other thing.. eg cut groups?
        if (i==16) EUC_printf("..."); // add extra dots for bass note indicator
        EUC_printf(".", i); EUC_printf(" ");
        douse_trigger(i, 127, true);
      } else {
        EUC_printf("  ");
        if (i==16) EUC_printf("   ");  // add extra spaces for bass note indicator
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

    if (bass.is_note_held()) {
      EUC_printf(" [%s]", bass.get_debug_notes_held());
    }
    
    EUC_println("");
  }
  //EUC_printf("ticks is %i, ticks_per_step/2 is %i, result of mod is %i\n", ticks, TICKS_PER_STEP/2, ticks%TICKS_PER_STEP);
  last_processed = ticks;
}

void initialise_euclidian() {
  const int LEN = SEQUENCE_LENGTH_STEPS;
  for (int i = 0 ; i < NUM_PATTERNS ; i++) {
    make_euclid(&patterns[i], LEN, 0, 1, STEPS_PER_BEAT/STEPS_PER_BEAT); // initialise patterns to default length, zero pulses, default rotation of '1' and default duration of 1 step
  }

  // definition: make_euclid( pattern_t *p,  int steps = 0,  int pulses = 0,   int rotation = 0,   int duration = STEPS_PER_BEAT/4   ) {
  
  EUC_println("initialise_euclidian():");
  make_euclid(&patterns[0],   LEN,    4);       // kick
  make_euclid(&patterns[1],   LEN,    5, 0);    // stick
  make_euclid(&patterns[2],   LEN,    2, 5);    // clap
  make_euclid(&patterns[3],   LEN/4,  16  );    // snare
  make_euclid(&patterns[4],   LEN,    3, 3);    // crash 1
  make_euclid(&patterns[5],   LEN,    7);       // tamb
  make_euclid(&patterns[6],   LEN,    9);       // hi tom!
  make_euclid(&patterns[7],   LEN/4,  2, 3);    // low tom
  make_euclid(&patterns[8],   LEN/2,  2, 3);    // pedal hat
  make_euclid(&patterns[9],   LEN,    4, 3);    // open hat
  make_euclid(&patterns[10],  LEN,    16);      // closed hat
  make_euclid(&patterns[11],  LEN,    1 , 1);   // crash 2
  make_euclid(&patterns[12],  LEN,    1 , 5);   // splash
  make_euclid(&patterns[13],  LEN,    1, 9);    // vibra
  make_euclid(&patterns[14],  LEN,    1, 13);   // bell
  make_euclid(&patterns[15],  LEN,    5, 13);   // cymbal
  make_euclid(&patterns[16],  LEN,    4, 3, STEPS_PER_BEAT/2);    // bass (neutron) offbeat
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
  delay(100);
  debug_patterns();
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

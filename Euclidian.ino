// based on pseudocode from https://www.computermusicdesign.com/simplest-euclidean-rhythm-algorithm-explained/

void make_euclid(pattern_t *p, int steps = 0, int pulses = 0, int rotation = 0) {
  //static bool stored[steps];
  //static pattern p;

  if (steps>0)    p->steps = steps;
  if (pulses>0)   p->pulses = pulses;
  if (rotation>0) p->rotation = rotation;

  int bucket = 0;
  for (int i = 0 ; i < p->steps ; i++) {
    bucket += pulses;
    if (bucket >= p->steps) {
      bucket -= p->steps;
      p->stored[i] = true;
    } else {
      p->stored[i] = false;
    }
  }
  p->original_steps = p->steps;

  if (p->rotation>0) {
    rotate_pattern(p, p->rotation);
  }

}

bool query_pattern(pattern_t *p, int beat) {
    int curStep = beat % p->steps; //wraps beat around if it is higher than the number of steps
    return p->stored[curStep];
}

void rotate_pattern(pattern_t *p, int rotate) {
  bool stored[p->steps];
  int offset = p->steps - rotate;
  for (int i = 0 ; i < p->steps ; i++) {
    stored[i] = p->stored[abs( (i+offset) % p->steps )];
  }
  memcpy(p->stored, stored, sizeof(stored));
}

void mutate_euclidian(pattern_t *p) {
  int r = random(0,100);
  if (r>50) {
    if (r>75) {
      p->pulses+=1;
    } else {
      p->pulses-=1;
    }
  } else if (r<25) {
    p->rotation += 1;
  } else if (r>25) {
    p->pulses *=2;
  } else {
    p->pulses /=2;
  }
  if (p->pulses >= p->original_steps || p->pulses<=0) {
    p->pulses = 1;
  }
  make_euclid(p, p->steps, p->pulses, p->rotation);
}

void process_euclidian(int ticks) {
  static int last_processed = 0;
  //ticks /= (PPQN/4);  // step = quarter-note beat 
  //ticks /= (PPQN);      // step = bar ?
  //ticks /= (PPQN/(24/2));      // step = half-beat

  ticks; //*= 4; // 
 
  //int current_step = (ticks/TICKS_PER_STEP)%16;
  //int current_beat = current_step / 4; //(ticks/24);//%16;

  if (ticks==last_processed) return;
  //if (0==(ticks * (16))) {
  if (0==ticks%TICKS_PER_STEP) {
    if (is_bpm_on_beat && is_bpm_on_step && (received_ticks/PPQN)%(SEQUENCE_LENGTH_STEPS/2)==0) { //==current_song_position%SEQUENCE_LENGTH_BEATS) {
      //if ((ticks/PPQN)%16 == 0) {
      int ran = random(1,NUM_PATTERNS);
      mutate_euclidian(&patterns[ran]);
      Serial.printf("mutated pattern %i!\r\n", ran);
      //debug_patterns();
    }
    
    // its a beat!
    //Serial.printf(" >>STEP %2.2u", current_step); 
    //Serial.printf(" >>BEAT %1.1u", current_beat); 
    Serial.printf(">>BPM %3.3f >>STEP %2.2u.%1.2u ", bpm_current, current_beat, current_step);
    Serial.printf(" (ticks = %.4u", ticks); Serial.print(") ");
    Serial.print("[ ");
    for (int i = 0 ; i < NUM_PATTERNS ; i++) {
      if (query_pattern(&patterns[i], current_step)) {
        //if (i<5) update_envelope(i, 127, true);
        Serial.printf("%01X", i); Serial.print(" ");
        if (i >= NUM_TRIGGERS) { // trigger envelope
          //handleNoteOn(10, i, random(1,127));
          update_envelope(i-NUM_TRIGGERS, 127, true);
        } else {
          fire_trigger(MUSO_NOTE_MINIMUM + i, 127);
        }
      } else {
          Serial.printf("  ");
      }
    }
    Serial.print("]  ");
    Serial.print (is_bpm_on_beat ? "<<<<BEAT" : "<<  STEP");
    if (current_beat==0) {
      Serial.print(" (first beat of bar)");
    }
    Serial.println("");
  } else if ((TICKS_PER_STEP/2)==ticks%TICKS_PER_STEP) {
    // its between a beat!
    //Serial.print("Should turn off on ticks = "); Serial.println(ticks);
    for (int i = 0 ; i < NUM_PATTERNS ; i++) {
      //if (i<5) update_envelope(i, 127, false);
      if (i >= NUM_TRIGGERS) { // trigger envelope
        update_envelope(i-NUM_TRIGGERS, 127, false);
      } else {
        douse_trigger(MUSO_NOTE_MINIMUM + i, 127);
      }
    }
  }
  //Serial.printf("ticks is %i, ticks_per_step/2 is %i, result of mod is %i\n", ticks, TICKS_PER_STEP/2, ticks%TICKS_PER_STEP);
  last_processed = ticks;
}

void initialise_euclidian() {
  const int LEN = SEQUENCE_LENGTH_STEPS;
  for (int i = 0 ; i < NUM_PATTERNS ; i++) {
    //make_euclid(&patterns[i], 16, 16-(i+1), 1);
    make_euclid(&patterns[i], LEN, 0, 1); // initialise patterns to empty
  }
  Serial.println("initialise_euclidian():");
  make_euclid(&patterns[0],   LEN,    4);       // kick
  make_euclid(&patterns[1],   LEN,    5, 0);    // stick
  make_euclid(&patterns[2],   LEN,    2, 5);    // clap
  make_euclid(&patterns[3],   LEN/4,  16);     // snare
  make_euclid(&patterns[4],   LEN,    3, 3);    // crash 1
  make_euclid(&patterns[5],   LEN,    7);       // tamb
  make_euclid(&patterns[6],   LEN,    9);       // hi tom!
  make_euclid(&patterns[7],   LEN/4,  2, 3);   // low tom
  make_euclid(&patterns[8],   LEN/2,  2, 3);   // pedal hat
  make_euclid(&patterns[9],   LEN,    4, 3);    // open hat
  make_euclid(&patterns[10],  LEN,    16);      // closed hat
  make_euclid(&patterns[11],  LEN,    1 , 1);    // crash 2
  make_euclid(&patterns[12],  LEN,    1 , 5);    // splash
  make_euclid(&patterns[13],  LEN,    1, 9);    // vibra
  make_euclid(&patterns[14],  LEN,    1, 13);   // bell
  make_euclid(&patterns[15],  LEN,    5, 13);   // cymbal

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
  Serial.print("                 [");
  for (int i = 0 ; i < 16 ; i++) {
    Serial.printf("%01X ", i);
  }
  Serial.println("]");

  for (int x = 0 ; x < NUM_PATTERNS ; x++) {
    Serial.print("Sequences are: "); Serial.printf("%01X ", x); Serial.print("[");
    //for (int i = 0 ; i < patterns[x].steps ; i++) {
    for (int i = 0 ; i < 16 ; i++) {
      Serial.print(patterns[x].stored[i%patterns[x].steps] ? '#' : (i>=patterns[x].steps?'_':'.'));
      Serial.print(" ");
    }
    Serial.println("]");
  }
}

/* the original javascript from computermusicdesign
 * //calculate a euclidean rhythm
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

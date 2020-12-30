// utility functions for calculating BPM

unsigned long received_ticks = 0;
unsigned long last_tick_received_at = 0;
unsigned long first_tick_received_at = 0;

// for telling the rest of the program about what step and beat we're on
int current_step = 0; 
int current_beat = 0; 

bool is_bpm_on_beat = false;
bool is_bpm_on_step = false;

double bpm_current = 60.0f; //120.0f;

bool bpm_internal_mode = false;

// stuff for calculating BPM
#define last_beat_sample_size 4
int last_beat_stamp[last_beat_sample_size];
int ph = 0;
unsigned long last_beat_at = 0;
static unsigned long last_ticked = 0;


void bpm_update_status( unsigned int received_ticks ) {
  current_step = (received_ticks/TICKS_PER_STEP) % SEQUENCE_LENGTH_STEPS;
  current_beat = current_step / STEPS_PER_BEAT; //(ticks/24);//%16;
  is_bpm_on_beat = (0==received_ticks%PPQN);
  is_bpm_on_step = (0==received_ticks%TICKS_PER_STEP);
}


void bpm_reset_clock (int offset = 0) {
  received_ticks = 0 + offset;  // set to -1 so the next tick starts us off on beat 0 step 0 tick 0
  last_tick_received_at = 0;
  first_tick_received_at = 0;
  last_ticked = millis();
  
  memset(last_beat_stamp,0,sizeof(last_beat_stamp));  // clear the bpm calculator history

  bpm_update_status(received_ticks - offset);

  Serial.printf("After reset, received_ticks is %i, current beat is %i, current step is %i\n", received_ticks, current_beat, current_step); 
  
}

unsigned int bpm_clock() {
  unsigned long now = millis();
  if (now - last_input_at > IDLE_TIMEOUT && now - last_tick_at > IDLE_TIMEOUT && activeNotes==0) {
    if (!bpm_internal_mode) {
      // we just switched from external to internal mode, so need to reset clock?
      bpm_reset_clock();
    }
    bpm_internal_mode = true;

    int delta_ms = (now - last_ticked);
    //float ms_per_tick = (60.0 / (bpm_current * PPQN));
    double ms_per_tick = (60.0d / (bpm_current * (double)PPQN));
    double delta_ticks = (double)delta_ms / (1000.0d*ms_per_tick);
    if ((int)delta_ticks>0) {
      received_ticks += delta_ticks;
      Serial.printf("got delta_ms %i and ms_per_tick %3.3f, delta_ticks is %3.3f\n", delta_ms, ms_per_tick, delta_ticks);
      // calculate what current ticks, beat and step should be based on internal BPM represention
      // so all we know is millis()
      // so we need to know how many ticks to update received_ticks by since the last time bpm_clock was called
      //      need to know ms per pulse at specified bpm
  
      bpm_update_status(received_ticks);
      //Serial.println("ticked!");
      last_ticked = now;
    }
  } else {
    if (bpm_internal_mode) {
      // we just switched from internal to external, so need to reset clock?
      bpm_reset_clock(-1);
      //received_ticks-=1;
      bpm_update_status(received_ticks);
    }
    bpm_internal_mode = false;
  }
  return received_ticks;
}


void push_beat(unsigned long duration) {
  last_beat_stamp[ph] = duration;
  //Serial.printf("pushed duration %i to slot %i\n", duration, ph);
  ph++;
  if (ph>=last_beat_sample_size) ph = 0;
}

double average (int * array, int len)  // assuming array is int.
{
  long sum = 0L ;  // sum will be larger than an item, long for safety.
  int counted = 0;
  for (int i = 0 ; i < len ; i++) {
    //Serial.printf("%i: got duration %u", i, array[i]);
    if (array[i]==0) continue;
    sum += array [i] ;
    counted++;
  }
  //Serial.printf("returning %3.3f\n", ((float) sum) / counted );
  return  ((double) sum) / (double)counted ;  // average will be fractional, so float may be appropriate.
}


double bpm_calculate_current () {

  /* use this block to calculate BPM from the start of receiving ticks -- this will not track well when BPM is changed!! */
  /*double beats = (double)received_ticks/(double)PPQN;
  double elapsed_ms = millis() - first_tick_received_at;*/
  //estimated_ticks_per_ms = received_ticks / elapsed_ms;

  /* this version uses average of the last 4 received beats (actually steps) to calculate BPM so it tracks to changes better */
  double beats = 1.0d / (double)last_beat_sample_size; //0.25; //1;
  
  double elapsed_ms = average(last_beat_stamp, 4);

  double elapsed_minutes = (elapsed_ms / 60000.0d);
  double bpm = (beats / elapsed_minutes);
  // speed = distance/time
  // bpm = beats/time

  //Serial.printf("bpm calculated as %3.3f (from elapsed_ms %3.3f and beats %3.3f)\r\n", bpm, elapsed_ms,  beats);

  return bpm;
}


void bpm_receive_clock_tick () {
  received_ticks++;
  unsigned long now = millis();
  last_tick_received_at = now;
  if (first_tick_received_at == 0) { 
    first_tick_received_at = now;
  }

  if (is_bpm_on_step) {
    push_beat(now - last_beat_at);
    last_beat_at = now;
  }

  bpm_update_status(received_ticks);

  if (is_bpm_on_beat)
    bpm_current = bpm_calculate_current();

}

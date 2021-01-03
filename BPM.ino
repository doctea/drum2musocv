#include "BPM.hpp"

#include "MidiInput.hpp"



void bpm_update_status( unsigned int received_ticks ) {
  current_step = (received_ticks/TICKS_PER_STEP) % SEQUENCE_LENGTH_STEPS;
  current_beat = current_step / STEPS_PER_BEAT; //(ticks/24);//%16;
  current_bar  = (received_ticks/(TICKS_PER_STEP*BEATS_PER_BAR)) % BARS_PER_PHRASE;
  current_phrase = (received_ticks/(TICKS_PER_STEP*BEATS_PER_BAR)) / BARS_PER_PHRASE;

  is_bpm_on_beat = (0==received_ticks%PPQN);
  is_bpm_on_step = (0==received_ticks%TICKS_PER_STEP);
  is_bpm_on_bar = is_bpm_on_beat && current_beat == 0;
  is_bpm_on_phrase = is_bpm_on_bar && (current_phrase % BARS_PER_PHRASE) == 0;
  if (is_bpm_on_beat) {
    current_song_position = received_ticks/PPQN;  // TODO: need to take into account that song position is set by the DAW sometimes....
    //Serial.printf("current_beat is %i, current song position is %i\r\n", current_beat, current_song_position);
  }
}


void bpm_reset_clock (int offset) {
  received_ticks = 0 + offset;  // set to -1 so the next tick starts us off on beat 0 step 0 tick 0
  last_tick_received_at = 0;
  first_tick_received_at = 0;
  last_ticked = millis();
  
  memset(last_beat_stamp,0,sizeof(last_beat_stamp));  // clear the bpm calculator history

  bpm_update_status(received_ticks - offset);

  Serial.printf("After reset, received_ticks is %i, current beat is %i, current step is %i\n", received_ticks, current_beat, current_step); 
}

void debug_print_step_info(char *mode) {
#ifdef ENABLE_STEP_DEBUG
    Serial.printf("[%s] >>BPM %3.3f >>PHRASE %i >> BAR %i >>STEP %2.2u.%1.2u ", mode, bpm_current, current_phrase, current_bar, current_beat, current_step);
    Serial.printf(" (received_ticks = %.4u", received_ticks); Serial.print(") ");
    Serial.print (is_bpm_on_beat ? "<<<<BEAT!" : "<<  STEP ");
    if (current_beat==0) {
      Serial.print(" (first beat of bar)");
    }
    if (is_bpm_on_beat && is_bpm_on_bar && is_bpm_on_phrase) {
      Serial.print(" (first beat of phrase!)");
    }
    Serial.println("");
#endif
}

unsigned int bpm_clock() {
  unsigned long now = millis();
  if (/*now - last_input_at > IDLE_TIMEOUT && activeNotes==0 && */now - last_tick_at > IDLE_TIMEOUT ) {
    // internal mode branch
    if (!bpm_internal_mode) {
      // we only just switched from external to internal mode, so need to reset clock?
      bpm_reset_clock();
    }
    bpm_internal_mode = true;

    int delta_ms = (now - last_ticked);
    double ms_per_tick = (60.0d / (bpm_current * (double)PPQN));
    double delta_ticks = (double)delta_ms / (1000.0d*ms_per_tick);
    if ((int)delta_ticks>0) {
      received_ticks += delta_ticks;
  
      bpm_update_status(received_ticks);
      last_ticked = now;

      if (is_bpm_on_step) {
        debug_print_step_info("INT");
      }
    }
  } else {
    // external clock branched
    if (bpm_internal_mode) {
      // we only just switched from internal to external, so need to reset clock?
      bpm_reset_clock(-1);
    }
    bpm_internal_mode = false;
    static int last_tick;
    if (last_tick!=received_ticks) {
      bpm_update_status(received_ticks);
      last_tick = received_ticks;
      if (is_bpm_on_step) {
        debug_print_step_info("EXT");
      }
    }
  }

  if (received_ticks%PPQN==0) {
    midi_send_clock();
  }
  return received_ticks;
}


void push_beat(unsigned long duration) {
  last_beat_stamp[ph] = duration;
  //Serial.printf("pushed duration %i to slot %i\n", duration, ph);
  ph++;
  if (ph>=last_beat_sample_size) ph = 0;
}

// pinched from interwebs and modified
double average_step_length (int len)  // assuming array is int.
{
  long sum = 0L ;  // sum will be larger than an item, long for safety.
  int counted = 0;
  for (int i = 0 ; i < last_beat_sample_size ; i++) {
    //Serial.printf("%i: got duration %u", i, array[i]);
    if (last_beat_stamp[i]==0l) continue;
    sum += last_beat_stamp [i] ;
    counted++;
  }
  
  if (counted >= len) {
    //Serial.printf("counted %i with sum %u, returning %3.3f \r\n", counted, sum, ((double) sum) / (double)counted );
    return  ((double) sum) / (double)counted ;  // average will be fractional, so float may be appropriate.
  } else {
    //Serial.printf("returning last value %3.3f\r\n", last_bpm);
    return last_bpm;
  }
}


double bpm_calculate_current () {

  /* use this block to calculate BPM from the start of receiving ticks -- this will not track well when BPM is changed!! */
  /*double beats = (double)received_ticks/(double)PPQN;
  double elapsed_ms = millis() - first_tick_received_at;*/
  //estimated_ticks_per_ms = received_ticks / elapsed_ms;

  /* this version uses average of the last 4 received beats (actually steps) to calculate BPM so it tracks to changes better */
  double beats = 1.0d / (double)last_beat_sample_size; //0.25; //1;
  
  double elapsed_ms = average_step_length(4);

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

  last_tick_at = now; //millis();
  
  last_tick_received_at = now;
  if (first_tick_received_at == 0) { 
    first_tick_received_at = now;
  }

  bpm_update_status(received_ticks);


  if (is_bpm_on_step) {
    push_beat(now - last_beat_at);
    last_beat_at = now;
  }

  if (is_bpm_on_beat) {
    bpm_current = bpm_calculate_current();
    last_bpm = bpm_current;
  }

}

// utility functions for calculating BPM

unsigned long received_ticks = 0;
unsigned long last_tick_received_at = 0;
unsigned long first_tick_received_at = 0;

// for telling the rest of the program about what step and beat we're on
int current_step = 0; 
int current_beat = 0; 

bool is_bpm_on_beat = false;
bool is_bpm_on_step = false;

float bpm_current = 60.0f; //120.0f;

bool bpm_internal_mode = false;

void bpm_update_status( unsigned int received_ticks ) {
  current_step = (received_ticks/TICKS_PER_STEP) % SEQUENCE_LENGTH_STEPS;
  current_beat = current_step / STEPS_PER_BEAT; //(ticks/24);//%16;
  is_bpm_on_beat = (0==received_ticks%PPQN);
  is_bpm_on_step = (0==received_ticks%TICKS_PER_STEP);
}

static unsigned long last_ticked = 0;
unsigned int bpm_clock() {
  unsigned long now = millis();
  if (now - last_input_at > IDLE_TIMEOUT && now - last_tick_at > IDLE_TIMEOUT && activeNotes==0) {
    bpm_internal_mode = true;

    int delta_ms = (now - last_ticked);
    float ms_per_tick = (60.0 / (bpm_current * PPQN));
    float delta_ticks = delta_ms * ms_per_tick;
    if ((int)delta_ticks>0) {
      received_ticks += delta_ticks;
      Serial.printf("got delta_ms %i and ms_per_tick %f, delta_ticks is %f\n", delta_ms, ms_per_tick, delta_ticks);
      // calculate what current ticks, beat and step should be based on internal BPM represention
      // so all we know is millis()
      // so we need to know how many ticks to update received_ticks by since the last time bpm_clock was called
      //      need to know ms per pulse at specified bpm
  
      bpm_update_status(received_ticks);
      Serial.println("ticked!");
      last_ticked = now;
    }
  } else {
    bpm_internal_mode = false;
  }
  return received_ticks;
}

void bpm_reset_clock () {
  received_ticks = 0;
  last_tick_received_at = 0;
  first_tick_received_at = 0;
  last_ticked = 0;
}

void bpm_receive_clock_tick () {
  received_ticks++;
  unsigned long now = millis();
  last_tick_received_at = now;
  if (first_tick_received_at == 0) { 
    first_tick_received_at = now;
  }

  bpm_update_status(received_ticks);

  bpm_current = bpm_calculate_current();

}

double bpm_calculate_current () {
  /*static int beat_count = 0;
  static unsigned long count_received_ticks, count_received_at;
  if (is_bpm_on_beat) {  // if on a beat, increment
    beat_count++;

    if (beat_count>=4) {
      // reset 
      Serial.println("fourth beat, resetting bpm counter..");
      count_received_ticks = received_ticks - count_received_ticks;
      count_received_at = millis();
      beat_count = 0;
    }
  }*/
  
  // bpm = beats per minute
  // = (received_ticks*PPQN) / (elapsed time in minutes)
  double beats = (double)received_ticks/(double)PPQN;
  
  double elapsed_ms = millis() - first_tick_received_at;
  //double elapsed_ms = millis() - count_received_at;
  estimated_ticks_per_ms = received_ticks / elapsed_ms;

  double elapsed_minutes = (elapsed_ms / 60000.0d);
  double bpm = (beats / elapsed_minutes);
  // speed = distance/time
  // bpm = beats/time

  Serial.printf("bpm calculated as %3.3f (from elapsed_ms %3.3f and beats %3.3f)\r\n", bpm, elapsed_ms,  beats);

  return bpm;
}

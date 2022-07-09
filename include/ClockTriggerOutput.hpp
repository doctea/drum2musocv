#ifndef CLOCKTRIG_INCLUDED
#define CLOCKTRIG_INCLUDED

#include "Config.h"

#ifdef ENABLE_CLOCK_TRIGGER

#define DEBUG_CLOCKTRIG false //set to true to enable debugging

/// clock stuff (hacky via CD74HC4067 multiplexor output module https://www.amazon.co.uk/dp/B07VF14YNG/ref=pe_3187911_185740111_TE_item)

#define CLOCK_OUT_START_PIN 2   // arduino pin number that the multiplexor address bus starts at
#define CLOCK_BEAT      0
#define CLOCK_BEAT_ALT  1
#define CLOCK_BAR       2
#define CLOCK_PHRASE    3
#define CLOCK_COUNT     4
const static byte clock_map[] = { 4, 2, 3, 10 };  // this is the one, for some reason?  doesn't make sense, have i got my pins swapped somewhere?
//const static byte clock_map[] = { 1, 2, 6, 11 };
//const static byte clock_map[] = { 1, 2, 6, 10 };
//const static byte clock_map[] = { 2, 6, 4, 10 };

#define CLOCK_LOOP_COUNT  5  // how many ms each trigger should stay HIGH before moving onto next trigger or going LOW

static int should_send_clock[4] = { 0, 0, 0, 0 }; // keep track of whether an output should be active + how long its been HIGH

bool clock_on = false;
static unsigned long last_clock_trigger_ticked;
static unsigned long last_beat_clock_millis;

void initialise_clock_outputs() {
  for (int i = 0 ; i < 4 ; i++) {
    pinMode(CLOCK_OUT_START_PIN + i, OUTPUT);
    digitalWrite(CLOCK_OUT_START_PIN + i, LOW);
  }
}

void clock_output (byte clock_number, bool on) {
  if (!on) {
    for (int i = 0 ; i < 4 ; i++) {
      digitalWrite( CLOCK_OUT_START_PIN + i, LOW );
    }
    clock_on = false;
  } else {
    if (DEBUG_CLOCKTRIG) Serial.printf("clock number %i on to mapped %i, beat number %i, count %i\r\n", clock_number, clock_map[clock_number], current_beat, should_send_clock[clock_number]);
    for (int i = 0 ; i < 4 ; i++) {
      digitalWrite( CLOCK_OUT_START_PIN + i, bitRead(clock_map[clock_number], i) );
    }
    clock_on = true;
  }
}

// should be called faster than every tick so that can process the triggers
void process_clock_triggers(unsigned long received_ticks) {

  //last_clock_ticked = received_ticks;
  if (last_clock_trigger_ticked!=received_ticks) {

    // send pulse on every beat
    if (!should_send_clock[CLOCK_BEAT] && is_bpm_on_beat) {
      last_clock_trigger_ticked = received_ticks;
      should_send_clock[CLOCK_BEAT] = CLOCK_LOOP_COUNT;
    }
    // send pulse on every upbeat
    if (!should_send_clock[CLOCK_BEAT_ALT] && is_bpm_on_beat && (current_beat==1 || current_beat==3)) {
      last_clock_trigger_ticked = received_ticks;
      if (DEBUG_CLOCKTRIG) Serial.printf("setting CLOCK_BEAT_ALT to %i, was already %i\r\n", should_send_clock[CLOCK_BEAT_ALT], CLOCK_LOOP_COUNT);
      should_send_clock[CLOCK_BEAT_ALT] = CLOCK_LOOP_COUNT;
    }
    // send pulse on every bar
    if (!should_send_clock[CLOCK_BAR] && is_bpm_on_beat && is_bpm_on_bar) {
      last_clock_trigger_ticked = received_ticks;
      should_send_clock[CLOCK_BAR] = CLOCK_LOOP_COUNT;
    }
    // send pulse on every phrase
    if (!should_send_clock[CLOCK_PHRASE] && is_bpm_on_beat && is_bpm_on_phrase) {
      last_clock_trigger_ticked = received_ticks;
      should_send_clock[CLOCK_PHRASE] = CLOCK_LOOP_COUNT;
    }
    
    /*  // for testing - trigger-per-beat-number
    if (is_bpm_on_beat && should_send_clock[current_beat]==0) {
      should_send_clock[current_beat] = CLOCK_LOOP_COUNT;
      last_clock_trigger_ticked = received_ticks;
    }*/
  }
  
  for (int i = 0 ; i < CLOCK_COUNT ; i++) {
    if (should_send_clock[i]>0) {
      /*if (!clock_on)*/ 
      if (should_send_clock[i]==CLOCK_LOOP_COUNT)   // only set arduino pins on first signal to trigger this output
        clock_output (i, true);
      should_send_clock[i] -= millis() - last_beat_clock_millis;  // decrement time left by delta
      if (should_send_clock[i]<0) should_send_clock[i] = 0;
      break;                                        // only one clock at a time, process in order 0..3
    }
  }
  last_beat_clock_millis = millis();

  // turn off outputs when they're all done
  //if (received_ticks > last_clock_trigger_ticked + 15) {
    if (clock_on && should_send_clock[0]==0 && should_send_clock[1]==0 && should_send_clock[2]==0 && should_send_clock[3]==0) {
      if (DEBUG_CLOCKTRIG) Serial.println("=== all clocks off");
      clock_output (0, false); 
    }
  //}  
}


#endif

#endif

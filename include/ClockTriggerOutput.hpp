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

#define CLOCK_LOOP_COUNT  5  // how many ms each trigger should stay HIGH before moving onto next trigger or going LOW

extern const byte clock_map[];  // this is the one, for some reason?  doesn't make sense, have i got my pins swapped somewhere?
//const static byte clock_map[] = { 1, 2, 6, 11 };
//const static byte clock_map[] = { 1, 2, 6, 10 };
//const static byte clock_map[] = { 2, 6, 4, 10 };

extern int should_send_clock[4]; // keep track of whether an output should be active + how long its been HIGH

extern bool clock_on;
extern unsigned long last_clock_trigger_ticked;
extern unsigned long last_beat_clock_millis;

void initialise_clock_outputs();

void clock_output (byte clock_number, bool on);

// should be called faster than every tick so that can process the triggers
void process_clock_triggers(unsigned long received_ticks);

#endif

#endif

#include "Config.h"

#include "MidiSetup.hpp"

#include "BPM.hpp"

#include "MidiEcho.h"
#include "MidiInput.hpp"
#include "MidiOutput.hpp"
#include "UI.h"

#include "Drums.h"
#include "Envelopes.h"

#include "Euclidian.h"
#include "Pixels.h"

#include "Harmony.hpp"

#ifdef ENABLE_EEPROM
#include "Eeprom.h"
#endif

// GLOBALS

// for demo mode
int last_played_trigger = -1;

// for handling clock ---------------------------------------------------------
// At 120 BPM, 24 clock ticks will last 0.02083 seconds. = 200ms
float estimated_ticks_per_ms = 0.1f;  // initial estimated speed
unsigned long time_last; // last time main loop was run, for calculating elapsed time

//float ticks = 0;  // store ticks as float, so can update by fractional ticks

#include "BPM.hpp"

#ifdef ENABLE_SCREEN

#ifdef ENABLE_SCREEN_ADA
#include "Screen_ada.hpp"
#endif

/*#ifdef ENABLE_SCREEN_LCDGFX
#include "Screen_lcdgfx.hpp"
#else
#include "Screen_snowflakes.hpp"
#endif*/
long last_updated_screen_at = 0;

#endif

// -----------------------------------------------------------------------------

void setup() {
  //randomSeed(analogRead(1));

  delay(500); // give half a second grace to allow for programming

  Serial.begin(115200);   // usb serial debug port
  unsigned long started_millis = millis();
  while (!Serial && millis() < started_millis+500 ) {};  // wait for serial to become available or timeout after half a second

  Serial.println("---> Bambleweeny57 starting up! <c> doctea/The Tyrell Corporation 2020+ <---");

#ifdef ENABLE_SCREEN
  initialise_screen();
#endif

  initialise_pitch_for_triggers();

#ifdef ENABLE_CLOCK_TRIGGER
  initialise_clock_outputs();
#endif

#ifdef ENABLE_EEPROM
  initialise_eeprom();
#endif

  setup_midi();

#ifdef ENABLE_PIXELS
  setup_pixels();
#endif

#ifdef ENABLE_BUTTONS
  setup_buttons();
#endif

  bpm_reset_clock();

  //initialise_harmony();
  initialise_euclidian();
  initialise_envelopes();

  //NOISY_DEBUG(1000, 1);

  kill_notes();
  kill_envelopes();

  debug_pitch_for_trigger();

  Serial.println("---> Bambleweeny57 setup done! <---");

  harmony.debug_inversions();
  
}

void loop() {
  process_midi();

#ifdef ENABLE_BUTTONS
  update_buttons();
#endif

  //unsigned long now = clock_millis();
  unsigned long now = bpm_clock();
  unsigned long now_ms = millis();
  unsigned long delta_ms = now_ms - time_last;
  //Serial.print("now is "); Serial.println(now);
  
  if (demo_mode==MODE_EUCLIDIAN || demo_mode==MODE_EUCLIDIAN_MUTATION) {
    //if (now%10) Serial.printf("demo_mode 1 looped at tick %i\r\n", now);
    mutate_enabled = demo_mode==MODE_EUCLIDIAN_MUTATION;
       
    process_euclidian(now);
  } else if (demo_mode==MODE_RANDOM) {
    //Serial.printf("looping in demo_mode = %i\r\n", demo_mode);
    if (is_bpm_on_step && random(0,5000)<10) {
      //Serial.printf("should trigger random!\r\n");
      if (last_played_trigger>-1) {
        //Serial.printf("noteoff = %i\r\n", last_played_pitch);
        douse_trigger(last_played_trigger, 0, true);
        last_played_trigger = -1;
      } else {
        last_played_trigger = random(0,NUM_TRIGGERS+NUM_ENVELOPES+NUM_MIDI_OUTS);
        //Serial.printf("noteon = %i\r\n", last_played_pitch);
        fire_trigger(last_played_trigger,random(1,127), true);
      }
    }
  }

  // update envelopes by time elapsed
  process_envelopes(now);

#ifdef ENABLE_PIXELS
  update_pixels(now_ms);
#endif

#ifdef ENABLE_SCREEN
  if (now_ms - last_updated_screen_at >= PIXEL_REFRESH * 2) {
    //Serial.printf("updating screen - last_updated_screen_at is %i, now_ms is %i\n", last_updated_screen_at, now_ms);
    last_updated_screen_at = now_ms;
    screen_update();
    Serial.printf("updated screen in %ims\n", millis() - last_updated_screen_at);
  }
#endif

  time_last = now_ms;
}

#include "Config.h"

#ifdef ENABLE_PIXELS
#ifdef ENABLE_PIXELS_FASTLED

#include "Drums.h"
#include "Envelopes.h"
#include "UI.h"
#include "BPM.hpp"
#include "Euclidian.h"
#include "MidiInput.hpp"

#define ENABLE_PIXEL_POSITION
//#define NO_IDLE_PIXEL_POSITION // unused
#define REVERSE_LEDS           // if pixel strips should be reversed
#define NO_ACTIVE_PIXEL_POSITION

#define IDLE_PIXEL_TIMEOUT IDLE_TIMEOUT // five second timeout

#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 16 //128

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 9
//#define CLOCK_PIN 13

long last_updated_pixels_at = 0;

// Define the array of leds
CRGB leds[NUM_LEDS];

void setup_pixels() {
  // todo: different modes?
  LEDS.addLeds<NEOPIXEL,DATA_PIN>(leds,NUM_LEDS);
  LEDS.setBrightness(255);

  FastLED.clear(true);

  int hue = 0 ;
  for(int i = 0; i < NUM_LEDS; i++) {   
    // fade everything out

    // let's set an led value
    leds[i] = CHSV(hue++,255,255);

    // now, let's first 20 leds to the top 20 leds, 
    delay(100);
    FastLED.show();
  }  
}

/*
 * hmmmm, flstudio doesnt send song position with every beat, and going off beats seems to drift very quickly(!), so this won't be any use..
 void update_pixels_position() { //unsigned int beats) {
  // beats = number of 16th notes since song start
  // 4 = 1 beat
  //beats = beats / 4;

  CRGB colour;

  if (millis() - last_tick_at < 250) {
    colour = CRGB::Red;
  } else {
    colour = CRGB::Green;
  }

  //int beats = (((int)ticks)/PPQN) % NUM_LEDS;
  int beats = song_position; 

  for (int i = 0 ; i < NUM_LEDS; i++) {
      if (beats==i) {
        leds[i] = colour; //CHSV(255, 255, 255);
      } else {
        leds[i] = CRGB::Black; //CHSV(0, 0, 0);
      }
  }
  FastLED.show();
}*/


void update_pixels(unsigned long now_ms) {
  if (now_ms - last_updated_pixels_at >= PIXEL_REFRESH) {
    last_updated_pixels_at = now_ms;
    update_pixels_triggers();
  }
} 

#define STRIP_LENGTH  (NUM_LEDS/2)

#define PIX_NONE      0
#define PIX_TRIGGER   1
#define PIX_ENVELOPE  2

// 0-4 inclusive: first 5 triggers, kick -> crash
// 10-16 inclusive: second 6 triggers, tamb -> ch
// leaving 5, 6, 7, 8, 9 for envelopes
//#if MUSO_MODE==MUSO_MODE_0B
#define TRIGGER_BANK_1_SIZE   5
#define TRIGGER_BANK_2_SIZE   6
/*#elif MUSO_MODE==MUSO_MODE_2B
#define TRIGGER_BANK_1_SIZE   3
#define TRIGGER_BANK_2_SIZE   4
#endif*/

// determine which trigger this pixel address is for
int get_trigger_for_pixel(int p) {
  if (p < TRIGGER_BANK_1_SIZE) {  // first bank
    return p;
  } else if (p >= NUM_LEDS - TRIGGER_BANK_2_SIZE) { // second bank  = 10
    // we want a number between 5-11 inclusive
    p = NUM_TRIGGERS - (TRIGGER_BANK_2_SIZE - (NUM_LEDS-p)) - 1;
    return p;
  } else {
    return -1;
  }
}

// determine which envelope this pixel address is for
int get_envelope_for_pixel(int i) {
  if (i >= TRIGGER_BANK_1_SIZE && i < TRIGGER_BANK_1_SIZE + NUM_ENVELOPES) { // envelopes
    return NUM_ENVELOPES - (i - TRIGGER_BANK_1_SIZE) - 1; // offset and invert
  } else {
    return -1;
  }
}

static CRGB trigger_colours[NUM_TRIGGERS] = {
  CRGB::Red,
  CRGB::OrangeRed,
  CRGB::Orange,
  CRGB::Yellow,
  CRGB::YellowGreen,
  CRGB::Green,
  CRGB::Aqua,
  CRGB::Blue,
  CRGB::BlueViolet,
  CRGB::Violet,
  CRGB::DarkViolet
};

void update_pixels_triggers() {
    bool changed = false;
    for (int i = 0 ; i < NUM_LEDS ; i++) {
      int pixel_type = PIX_NONE; //PIX_TRIGGER;
      bool active = false;

#ifdef REVERSE_LEDS
      int p = NUM_LEDS - i - 1;
      if (p >= STRIP_LENGTH) {
        // reverse for the second strip
        p = NUM_LEDS - (p % STRIP_LENGTH) - 1;
      } else {
        p = STRIP_LENGTH - p - 1;
      }
#else
      int p = i;
      if (p >= STRIP_LENGTH) {
        // reverse for the second strip
        p = STRIP_LENGTH + (p % STRIP_LENGTH);
      } 
#endif

      // determine if this pixel is active and what type, and set 't' to the trigger or envelope
      int t = get_trigger_for_pixel(i);
      if (t>=0) {
        pixel_type = PIX_TRIGGER;
        active = trigger_status[t];
      } else {
        t = get_envelope_for_pixel(i);
        if (t>=0) {
          pixel_type = PIX_ENVELOPE;
          //active = envelopes[t].stage != OFF;
          active = envelopes[t].last_sent_actual_lvl>0; // != OFF;
        }
      }

      // pick the colour to use for this pixel based on if we are active and the type
      CRGB colour;
      if (active) {
        if (pixel_type==PIX_TRIGGER) {
          // handling a trigger pixel that is on
          //colour = CRGB::Red;
          colour = trigger_colours[t];
        } else if (pixel_type==PIX_ENVELOPE) {
          // handling an envelope pixel that is active
          if (envelopes[t].stage==ATTACK) {
            colour = CRGB::Red;
          } else if (envelopes[i].stage==HOLD) {
            colour = CRGB::White;
          } else if (envelopes[t].stage==DECAY) {
            colour = CRGB::Yellow;
          } else if (envelopes[t].stage==SUSTAIN) {
            colour = CRGB::Orange;
          } else if (envelopes[t].stage==RELEASE) {
            colour = CRGB::Crimson;
          } else {
            colour = CRGB::Green;
          }
          
          //colour.fadeToBlackBy(255.0 * (1.0-((float)envelopes[t].actual_level / (float)envelopes[t].velocity)));// fade by envelope level relative to velocity
          colour.fadeToBlackBy(255.0 * (1.0-((float)envelopes[t].last_sent_actual_lvl/127.0))); // by the actual level....
        }
      } else {
        //colour = CRGB::Black;
        if (pixel_type==PIX_TRIGGER)
          colour = leds[p]/4; //CRGB::Black;
        else 
          colour = CRGB::Black;
      }

#ifdef ENABLE_PIXEL_POSITION
#ifdef NO_ACTIVE_PIXEL_POSITION
      unsigned long now = millis();
      //Serial.printf("  pixels -- now is %u, last_input_at is %u, last_tick_at is %u, IDLE is %u\r\n", now, last_input_at, last_tick_at, IDLE_PIXEL_TIMEOUT);
      if ((now - last_input_at > IDLE_PIXEL_TIMEOUT && now - last_tick_at > IDLE_PIXEL_TIMEOUT)
          && activeNotes==0
      ) {
#endif
        int beats = current_song_position%NUM_LEDS;
        //Serial.printf("  pixels -- i is %i, beats is %i\r\n", i, beats);
        int steps = current_step;
        if ((i==beats && steps%STEPS_PER_BEAT<2)) {  // only display for first tick / (6 = sixteenth note ?)
          if (beats % 4) 
            colour += CRGB::Yellow;
          else
            colour += CRGB::White;
        } /*else {
          colour = CRGB::Black;
        }*/
        if (p==0) {
          //Serial.println("pixel loop");
        }
#ifdef NO_ACTIVE_PIXEL_POSITION
      } /*else {
        Serial.printf("isn't idle?  now is %u, last_input_at is %u, last_tick_at is %u, ", now, last_input_at, last_tick_at);
        Serial.printf("pixel_timeout is %u, activeNotes is %i\r\n", IDLE_PIXEL_TIMEOUT, activeNotes);
      }*/
#endif
#endif

      if (button_pressed_at > now - UI_BUTTON_PRESSED_INDICATOR_INTERVAL) {
        // use i instead of p 
        if (NUM_DEMO_MODES + STRIP_LENGTH - p == (NUM_DEMO_MODES - demo_mode)) { // indicate mode
          colour = CRGB::Blue;
        } else if (p == 0) {
          colour = euclidian_auto_play ? CRGB::Red : CRGB::Green;
        } else {
          colour = ui_last_action==ACTION_RESET_EUCLIDIAN ? CRGB(p*(255/NUM_LEDS), 0, 0) : CRGB::Violet;
        }
      }

      if (leds[p] != colour)
        changed = true;
      leds[p] = colour;
    }
    if (changed)
      FastLED.show();
}



#endif
#endif

#ifdef ENABLE_PIXELS

#define ENABLE_PIXEL_POSITION
//#define NO_IDLE_PIXEL_POSITION // unused
//#define REVERSE_LEDS           // if pixel strips should be reversed
#define NO_ACTIVE_PIXEL_POSITION

#define IDLE_PIXEL_TIMEOUT 5000 // five second timeout

#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 16

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 7
//#define CLOCK_PIN 13

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


void update_pixels() {
  update_pixels_triggers();
} 

#define STRIP_LENGTH  (NUM_LEDS/2)

#define PIX_NONE      0
#define PIX_TRIGGER   1
#define PIX_ENVELOPE  2

// 0-4 inclusive: first 5 triggers, kick -> crash
// 10-16 inclusive: second 6 triggers, tamb -> ch
// leaving 6, 7, 8 for envelopes?

#define TRIGGER_BANK_1_SIZE   5
#define TRIGGER_BANK_2_SIZE   6

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

int get_envelope_for_pixel(int i) {
  if (i >= TRIGGER_BANK_1_SIZE && i < TRIGGER_BANK_1_SIZE + NUM_ENVELOPES) { // envelopes
    return NUM_ENVELOPES - (i - TRIGGER_BANK_1_SIZE) - 1; // offset and invert
  } else {
    return -1;
  }
}

void update_pixels_triggers() {
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
      } /*else {
        p = STRIP_LENGTH - p - 1;
      }*/
#endif

      //int p = i;
      
      /*if (i >= STRIP_LENGTH) {
        //p = NUM_LEDS/2 + (NUM_LEDS - i);
        //p = (NUM_LEDS - (i%(NUM_LEDS/2)));
        //p = NUM_LEDS - ((NUM_LEDS/2)%i);
        // so we want i=8 to become p=16 = 
        p = NUM_LEDS - (i%STRIP_LENGTH) - 1;
        // 9 to become 15 = 16/2 + 9
        // 10 to become 14    = NUM_LEDS - 6 (NUM_LEDS 16 - i = 6)
        // 11 to become 13
        // 12 to become 12
        // 13 to become 11  16 - (16 - (13
        // 16 to become 8   i - NUM_LEDS + (NUM_LEDS/2)
        //p = NUM_LEDS - (NUM_LEDS - i); // + NUM_LEDS/2;
      }*/

      int t = get_trigger_for_pixel(i);
      if (t>=0) {
        pixel_type = PIX_TRIGGER;
        active = trigger_status[t];
      } else {
        t = get_envelope_for_pixel(i);
        if (t>=0) {
          pixel_type = PIX_ENVELOPE;
          active = envelopes[t].stage != OFF;
        }
      }
      /*} else {
        active = false;
      }*/

      /*if (i < NUM_TRIGGERS) {
        pixel_type = PIX_TRIGGER;
        active = trigger_status[i];
      } else if (i >= NUM_TRIGGERS && i - NUM_TRIGGERS < NUM_ENVELOPES) {
        pixel_type = PIX_ENVELOPE;
        active = envelopes[i - NUM_TRIGGERS].stage != OFF;
      }*/
      
      CRGB colour;
      if (active) {
        if (pixel_type==PIX_TRIGGER) {
          colour = CRGB::Red;
        } else if (pixel_type==PIX_ENVELOPE) {
          if (envelopes[t].stage==RELEASE) {
            colour = CRGB::Aqua;
          } else {
            colour = CRGB::Green;
          }
          colour.fadeToBlackBy(255.0 * (1.0-((float)envelopes[t].actual_level / (float)envelopes[t].velocity)));
        }
        //leds[i] = colour;
      } else {
        //colour = CRGB::Black;
        colour = leds[p]/4; //CRGB::Black;
      }

#ifdef ENABLE_PIXEL_POSITION
#ifdef NO_ACTIVE_PIXEL_POSITION
      if (millis() - last_input_at > IDLE_PIXEL_TIMEOUT
      ) {
#endif
        int beats;
        if (millis() - last_tick_at > 250) {
          beats = (int)((clock_millis()*estimated_ticks_per_ms)/PPQN) % NUM_LEDS;  // runs based on last estimated clock
        } else {
          beats = (((int)ticks)/PPQN) % NUM_LEDS;  //only runs when real clock is running
        }
        if ((i==beats && (int)ticks%PPQN<3)) {  // only display for first tick / (6 = sixteenth note ?)
          if (beats % 4) 
            colour += CRGB::Blue;
          else
            colour += CRGB::White;
        } /*else {
          colour = CRGB::Black;
        }*/
#ifdef NO_ACTIVE_PIXEL_POSITION
      }
#endif
#endif
      
      leds[p] = colour;
    }
    FastLED.show();
}



#endif

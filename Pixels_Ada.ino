// experimental Adafruit_NeoPixels support (basically works but colours are off)

#ifdef ENABLE_PIXELS_ADA

#define CRGB_T uint32_t

#define CRGB__Red(v)     pixels.ColorHSV(30,255,(int)v)
#define CRGB__Yellow(v)  pixels.ColorHSV(60,255,(int)v)
#define CRGB__Green(v)   pixels.ColorHSV(90,255,(int)v)
#define CRGB__Aqua(v)    pixels.ColorHSV(120,255,(int)v)
#define CRGB__Blue(v)    pixels.ColorHSV(160,255,(int)v)
#define CRGB__White(v)    pixels.ColorHSV(0,0,(int)v)
#define CHSV(h,s,v)   pixels.ColorHSV(h,s,v)


#define ENABLE_PIXEL_POSITION
//#define NO_IDLE_PIXEL_POSITION // unused
//#define REVERSE_LEDS           // if pixel strips should be reversed
#define NO_ACTIVE_PIXEL_POSITION

#define IDLE_PIXEL_TIMEOUT 5000 // five second timeout

#include <Adafruit_NeoPixel.h>


// How many leds in your strip?
#define NUM_LEDS 16 //128

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN D9
//#define CLOCK_PIN 13

Adafruit_NeoPixel pixels(NUM_LEDS, DATA_PIN, NEO_RGB + NEO_KHZ800);

// Define the array of leds
CRGB_T leds[NUM_LEDS];

void setup_pixels() {
  // todo: different modes?
  pixels.begin();

  pixels.clear();
  pixels.show();

  int hue = 0 ;
  for(int i = 0; i < NUM_LEDS; i++) {   
    // fade everything out

    // let's set an led value
    leds[i] = CHSV(hue++,255,255);
    pixels.setPixelColor(i, leds[i]);

    // now, let's first 20 leds to the top 20 leds, 
    delay(100);
    pixels.show();
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
    colour = CRGB__Red;
  } else {
    colour = CRGB__Green;
  }

  //int beats = (((int)ticks)/PPQN) % NUM_LEDS;
  int beats = song_position; 

  for (int i = 0 ; i < NUM_LEDS; i++) {
      if (beats==i) {
        leds[i] = colour; //CHSV(255, 255, 255);
      } else {
        leds[i] = CRGB__Black; //CHSV(0, 0, 0);
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
#define TRIGGER_BANK_1_SIZE   5
#define TRIGGER_BANK_2_SIZE   6

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
          active = envelopes[t].stage != OFF;
        }
      }

      // pick the colour to use for this pixel based on if we are active and the type
      CRGB_T colour;
      if (active) {
        if (pixel_type==PIX_TRIGGER) {
          // handling a trigger pixel that is on
          colour = CRGB__Red(255.0 * (1.0-((float)envelopes[t].actual_level / (float)envelopes[t].velocity)));
        } else if (pixel_type==PIX_ENVELOPE) {
          // handling an envelope pixel that is active
          if (envelopes[t].stage==ATTACK) {
            colour = CRGB__Red(255.0 * (1.0-((float)envelopes[t].actual_level / (float)envelopes[t].velocity)));
          } else if (envelopes[t].stage==DECAY) {
            colour = CRGB__Yellow(255.0 * (1.0-((float)envelopes[t].actual_level / (float)envelopes[t].velocity)));
          } else if (envelopes[t].stage==RELEASE) {
            colour = CRGB__Aqua(255.0 * (1.0-((float)envelopes[t].actual_level / (float)envelopes[t].velocity)));
          } else {
            colour = CRGB__Green(255.0 * (1.0-((float)envelopes[t].actual_level / (float)envelopes[t].velocity)));
          }
          // fade by envelope level relative to velocity
          //colour.fadeToBlackBy(255.0 * (1.0-((float)envelopes[t].actual_level / (float)envelopes[t].velocity)));
        }
      } else {
        //colour = CRGB__Black;
        colour = leds[p]/4; //CRGB__Black;
      }

#ifdef ENABLE_PIXEL_POSITION
#ifdef NO_ACTIVE_PIXEL_POSITION
      if (millis() - last_input_at > IDLE_PIXEL_TIMEOUT && millis() - last_tick_at > IDLE_PIXEL_TIMEOUT
          && activeNotes==0
      ) {
#endif
        int beats;
        int t_ticks;
        if (millis() - last_tick_at > 250) {
          beats = (int)((clock_millis()*estimated_ticks_per_ms)/PPQN) % NUM_LEDS;  // runs based on last estimated clock
        } else {
          beats = (((int)ticks)/PPQN) % NUM_LEDS;  //only runs when real clock is running
        }
        t_ticks = clock_millis()*estimated_ticks_per_ms;
        if ((i==beats && (int)t_ticks%PPQN<6)) {  // only display for first tick / (6 = sixteenth note ?)
          if (beats % 4) 
            colour += CRGB__Blue(255);
          else
            colour += CRGB__White(255);
        } /*else {
          colour = CRGB__Black;
        }*/
#ifdef NO_ACTIVE_PIXEL_POSITION
      }
#endif
#endif

      if (leds[p] != colour)
        changed = true;
      leds[p] = colour;
      pixels.setPixelColor(p, colour);
    }
    if (changed)
      pixels.show();
}



#endif

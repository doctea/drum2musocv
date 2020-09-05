#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 8

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
  LEDS.setBrightness(84);

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

void kill_notes() {
  // turn off the triggers
  for (int i = 0 ; i < NUM_TRIGGERS ; i++) {
    trigger_status[i] = TRIGGER_IS_OFF;
  }
}

void update_pixels() {
  update_pixels_triggers();
}

void update_pixels_triggers() {
  /*for (int i = 0 ; i < NUM_TRIGGERS ; i++) {
    int bank = i / NUM_LEDS;
    int pix = i % NUM_LEDS;  

    CRGB colour = bank==0 ? CRGB::Blue : CRGB::Yellow;
    leds[pix] = trigger_status[i] ? colour : CRGB::Black;
    //leds[NUM_LEDS-1] = colour;
  } 
  FastLED.show();*/

  for (int i = 0 ; i < NUM_LEDS ; i++) {
    /*CRGB colour_bank_1 = trigger_status[i] ? CRGB::Blue : CRGB::Black;  // cover triggers 1-8
    // trigger number for second bank of LEDs - triggers 9 - 11
    
    CRGB colour_bank_2;
    if (trig < NUM_TRIGGERS) {
      colour_bank_2 = trigger_status[trig] ? CRGB::Green : CRGB::Black;
    } else {
      colour_bank_2 = CRGB::Black;
    }
    CRGB colour = blend(colour_bank_1, colour_bank_2, 10.0);
    */
    leds[i] = CRGB::Black;
    
    bool layer_1 = false;
    bool layer_2 = false;
    
    CRGB colour;
    if (trigger_status[i]) {
      layer_1 = true;
    }

    NUMBER_DEBUG(9, 1, NUM_TRIGGERS);
    int trig = NUM_LEDS + i;  // second layer trigger number  // 8 + 3
    if (trig < NUM_TRIGGERS && trigger_status[trig]) {
      layer_2 = true;
      //NUMBER_DEBUG(1, i, trig);
    } else if (trig < NUM_TRIGGERS) {
      //NUMBER_DEBUG(8, i, trig);
    } 

    if (layer_1 && layer_2) {
      colour = CRGB::Red;
    } else if (layer_1 && !layer_2) { // only first layer
      colour = CRGB::Blue;
    } else if (layer_2 && !layer_1) { // only second layer
      colour = CRGB::Green;
    } else if (!layer_1 && !layer_2) {  // no layers
      colour = leds[i]/8; //CRGB::Black;
    }
    
    leds[i] = colour;
  }
  FastLED.show();
}

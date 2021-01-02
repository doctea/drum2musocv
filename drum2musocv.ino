// config settings
//#define TEST_TRIGGERS
#define ENABLE_PIXELS
//#define ENABLE_PIXELS_ADA // choose this or ENABLE_PIXELS_FASTLED
#define ENABLE_PIXELS_FASTLED
#define PIXEL_REFRESH   50  // number of milliseconds to wait between updating pixels (if enabled ofc)
#define BUTTON_PIN A0

//#define ENABLE_EEPROM     // untested, not available on SAMD platforms
#define ENABLE_MIDI_ECHO

#define IDLE_TIMEOUT 5000 // five second timeout before going into 'idle mode' ie running own clock and displaying 'screensaver'

#define USB_NATIVE  // enable native usb support
#define SEEEDUINO // enable seeduino cortex m0+ compatibility for FastLED (see Pixels.ino)


#include "MidiInput.hpp"

#include "Drums.h"
#include "Envelopes.h"

#include "Euclidian.h"

#ifdef BUTTON_PIN
#include <DebounceEvent.h>
#endif

#ifdef ENABLE_EEPROM
#include "Eeprom.h"
#endif

// GLOBALS

// for demo mode
short demo_mode = 0;
int last_played_pitch = 0;

// for handling clock ---------------------------------------------------------
// At 120 BPM, 24 clock ticks will last 0.02083 seconds. = 200ms
float estimated_ticks_per_ms = 0.1f;  // initial estimated speed
unsigned long time_last; // last time main loop was run, for calculating elapsed time

//float ticks = 0;  // store ticks as float, so can update by fractional ticks

#include "BPM.hpp"


// override default midi library settings, so that notes with velocity 0 aren't treated as note-offs
// however this doesn't work like i need it to
/*struct MySettings : public midi::DefaultSettings {
    static const long BaudRate = 31250;
    const bool HandleNullVelocityNoteOnAsNoteOff = false;
};
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, MySettings);*/


// -----------------------------------------------------------------------------

byte convert_drum_pitch(byte pitch) {
  // in mode 0x0b there are 11 triggers available and a pitch out
  byte p;
  if (pitch >= GM_NOTE_MINIMUM && pitch <= GM_NOTE_MAXIMUM) {   // only process notes within GM drumkit range
    p = pitch;
    switch (pitch) {
      // right-hand column (triggers 1-5 running down)
      case GM_NOTE_ELECTRIC_BASS_DRUM:  p = MUSO_NOTE_GATE_1;   break; //Electric Bass Drum - C5 72
      case GM_NOTE_SIDE_STICK:          p = MUSO_NOTE_GATE_2;   break; //Side Stick - C#5/Db5 73
      case GM_NOTE_HAND_CLAP:           p = MUSO_NOTE_GATE_3;   break; //Hand Clap - D5 74
      case GM_NOTE_ELECTRIC_SNARE:      p = MUSO_NOTE_GATE_4;   break; //Electric Snare - D#5/Eb5 75
      case GM_NOTE_CRASH_CYMBAL_1:      p = MUSO_NOTE_GATE_5;   break; //Crash Cymbal 1 - F#5/Gb5 78
      
      // left-hand column (triggers 11-6 running down)
      case GM_NOTE_CLOSED_HI_HAT:       p = MUSO_NOTE_GATE_11;  break; //Closed Hi-hat - E5 76
      case GM_NOTE_OPEN_HI_HAT:         p = MUSO_NOTE_GATE_10;  break; //77 //Open Hi-hat - F5
      case GM_NOTE_PEDAL_HI_HAT:        p = MUSO_NOTE_GATE_9;   break; //Pedal Hi-hat - A#5/Bb5 82

      case GM_NOTE_LOW_TOM:             p = MUSO_NOTE_GATE_8;   break; //79 //Low Tom - G5
      case GM_NOTE_HIGH_TOM:            p = MUSO_NOTE_GATE_7;   break; //High Tom - G#5/Ab5 80
      case GM_NOTE_TAMBOURINE:          p = MUSO_NOTE_GATE_6;   break; //Tambourine - A5 81
      //case GM_NOTE_RIDE_BELL:           p = MUSO_GATE_12; break; //Ride Bell - B5 83  // there is no gate 12!
      //default: p = pitch + 12; //itch = 72; break;
      default:                          p = pitch;              break; // pass thru other notes unmodified
    }
  }
  return p;
}



bool process_triggers_for_pitch(byte pitch, byte velocity, bool state) {
  // in the 0x0b midimuso-cv mode, there are 5 CV outputs and a clock output
  // the mapping is currently all hardcoded here and in Drums.h
  // TODO: some way to link envelopes/triggers so as to be able to 'cut by' or choke/release hihats
  byte p;
  int trig = get_trigger_for_pitch(pitch);
  if (trig>NUM_TRIGGERS) {
    update_envelope(trig-NUM_TRIGGERS, velocity, state);
    return true;
  }
  return false;
  /*switch (pitch) {
    //case GM_NOTE_PEDAL_HI_HAT:  // TODO: figure out how to link pedal hihat with envelopes so as to choke?  would need an envelope dedicated to the open hats i guess...
    //  return true 
    //  break;
    case GM_NOTE_CRASH_CYMBAL_2:  // cymbal crash 2
      // trigger envelope
      update_envelope(ENV_CRASH, velocity, state);
      return true;
      break;
    case GM_NOTE_SPLASH_CYMBAL:  // splash cymbal
      update_envelope(ENV_SPLASH, velocity, state);
      return true;
      break;
      //case GM_NOTE_:  // TODO: add more envelopes
      //  break;
    case GM_NOTE_VIBRA_SLAP:    
      update_envelope(ENV_WOBBLY, velocity, state);
      return true;
    case GM_NOTE_RIDE_BELL:
      update_envelope(ENV_RIDE_BELL, velocity, state);
      return true;
    case GM_NOTE_RIDE_CYMBAL_1:
      update_envelope(ENV_RIDE_CYMBAL, velocity, state);
      return true;
  }
  return false;*/
}



#ifdef ENABLE_PIXELS
long last_updated_pixels_at = 0;
#endif


// -----------------------------------------------------------------------------

void setup() {
  //randomSeed(analogRead(1));

  delay(500); // give half a second grace to allow for programming

  initialise_pitch_for_triggers();

#ifdef ENABLE_EEPROM
  initialise_eeprom();
#endif

  setup_midi();

#ifdef ENABLE_PIXELS
  setup_pixels();
#endif

#ifdef BUTTON_PIN
  //pinMode(BUTTON_PIN, INPUT_PULLUP);
  //button.setCallback(handleButtonPressed);
  //DebounceEvent button = DebounceEvent(BUTTON_PIN, handleButtonPressed, BUTTON_PUSHBUTTON);// | BUTTON_DEFAULT_LOW );// | BUTTON_SET_PULLUP);  // may need to change these if using different circuit
  setup_buttons();
#endif

  bpm_reset_clock();

  initialise_euclidian();

  initialise_envelopes();

  //NOISY_DEBUG(1000, 1);

  kill_envelopes();
}

void loop() {
  // Call MIDI.read the fastest you can for real-time performance.
  MIDIIN.read();

#ifdef BUTTON_PIN
  update_buttons();
#endif

  // There is no need to check if there are messages incoming
  // if they are bound to a Callback function.
  // The attached method will be called automatically
  // when the corresponding message has been received.

  //unsigned long now = clock_millis();
  unsigned long now = bpm_clock();
  unsigned long now_ms = millis();
  unsigned long delta_ms = now_ms - time_last;
  //Serial.print("now is "); Serial.println(now);
  
  if (demo_mode==1 || demo_mode==2) {
    //if (now%10) Serial.printf("demo_mode 1 looped at tick %i\r\n", now);
    mutate_enabled = demo_mode==2;
       
    process_euclidian(now);
  } else if (demo_mode==3) {
    //Serial.printf("looping in demo_mode = %i"\r, demo_mode);
    if (random(0,5000)<10) {
      if (last_played_pitch>0) {
        //Serial.printf("noteoff = %i\r\n", last_played_pitch);
        douse_trigger(MUSO_NOTE_MINIMUM+last_played_pitch, 0, true);
        last_played_pitch = 0;
      } else {
        last_played_pitch = random(0,NUM_TRIGGERS); //_NOTE_MINIMUM+1,GM_NOTE_MAXIMUM);
        //Serial.printf("noteon = %i\r\n", last_played_pitch);
        fire_trigger(MUSO_NOTE_MINIMUM+last_played_pitch,random(1,127), true);
      }
    }
  }

  // update envelopes by time elapsed
  process_envelopes(now);

#ifdef ENABLE_PIXELS
  if (last_updated_pixels_at - now_ms >= PIXEL_REFRESH) {
    last_updated_pixels_at = now_ms;
    update_pixels();
  }
#endif

  time_last = now_ms;
}

// config settings
//#define TEST_TRIGGERS
#define ENABLE_PIXELS
//#define ENABLE_PIXELS_ADA // choose this or ENABLE_PIXELS_FASTLED
#define ENABLE_PIXELS_FASTLED

#define ENABLE_BUTTONS

#define PIXEL_REFRESH   50  // number of milliseconds to wait between updating pixels (if enabled ofc)

//#define ENABLE_EEPROM     // untested, not available on SAMD platforms
#define ENABLE_MIDI_ECHO

#define IDLE_TIMEOUT 5000 // five second timeout before going into 'idle mode' ie running own clock and displaying 'screensaver'

#define USB_NATIVE  // enable native usb support
#define SEEEDUINO // enable seeduino cortex m0+ compatibility for FastLED (see Pixels.ino)


#include "MidiInput.hpp"
#include "UI.h"

#include "Drums.h"
#include "Envelopes.h"

#include "Euclidian.h"


#ifdef ENABLE_EEPROM
#include "Eeprom.h"
#endif

// GLOBALS

// for demo mode
short demo_mode = 0;
int last_played_pitch = -1;

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



#ifdef ENABLE_PIXELS
long last_updated_pixels_at = 0;
#endif


// -----------------------------------------------------------------------------

void setup() {
  //randomSeed(analogRead(1));

  delay(500); // give half a second grace to allow for programming

  Serial.println("---> Bambleweeny57 starting up! <c> doctea/The Tyrell Corporation 2020+ <---");

  initialise_pitch_for_triggers();

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

  initialise_euclidian();

  initialise_envelopes();

  //NOISY_DEBUG(1000, 1);

  kill_envelopes();

  Serial.println("---> Bambleweeny57 setup done! <---");
}

void loop() {
  if (MIDIIN.read()) {
    //Serial.printf("received message from MIDIIN, channel is %i: type is %i, ", MIDIIN.getChannel(), MIDIIN.getType()  );
    //Serial.printf("data1 is %i, data2 is %i\r\n", MIDIIN.getData1(), MIDIIN.getData2() );
    if (MIDIIN.getChannel()==MIDI_CHANNEL_NEUTRON_IN) {
      // relay all incoming messages for the Neutron
      MIDIOUT.send(MIDIIN.getType(),
                   MIDIIN.getData1(),
                   MIDIIN.getData2(),
                   MIDI_CHANNEL_NEUTRON_OUT
      );
    }
  }

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
      if (last_played_pitch>-1) {
        //Serial.printf("noteoff = %i\r\n", last_played_pitch);
        douse_trigger(MUSO_NOTE_MINIMUM+last_played_pitch, 0, true);
        last_played_pitch = -1;
      } else {
        last_played_pitch = random(0,NUM_TRIGGERS+NUM_ENVELOPES);
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

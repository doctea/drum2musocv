// config settings
//#define TEST_TRIGGERS
#define ENABLE_PIXELS
//#define ENABLE_PIXELS_ADA // choose this or ENABLE_PIXELS_FASTLED
#define ENABLE_PIXELS_FASTLED
#define PIXEL_REFRESH   50  // number of milliseconds to wait between updating pixels (if enabled ofc)
#define BUTTON_PIN A0

#define USB_NATIVE  // enable native usb support
#define SEEEDUINO // enable seeduino cortex m0+ compatibility for FastLED (see Pixels.ino)

#ifdef USB_NATIVE  // use native usb version, eg for seeduino or (presumably) other boards with Native USB support

#include <USB-MIDI.h> 
typedef USBMIDI_NAMESPACE::usbMidiTransport __umt;
typedef MIDI_NAMESPACE::MidiInterface<__umt> __ss;
__umt usbMIDI(0); // cableNr
__ss MIDICoreUSB((__umt&)usbMIDI);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, midiB);

#define MIDIOUT midiB
#define MIDIIN  MIDICoreUSB

//#else              // arduino uno / serial midi version (for USBMidiKlik)
#endif

//#include <MIDI.h>

#ifndef USB_NATIVE
MIDI_CREATE_DEFAULT_INSTANCE();
#define MIDIOUT MIDI
#define MIDIIN  MIDI

#endif

//TODO: make these CC values sensible and map them in FL
#define CC_SYNC_RATIO   110

#define PPQN  24  // midi clock ticks per quarter-note

#include "drums.h"
#include "Euclidian.h"

#ifdef BUTTON_PIN
#include <ButtonDebounce.h>
ButtonDebounce button(BUTTON_PIN, 250);
#endif
// GLOBALS



byte activeNotes = 0;

// for demo mode
bool demo_mode = false;
int last_played_pitch = 0;

// for handling clock ---------------------------------------------------------
float estimated_ticks_per_ms = 0.1f;  // initial estimated speed
unsigned long time_last; // last time main loop was run, for calculating elapsed time

float ticks = 0;  // store ticks as float, so can update by fractional ticks
unsigned long last_tick_at = 0;
unsigned long last_input_at = 0;

unsigned int song_position;

byte cc_value_sync_modifier = 127;  // initial global clock sync modifier

// tracking what triggers are currently active, for the sake of pixel output 
int trigger_status[NUM_TRIGGERS];

// override default midi library settings, so that notes with velocity 0 aren't treated as note-offs
// however this doesn't work like i need it to
/*struct MySettings : public midi::DefaultSettings {
    static const long BaudRate = 31250;
    const bool HandleNullVelocityNoteOnAsNoteOff = false;
};
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, MySettings);*/

enum envelope_types : byte {
  ENV_CRASH = 0,
  ENV_SPLASH = 1,
  ENV_WOBBLY = 2,
  ENV_RIDE_BELL = 3,
  ENV_RIDE_CYMBAL = 4
  // TODO: more envelope types...
};
#define NUM_ENVELOPES 5

unsigned long clock_millis() {
  // if external clock is running, use external clock, otherwise use an internal clock based on the last-known speed
  if (millis() - last_tick_at > 100) { // if we haven't received a clock for 100ms, fall back to internal millis clock
    return  (millis() * estimated_ticks_per_ms) * 
            //PPQN *
            ((float)(cc_value_sync_modifier^2)/127.0f);
  }
  return ticks; // * PPQN; // * ((float)(cc_sync_modifier^2)/127.0f);   // TODO: need to experiment to find a good tradeoff between allowing very short and very long stages at all tempos?  tempo-sync this basically?  
}

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

void kill_notes() {
  // forget which triggers are active (doesn't actually send stop notes)
  for (int i = 0 ; i < NUM_TRIGGERS ; i++) {
    trigger_status[i] = TRIGGER_IS_OFF;
  }
}


bool process_triggers_for_pitch(byte pitch, byte velocity, bool state) {
  // in the 0x0b midimuso-cv mode, there are 5 CV outputs and a clock output
  // the mapping is currently all hardcoded here and in Drums.h
  // TODO: some way to link envelopes/triggers so as to be able to 'cut by' or choke/release hihats
  byte p;
  switch (pitch) {
    /*case GM_NOTE_PEDAL_HI_HAT:  // TODO: figure out how to link pedal hihat with envelopes so as to choke?  would need an envelope dedicated to the open hats i guess...
      return true 
      break;*/
    case GM_NOTE_CRASH_CYMBAL_2:  // cymbal crash 2
      // trigger envelope
      update_envelope(ENV_CRASH, velocity, state);
      return true;
      break;
    case GM_NOTE_SPLASH_CYMBAL:  // splash cymbal
      update_envelope(ENV_SPLASH, velocity, state);
      return true;
      break;
      /*case GM_NOTE_:  // TODO: add more envelopes
        break;*/
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
  return false;
}

void handleNoteOn(byte channel, byte pitch, byte velocity) {
  byte p = pitch;
  byte v = velocity;

  if (velocity==0) 
    handleNoteOff(channel, pitch, v);

  activeNotes++;
  if (!process_triggers_for_pitch(p, v, true)) {
    p = convert_drum_pitch(p);
    fire_trigger(p, v);
    /*if (p>=MUSO_NOTE_MINIMUM && p<=MUSO_NOTE_MAXIMUM) {
      trigger_status[p - MUSO_NOTE_MINIMUM] = v>0; // TRIGGER_IS_ON;
    }
    
    MIDIOUT.sendNoteOn(p, v, MUSO_GATE_CHANNEL); //CHANNEL_DRUMS);  // output channel that the midimuso expects its triggers on*/
    last_input_at = millis();
  }
}

void fire_trigger(byte p, byte v) {
    if (p>=MUSO_NOTE_MINIMUM && p<=MUSO_NOTE_MAXIMUM) {
      trigger_status[p - MUSO_NOTE_MINIMUM] = v>0; // TRIGGER_IS_ON;
    }
    
    MIDIOUT.sendNoteOn(p, v, MUSO_GATE_CHANNEL); //CHANNEL_DRUMS);  // output channel that the midimuso expects its triggers on
}
void douse_trigger(byte p, byte v) {
    if (p>=MUSO_NOTE_MINIMUM && p<=MUSO_NOTE_MAXIMUM) {
      trigger_status[p - MUSO_NOTE_MINIMUM] = TRIGGER_IS_OFF;
    }
    
    MIDIOUT.sendNoteOff(p, v, MUSO_GATE_CHANNEL);   // hardcoded channel 16 for midimuso
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  byte p = pitch;
  byte v = velocity;

  activeNotes--;
  if (!process_triggers_for_pitch(pitch, velocity, false)) {
    p = convert_drum_pitch(pitch);
    /*if (p>=MUSO_NOTE_MINIMUM && p<=MUSO_NOTE_MAXIMUM) {
      trigger_status[p - MUSO_NOTE_MINIMUM] = TRIGGER_IS_OFF;
    }
    
    MIDIOUT.sendNoteOff(p, v, MUSO_GATE_CHANNEL);   // hardcoded channel 16 for midimuso
    */
  }

  last_input_at = millis();
}

void handleControlChange(byte channel, byte number, byte value) {
  // pass thru control changes, shifted to channel 1
  // TODO: intercept our own control messages to do things like set envelope settings, LFO settings, trigger targets/choke linking..
  if (number==CC_SYNC_RATIO) {
    cc_value_sync_modifier = constrain(value,1,127); //1 + (value-1); // minimum of 1    
  } else if (!handle_envelope_ccs(channel, number, value)) {
    //MIDI.sendControlChange(number, value, 1); // pass thru unhandled CV
  }
  last_input_at = millis();
}

void handleSongPosition(unsigned int beats) {
  // TODO: put LFO / envelope timer into correct phase, if that is possible?

  // hmm this doesnt get called?
  //update_pixels_position(beats);
  //NOISY_DEBUG(1, beats);

  song_position = beats/4;
  last_input_at = millis();
}

void handleClock() {
  // TODO: increment LFO / envelope timer by sync ratio..?
  // would it be enough just to replace calls to millis() with a call to eg get_clock_count()
  // and reset the clock_count on song start/stop?
  // and set the clock_count to an appropriate value in handle SongPosition ? clock_count = beats * 24 or somesuch, if its 24pqn ? beat is a 16th note i think tho so would it be * 96 ? need to check this
  MIDIOUT.sendClock();
  //NOISY_DEBUG(ticks,10);
  //NOISY_DEBUG(250,1);
  //ticks++;
  ticks += 1; //((float)(cc_value_sync_modifier^2)/127.0f);  // += 1
  estimated_ticks_per_ms = 1.0 / (millis() - last_tick_at);
  last_tick_at = millis();
  //update_pixels_position((int)ticks);
}

void handleStart() {
  // TODO: start LFOs?
  MIDIOUT.sendStart();
  ticks = 0;
}
void handleContinue() {
  // TODO: continue LFOs
  MIDIOUT.sendContinue();
  kill_envelopes();
}
void handleStop() {
  MIDIOUT.sendStop();
  // TODO: stop+reset LFOs
  kill_envelopes();
#ifdef ENABLE_PIXELS
  kill_notes();
#endif
  ticks = 0;
  //update_pixels_position((int)ticks);
}

void handleSystemExclusive(byte* array, unsigned size) {
  // pass sysex messages through to the midimuso
  // TODO: configuration of drum2musocv via sysex?
  MIDIOUT.sendSysEx(size, array, false); // true/false means "array contains start/stop padding" -- think what we receive here is without padding..?
}



#ifdef BUTTON_PIN
void handleButtonPressed(int state) { //uint8_t pin, uint8_t event, uint8_t count, uint16_t length) {
    /*Serial.print("Event : "); Serial.print(event);
    Serial.print(" Count : "); Serial.print(count);
    Serial.print(" Length: "); Serial.print(length);
    Serial.println();*/
    //handleNoteOn(10, GM_NOTE_MINIMUM+count, 127); //random(1,127));
    Serial.print("Button ");
    Serial.println(String(state));
    if (!state) { //event == EVENT_RELEASED) {
      demo_mode = !demo_mode;
      if (last_played_pitch>0)
        handleNoteOff(10, last_played_pitch, 0);
      kill_notes();
    }
}
#endif


#ifdef ENABLE_PIXELS
long last_updated_pixels_at = 0;
#endif


// -----------------------------------------------------------------------------

void setup() {
  //randomSeed(analogRead(1));

  delay(500); // give half a second grace to allow for programming

#ifdef USB_NATIVE
  Serial.begin(115200);   // usb serial debug port
  //while (!Serial);
  
  MIDIOUT.begin(GM_CHANNEL_DRUMS);
#endif
  MIDIIN.begin(GM_CHANNEL_DRUMS);

#ifdef ENABLE_PIXELS
  setup_pixels();
#endif

#ifdef BUTTON_PIN
  //pinMode(BUTTON_PIN, INPUT_PULLUP);
  button.setCallback(handleButtonPressed);
#endif

  initialise_euclidian();

  initialise_envelopes();

  // Initiate MIDI communications, listen to all channels
  //MIDIIN.begin(GM_CHANNEL_DRUMS); //MIDI_CHANNEL_OMNI);

  MIDIIN.turnThruOff();

  MIDIIN.setHandleNoteOn(handleNoteOn);
  MIDIIN.setHandleNoteOff(handleNoteOff);

  MIDIIN.setHandleControlChange(handleControlChange);

  MIDIIN.setHandleStop(handleStop);
  MIDIIN.setHandleStart(handleStart);
  MIDIIN.setHandleStart(handleContinue);

  MIDIIN.setHandleClock(handleClock);

  MIDIIN.setHandleSongPosition(handleSongPosition);

  //NOISY_DEBUG(1000, 1);

  kill_envelopes();
}

void loop() {
  // Call MIDI.read the fastest you can for real-time performance.
  MIDIIN.read();

#ifdef BUTTON_PIN
  //Serial.print("updating button - demo state is ");
  //Serial.println(String(demo_mode));
  button.update();
#endif

  // There is no need to check if there are messages incoming
  // if they are bound to a Callback function.
  // The attached method will be called automatically
  // when the corresponding message has been received.

  unsigned long now = clock_millis();
  unsigned long delta = now - time_last;

  if (demo_mode) {
    process_euclidian(now);
    /*if (random(0,5000)<10) {
      if (last_played_pitch>0) {
        handleNoteOff(10, last_played_pitch, 0);
        last_played_pitch = 0;
      } else {
        last_played_pitch = random(GM_NOTE_MINIMUM+1,GM_NOTE_MAXIMUM);
        handleNoteOn(10, last_played_pitch, random(1,127));
      }
    }*/
  }

  // update envelopes by time elapsed
  process_envelopes(now, delta);

  //Serial.print("now is "); Serial.println(now);

#ifdef ENABLE_PIXELS
  if (last_updated_pixels_at - millis() >= PIXEL_REFRESH) {
    last_updated_pixels_at = millis();
    update_pixels();
  }
#endif

  time_last = clock_millis();
}

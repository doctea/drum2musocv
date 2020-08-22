#include <MIDI.h>

#include "drums.h"

MIDI_CREATE_DEFAULT_INSTANCE();

// override default settings so that notes with velocity 0 aren't treated as note-offs
// doesn't work like i need
/*struct MySettings : public midi::DefaultSettings {
    static const long BaudRate = 31250;
    const bool HandleNullVelocityNoteOnAsNoteOff = false;
};
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, MySettings);*/

#define CONFIG_THROTTLE_MS 5

//TODO: make these CC values sensible and map them in FL
#define CC_SYNC_MOD 73

#define PPQN 24

enum envelope_types : byte {
  ENV_CRASH = 0,
  ENV_SPLASH = 1,
  ENV_WOBBLY = 2,
  // TODO: more envelope types...
};

// handling last time main loop was run, for calculating elapsed
unsigned long time_last;

byte cc_value_sync_modifier = 127;

float estimated_ticks_per_ms = 1.0f;

// for handling clock
//unsigned long 
float ticks = 0;  // store ticks as float, so can update by fractional ticks
unsigned long last_tick_at = 0;
unsigned long clock_millis() {
  //return millis();
  if (millis() - last_tick_at > 500) { // if we haven't received a clock for 100ms, fall back to internal millis clock
    return (millis() * 
      estimated_ticks_per_ms) * PPQN *
      ((float)(cc_value_sync_modifier^2)/127.0f);
  }
  return ticks * PPQN; // * ((float)(cc_sync_modifier^2)/127.0f);   // TODO: need to experiment to find a good tradeoff between allowing very short and very long stages at all tempos?  tempo-sync this basically?  
}

// -----------------------------------------------------------------------------

// This function will be automatically called when a NoteOn is received.
// It must be a void-returning function with the correct parameters,
// see documentation here:
// https://github.com/FortySevenEffects/arduino_midi_library/wiki/Using-Callbacks

byte convert_drum_pitch(byte pitch) {
  // in mode 0x0b (i think, need to theck this) there are 11 triggers available and a pitch out
  // TODO: make handy #defines for the different midimuso outputs + config
  byte p;
  if (pitch > 35 && pitch < 94) {
    p = pitch;
    switch (pitch) {
      case GM_NOTE_ELECTRIC_BASS_DRUM:  p = 72; break; //Electric Bass Drum - C5
      case GM_NOTE_SIDE_STICK:          p = 73; break; //Side Stick - C#5/Db5
      case GM_NOTE_HAND_CLAP:           p = 74; break; //Hand Clap - D5
      case GM_NOTE_ELECTRIC_SNARE:      p = 75; break; //Electric Snare - D#5/Eb5
      case GM_NOTE_CLOSED_HI_HAT:       p = 76; break; //Closed Hi-hat - E5
      case GM_NOTE_PEDAL_HI_HAT:        p = 82; break; //Pedal Hi-hat - A#5/Bb5
      case GM_NOTE_LOW_TOM:             p = 79; break; //Low Tom - G5
      case GM_NOTE_OPEN_HI_HAT:         p = 77; break; //Open Hi-hat - F5
      case GM_NOTE_CRASH_CYMBAL_1:      p = 78; break; //Crash Cymbal 1 - F#5/Gb5
      case GM_NOTE_HIGH_TOM:            p = 80; break; //High Tom - G#5/Ab5
      case GM_NOTE_RIDE_BELL:           p = 83; break; //Ride Bell - B5
      case GM_NOTE_TAMBOURINE:          p = 81; break; //Tambourine - A5
      //default: p = pitch + 12; //itch = 72; break;
      default: p = pitch; break;  
    }
    p = p - 12;
  }
  return p;
}


bool process_triggers_for_pitch(byte pitch, byte velocity, bool state) {
  // in the 0x0b midimuso-cv mode, there are 5 CV outputs and a clock output
  // the mapping is currently all hardcoded here and in Drums.h
  // TODO: make handy #defines for the different midimuso outputs + config, some way to link envelopes/triggers so as to be able to 'cut by' or choke/release hihats
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
  }
  return false;
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  //byte c = 16;
  byte p = pitch;
  byte v = velocity;

  p = convert_drum_pitch(pitch);

  if (!process_triggers_for_pitch(pitch, velocity, true))
    //if (v>0) // alt note off?
    MIDI.sendNoteOn(p, v, 16);  // hardcoded channel 16 for midimuso
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  //byte c = 16;
  byte p = pitch;
  byte v = velocity;

  p = convert_drum_pitch(pitch);

  if (!process_triggers_for_pitch(pitch, velocity, false)) {
    MIDI.sendNoteOff(p, v, 16);   // hardcoded channel 16 for midimuso
  }
}

void handleControlChange(byte channel, byte number, byte value) {
  // pass thru control changes, shifted to channel 1
  // TODO: intercept our own control messages to do things like set envelope settings, LFO settings, trigger targets/choke linking..
  if (number==CC_SYNC_MOD) {
    /*
     * TODO: reset the clock, so that can change sync amount without killing channels
     *        do i need need to move to a delta approach to do this ?
     *        float old_factor = ticks / cc_sync_modifier;
    ticks = old_factor * value;  // reset the clock to position according to the old scale, so we can vary this dynamically...
    time_last = (time_last * old_factor) * millis();*/
    cc_value_sync_modifier = value;
  } else {
    MIDI.sendControlChange(number, value, 1);
  }
}

void handleSongPosition(unsigned int beats) {
  // TODO: put LFO / envelope timer into correct phase, if that is possible?
}

void handleClock() {
  // TODO: increment LFO / envelope timer by sync ratio..?
  // would it be enough just to replace calls to millis() with a call to eg get_clock_count()
  // and reset the clock_count on song start/stop?
  // and set the clock_count to an appropriate value in handle SongPosition ? clock_count = beats * 24 or somesuch, if its 24pqn ? beat is a 16th note i think tho so would it be * 96 ? need to check this
  MIDI.sendClock();
  //NOISY_DEBUG(ticks,10);
  //NOISY_DEBUG(250,1);
  //ticks++;
  ticks+=((float)(cc_value_sync_modifier^2)/127.0f);
  estimated_ticks_per_ms = 1.0/((millis() - last_tick_at));
  last_tick_at = millis();
}

void handleStart() {
  // TODO: start LFOs?
  MIDI.sendStart();
  ticks = 0;
}
void handleContinue() {
  // TODO: start LFOs
  MIDI.sendContinue();
  kill_envelopes();
}
void handleStop() {
  MIDI.sendStop();
  // TODO: kill all envelopes, stop LFOs
  kill_envelopes();
}

void handleSystemExclusive(byte* array, unsigned size) {
  MIDI.sendSysEx(size, array, false); // true/false means "array contains start/stop padding" -- think what we receive here is without padding..?
}


// -----------------------------------------------------------------------------

void setup()
{

  randomSeed(analogRead(0));

  initialise_envelopes();

  // Initiate MIDI communications, listen to all channels
  MIDI.begin(10); //MIDI_CHANNEL_OMNI);

  MIDI.turnThruOff();

  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);

  MIDI.setHandleControlChange(handleControlChange);

  MIDI.setHandleStop(handleStop);
  MIDI.setHandleStart(handleStart);
  MIDI.setHandleStart(handleContinue);

  MIDI.setHandleClock(handleClock);

  NOISY_DEBUG(1000, 1);
}


void loop()
{
  // Call MIDI.read the fastest you can for real-time performance.
  MIDI.read();

  unsigned long now = clock_millis();
  unsigned long delta = now - time_last;

  // update envelope by time elapsed
  //unsigned long elapsed = now - decay_triggered;

  //decay_1 = ((now/1000)%127) / 127;
  //decay_1 += 0.1f;
  /*if (decay_1>1.0f) {
    decay_1 = 0.0f;
    }*/
  //if (decay_triggered > (now-decay_length))
  process_envelopes(now, delta);

  time_last = clock_millis();
  // There is no need to check if there are messages incoming
  // if they are bound to a Callback function.
  // The attached method will be called automatically
  // when the corresponding message has been received.
}

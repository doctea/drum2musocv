#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();


enum envelope_types {
  ENV_CRASH = 0,
  ENV_WOBBLE = 1,
  // TODO: more envelope types...
};

#define CONFIG_THROTTLE_MS 5


void NOISY_DEBUG(long t, int d) {
  for (int i = 0 ; i < t; i++) {
    MIDI.sendControlChange(7, i%127, 1);
    delay(d);
  }
}

void NUMBER_DEBUG(byte channel, byte data1, byte data2) {
  MIDI.sendControlChange(data1, data2, channel);
}

// -----------------------------------------------------------------------------

// This function will be automatically called when a NoteOn is received.
// It must be a void-returning function with the correct parameters,
// see documentation here:
// https://github.com/FortySevenEffects/arduino_midi_library/wiki/Using-Callbacks

byte convert_drum_pitch(byte pitch) {
      byte p;
      if (pitch>35 && pitch < 94) {
        p = 0;
        switch(pitch) {
          case 36: p = 72; break; //Electric Bass Drum - C5
          case 37: p = 73; break; //Side Stick - C#5/Db5
          case 39: p = 74; break; //Hand Clap - D5
          case 40: p = 75; break; //Electric Snare - D#5/Eb5
          case 42: p = 76; break; //Closed Hi-hat - E5
          case 44: p = 82; break; //Pedal Hi-hat - A#5/Bb5
          case 45: p = 79; break; //Low Tom - G5
          case 46: p = 77; break; //Open Hi-hat - F5
          case 49: p = 78; break; //Crash Cymbal 1 - F#5/Gb5
          case 50: p = 80; break; //High Tom - G#5/Ab5
          case 53: p = 83; break; //Ride Bell - B5
          case 54: p = 81; break; //Tambourine - A5
          //default: p = pitch + 12; //itch = 72; break;
        }
        p = p - 12;
      }
      return p;
}


bool process_triggers_for_pitch(byte pitch, byte velocity, bool state) {
  byte p;
  switch (pitch) {
    case 44:  // pedal hihat -- choke?
      break;
    case 57:  // cymbal crash 2
      break;
    case 55:  // splash cymbal
      break;
    case 67:
      // trigger envelope
      update_envelope(ENV_CRASH, velocity, state);
      return true;
      break;
  }
  return false;
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
    // Do whatever you want when a note is pressed.

    // Try to keep your callbacks short (no delays ect)
    // otherwise it would slow down the loop() and have a bad impact
    // on real-time performance.
    
    byte c = 16; //channel+1;
    byte p = pitch;
    byte v = velocity;

    p = convert_drum_pitch(pitch);

    if (!process_triggers_for_pitch(pitch, velocity, true))
      //if (v>0) // alt note off?
        MIDI.sendNoteOn(p,v,16); //channel);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
    // Do something when the note is released.
    // Note that NoteOn messages with 0 velocity are interpreted as NoteOffs.*/

    byte c = 16; //channel+1;
    byte p = pitch;
    byte v = velocity;

    p = convert_drum_pitch(pitch);   

    if (!process_triggers_for_pitch(pitch, velocity, false)) {
      MIDI.sendNoteOff(p,v,16); //channel);
      //MIDI.sendControlChange(7, 0, 1);
    } 
}

void handleControlChange(byte channel, byte number, byte value) {
    MIDI.sendControlChange(number, value, 1);
}

void handleSongPosition(unsigned int beats) {
  
}

void handleClock(void) {
  
}

// -----------------------------------------------------------------------------

void setup()
{

    randomSeed(analogRead(0));
    // Initiate MIDI communications, listen to all channels
    MIDI.begin(10); //MIDI_CHANNEL_OMNI);

    MIDI.turnThruOff();
    
    // Connect the handleNoteOn function to the library,
    // so it is called upon reception of a NoteOn.
    MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function

    // Do the same for NoteOffs
    MIDI.setHandleNoteOff(handleNoteOff);

    MIDI.setHandleControlChange(handleControlChange);

    NOISY_DEBUG(1000,1);
}

unsigned long time_last;

void loop()
{
    // Call MIDI.read the fastest you can for real-time performance.
    MIDI.read();

    unsigned long now = millis();
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

    time_last = millis();
    // There is no need to check if there are messages incoming
    // if they are bound to a Callback function.
    // The attached method will be called automatically
    // when the corresponding message has been received.
}

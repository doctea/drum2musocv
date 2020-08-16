#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

// -----------------------------------------------------------------------------

// This function will be automatically called when a NoteOn is received.
// It must be a void-returning function with the correct parameters,
// see documentation here:
// https://github.com/FortySevenEffects/arduino_midi_library/wiki/Using-Callbacks


float decay_1 = 0.0;
unsigned long decay_triggered = 0;
bool decay_1_active = false;
unsigned long last_sent = 0;
int decay_1_level = 127; // initial level of decay

#define decay_length 1000

byte convert_drum_pitch(byte pitch, bool triggers) {
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
          case 57: 
          case 67:
              if (triggers) {
                // trigger envelope? 
                decay_1 = 0.0f;
                decay_triggered = millis();
                decay_1_active = true;
                decay_1_level = MIDI.getData2(); //// need to know velocity here to handle this...
              }
              break;
          
          //default: p = pitch + 12; //itch = 72; break;
        }
        p = p - 12;
      }
      return p;
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

    //if (channel==10) {
      p = convert_drum_pitch(pitch,true);
    //}
    if (v>0) // alt note off?
      MIDI.sendNoteOn(p,v,16); //channel);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
    // Do something when the note is released.
    // Note that NoteOn messages with 0 velocity are interpreted as NoteOffs.*/

    byte c = 16; //channel+1;
    byte p = pitch;
    byte v = velocity;

    //if (channel==10) {
      p = convert_drum_pitch(pitch,false);
    //}
    
    MIDI.sendNoteOff(p,v,16); //channel);
}

void handleControlChange(byte channel, byte number, byte value) {
    //MIDI.sendControlChange(number, value, 1);
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

}

unsigned long time_last;

void loop()
{
    // Call MIDI.read the fastest you can for real-time performance.
    MIDI.read();

    unsigned long now = millis();
    unsigned long delta = now - time_last;
    
    // update envelope by time elapsed
    unsigned long elapsed = now - decay_triggered;

    //decay_1 = ((now/1000)%127) / 127;
    //decay_1 += 0.1f;
    /*if (decay_1>1.0f) {
      decay_1 = 0.0f;
    }*/
    //if (decay_triggered > (now-decay_length))
    if (decay_1_active) {
      if (last_sent==0 || abs(now - last_sent)>=5) { // only update every 10 ms
        if (elapsed <= (decay_length/10)) {
          MIDI.sendControlChange(7, decay_1_level, 1);
        } else if (elapsed <= decay_length) {
          decay_1 = 1.0f - (float)elapsed / (float)decay_length; /// decay_length; // reversing this gives cool effect?
          MIDI.sendControlChange(7, (byte)((float)decay_1_level * decay_1), 1); //decay_1 * 127, 1);
          //MIDI.sendControlChange(7, 127, 1); //(byte)random(0,127), 1); //decay_1 * 127, 1);
        } else {
          decay_1_active = false;
          MIDI.sendControlChange(7, 0, 1);
        }
        last_sent = millis();
      }
    }

    time_last = millis();
    // There is no need to check if there are messages incoming
    // if they are bound to a Callback function.
    // The attached method will be called automatically
    // when the corresponding message has been received.
}

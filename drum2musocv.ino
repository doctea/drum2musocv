#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

// -----------------------------------------------------------------------------

// This function will be automatically called when a NoteOn is received.
// It must be a void-returning function with the correct parameters,
// see documentation here:
// https://github.com/FortySevenEffects/arduino_midi_library/wiki/Using-Callbacks

byte convert_drum_pitch(byte pitch) {
      byte p;
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
        default: p = pitch + 12; //itch = 72; break;
      }
      p = p - 12;
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
      p = convert_drum_pitch(pitch);
    //}
    
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
      p = convert_drum_pitch(pitch);
    //}
    
    MIDI.sendNoteOff(p,v,16); //channel);
}

// -----------------------------------------------------------------------------

void setup()
{
    // Initiate MIDI communications, listen to all channels
    MIDI.begin(10); //MIDI_CHANNEL_OMNI);

    MIDI.turnThruOff();

    // Connect the handleNoteOn function to the library,
    // so it is called upon reception of a NoteOn.
    MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function

    // Do the same for NoteOffs
    MIDI.setHandleNoteOff(handleNoteOff);
}

void loop()
{
    // Call MIDI.read the fastest you can for real-time performance.
    MIDI.read();

    // There is no need to check if there are messages incoming
    // if they are bound to a Callback function.
    // The attached method will be called automatically
    // when the corresponding message has been received.
}

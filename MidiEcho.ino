// functions to help echo whats happening back to the host, eg if we want to MIDI record the euclidian patterns...

#include "MidiEcho.h"

void initialise_pitch_for_triggers() {
/*#if MUSO_MODE==MUSO_MODE_0B
  // right-hand column (triggers 1-5 running down)
  pitch_for_trigger[0] = GM_NOTE_ELECTRIC_BASS_DRUM;  //Electric Bass Drum - C5 72  // mode 0B = gate 1 = pin D1      / in mode 2B = pitch gate 1 / in mode 1B = pitch gate 1
  pitch_for_trigger[1] = GM_NOTE_SIDE_STICK;          //Side Stick - C#5/Db5 73     // mode 0B = gate 2 = pin D2      / in mode 2B = pitch gate 2
  pitch_for_trigger[2] = GM_NOTE_HAND_CLAP;           //Hand Clap - D5 74           // mode 0B = gate 3 = pin D3
  pitch_for_trigger[3] = GM_NOTE_ELECTRIC_SNARE;      //Electric Snare - D#5/Eb5 75 // mode 0B = gate 4 = pin D4
  pitch_for_trigger[4] = GM_NOTE_CRASH_CYMBAL_1;      //Crash Cymbal 1 - F#5/Gb5 78 // mode 0B = gate 5 = pin D5
  // (then clock out on D6)
  
  // left-hand column (triggers 11-6 running UP)
  pitch_for_trigger[10] = GM_NOTE_CLOSED_HI_HAT;       //Closed Hi-hat - E5 76      // mode 0B = gate 11 = pin B6
  pitch_for_trigger[9]  = GM_NOTE_OPEN_HI_HAT;         //77 //Open Hi-hat - F5      // mode 0B = gate 10 = pin B5
  pitch_for_trigger[8]  = GM_NOTE_PEDAL_HI_HAT;        //Pedal Hi-hat - A#5/Bb5 82  // mode 0B = gate 9  = pin B4

  pitch_for_trigger[7]  = GM_NOTE_LOW_TOM;             //79 //Low Tom - G5          // mode 0B = gate 8  = pin B3
  pitch_for_trigger[6]  = GM_NOTE_HIGH_TOM;            //High Tom - G#5/Ab5 80      // mode 0B = gate 7  = pin B2     / in mode 2B = Pitch 2
  pitch_for_trigger[5]  = GM_NOTE_TAMBOURINE;          //Tambourine - A5 81         // mode 0B = gate 6  = pin B1     / in mode 2B = Pitch 1  / in mode 1B = Pitch 1
#elif MUSO_MODE==MUSO_MODE_2B
  //pitch_for_trigger[0] = GM_NOTE_ELECTRIC_BASS_DRUM;  //Electric Bass Drum - C5 72  // mode 0B = gate 1 = pin D1      / in mode 2B = pitch gate 1 / in mode 1B = pitch gate 1
  //pitch_for_trigger[1] = GM_NOTE_SIDE_STICK;          //Side Stick - C#5/Db5 73     // mode 0B = gate 2 = pin D2      / in mode 2B = pitch gate 2
  pitch_for_trigger[0] = GM_NOTE_HAND_CLAP;           //Hand Clap - D5 74           // mode 0B = gate 3 = pin D3
  pitch_for_trigger[1] = GM_NOTE_ELECTRIC_SNARE;      //Electric Snare - D#5/Eb5 75 // mode 0B = gate 4 = pin D4
  pitch_for_trigger[2] = GM_NOTE_CRASH_CYMBAL_1;      //Crash Cymbal 1 - F#5/Gb5 78 // mode 0B = gate 5 = pin D5
  // (then clock out on D6)
  
  // left-hand column (triggers 11-6 running UP)
  pitch_for_trigger[10] = GM_NOTE_CLOSED_HI_HAT;       //Closed Hi-hat - E5 76      // mode 0B = gate 11 = pin B6
  pitch_for_trigger[9]  = GM_NOTE_OPEN_HI_HAT;         //77 //Open Hi-hat - F5      // mode 0B = gate 10 = pin B5
  pitch_for_trigger[8]  = GM_NOTE_PEDAL_HI_HAT;        //Pedal Hi-hat - A#5/Bb5 82  // mode 0B = gate 9  = pin B4

  pitch_for_trigger[7]  = GM_NOTE_LOW_TOM;             //79 //Low Tom - G5          // mode 0B = gate 8  = pin B3
  pitch_for_trigger[6]  = GM_NOTE_HIGH_TOM;            //High Tom - G#5/Ab5 80      // mode 0B = gate 7  = pin B2     / in mode 2B = Pitch 2
  pitch_for_trigger[5]  = GM_NOTE_TAMBOURINE;          //Tambourine - A5 81         // mode 0B = gate 6  = pin B1     / in mode 2B = Pitch 1  / in mode 1B = Pitch 1
#endif*/

  int t = 0;
//#if MUSO_MODE==MUSO_MODE_0B       // include kick and side stick if in all-gates mode
  pitch_for_trigger[t++] = GM_NOTE_ELECTRIC_BASS_DRUM;  //Electric Bass Drum - C5 72  // mode 0B = gate 1 = pin D1      / in mode 2B = pitch gate 1 / in mode 1B = pitch gate 1
  pitch_for_trigger[t++] = GM_NOTE_SIDE_STICK;          //Side Stick - C#5/Db5 73     // mode 0B = gate 2 = pin D2      / in mode 2B = pitch gate 2
//#endif
  pitch_for_trigger[t++] = GM_NOTE_HAND_CLAP;           //Hand Clap - D5 74           // mode 0B = gate 3 = pin D3
  pitch_for_trigger[t++] = GM_NOTE_ELECTRIC_SNARE;      //Electric Snare - D#5/Eb5 75 // mode 0B = gate 4 = pin D4
  pitch_for_trigger[t++] = GM_NOTE_CRASH_CYMBAL_1;      //Crash Cymbal 1 - F#5/Gb5 78 // mode 0B = gate 5 = pin D5
  
  // left-hand column (triggers 11-6 running DOWN)
//#if MUSO_MODE==MUSO_MODE_0B       // include tambourine and high tom if in all-gates mode
  pitch_for_trigger[t++]  = GM_NOTE_TAMBOURINE;          //Tambourine - A5 81         // mode 0B = gate 6  = pin B1     / in mode 2B = Pitch 1  / in mode 1B = Pitch 1
  pitch_for_trigger[t++]  = GM_NOTE_HIGH_TOM;            //High Tom - G#5/Ab5 80      // mode 0B = gate 7  = pin B2     / in mode 2B = Pitch 2
  pitch_for_trigger[t++]  = GM_NOTE_LOW_TOM;             //79 //Low Tom - G5          // mode 0B = gate 8  = pin B3     / in mode 2B = gate 4
/*#elif MUSO_MODE==MUSO_MODE_2B     // only include kick here if in pitches omde
  pitch_for_trigger[t++]  = GM_NOTE_ELECTRIC_BASS_DRUM;  // replace low tom with kick
#endif*/
  pitch_for_trigger[t++]  = GM_NOTE_PEDAL_HI_HAT;        //Pedal Hi-hat - A#5/Bb5 82  // mode 0B = gate 9  = pin B4     / in mode 2B = gate 5
  pitch_for_trigger[t++]  = GM_NOTE_OPEN_HI_HAT;         //77 //Open Hi-hat - F5      // mode 0B = gate 10 = pin B5     / in mode 2B = gate 6
  pitch_for_trigger[t++]  = GM_NOTE_CLOSED_HI_HAT;       //Closed Hi-hat - E5 76      // mode 0B = gate 11 = pin B6     / in mode 2B = gate 7

  // center column, CV outs
  pitch_for_trigger[t++] = GM_NOTE_CRASH_CYMBAL_2;       // MUSO_CC_CV_2             // mode 0B = CV2 / CC7   = pin C3
  pitch_for_trigger[t++] = GM_NOTE_SPLASH_CYMBAL;        // MUSO_CC_CV_3             // mode 0B = CV3 / CC11  = pin C4
  pitch_for_trigger[t++] = GM_NOTE_VIBRA_SLAP;           // MUSO_CC_CV_4             // mode 0B = CV4 / CC71  = pin C5
  pitch_for_trigger[t++] = GM_NOTE_RIDE_BELL;            // MUSO_CC_CV_1             // mode 0B = CV1 / CC1   = pin C2
  pitch_for_trigger[t++] = GM_NOTE_RIDE_CYMBAL_1;        // MUSO_CC_CV_5             // mode 0B = CV5 / CC74  = pin C6

}

// get the midi drum note associated with this trigger number, for midi echo
int get_pitch_for_trigger(int trigger) {
  return pitch_for_trigger[trigger];
}

// get the trigger number from incoming gm drum midi note
int get_trigger_for_pitch(int pitch) {
  //debug_pitch_for_trigger();
  for (int i = 0 ; i < NUM_TRIGGERS + NUM_ENVELOPES ; i++) {
    if (pitch_for_trigger[i]==pitch) {
      Serial.printf("get_trigger_for_pitch(%i), returning trigger %i\r\n", pitch, i);
      return i;
    }
  }
  return pitch; // or return the same pitch we were passed...?
}

void echo_fire_trigger(int trigger, int velocity) {
  if (!midiecho_enabled) return;
  if (trigger<NUM_TRIGGERS+NUM_ENVELOPES) { // dont send notes if this is the bass track todo: handle bass here too?
    int ext = get_pitch_for_trigger(trigger);
    //Serial.printf("# MidiEcho GOT TRIGGER FOR INTERNAL TRIGGER %i - sending external pitch %i!\r\n", trigger, ext); //p-MUSO_NOTE_MINIMUM));
    MIDIIN.sendNoteOn(ext, velocity, GM_CHANNEL_DRUMS);
  }
}

void echo_douse_trigger(int trigger, int velocity) {
  if (!midiecho_enabled) return;
  if (trigger<NUM_TRIGGERS+NUM_ENVELOPES) { // dont send notes if this is the bass track todo: handle bass here too?
    int ext = get_pitch_for_trigger(trigger);
    //Serial.printf("########## GOT DOUSE FOR INTERNAL TRIGGER %i - converted to external pitch %i!\r\n", trigger, ext);
    MIDIIN.sendNoteOff(ext, 0,       GM_CHANNEL_DRUMS); 
  }
}

bool handle_midiecho_ccs(int channel, int number, int value) {
  if (channel!=GM_CHANNEL_DRUMS) return false;

  if (number==CC_MIDIECHO_ENABLED) {
    //Serial.printf("Setting midiecho_enabled to %i\n", value);
    midiecho_enabled = value>0;
    return true;
  }
  return false;
}

String get_note_name(int pitch) {
  if (pitch==-1 || pitch>127) {
    String s = "_"; //note_names[chromatic_degree] + String(octave);
    return s;
  }
  int octave = pitch / 12;
  int chromatic_degree = pitch % 12; 
  const String note_names[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
  };
  
  String s = note_names[chromatic_degree] + String(octave);
  return s;
}

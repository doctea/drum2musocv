// functions to help echo whats happening back to the host, eg if we want to MIDI record the euclidian patterns...

int pitch_for_trigger[NUM_TRIGGERS + NUM_ENVELOPES];

void initialise_pitch_for_triggers() {
  pitch_for_trigger[0] = GM_NOTE_ELECTRIC_BASS_DRUM;  //Electric Bass Drum - C5 72
  pitch_for_trigger[1] = GM_NOTE_SIDE_STICK;          //Side Stick - C#5/Db5 73
  pitch_for_trigger[2] = GM_NOTE_HAND_CLAP;           //Hand Clap - D5 74
  pitch_for_trigger[3] = GM_NOTE_ELECTRIC_SNARE;      //Electric Snare - D#5/Eb5 75
  pitch_for_trigger[4] = GM_NOTE_CRASH_CYMBAL_1;      //Crash Cymbal 1 - F#5/Gb5 78
  
  // left-hand column (triggers 11-6 running down)
  pitch_for_trigger[10] = GM_NOTE_CLOSED_HI_HAT;       //Closed Hi-hat - E5 76
  pitch_for_trigger[9]  = GM_NOTE_OPEN_HI_HAT;         //77 //Open Hi-hat - F5
  pitch_for_trigger[8]  = GM_NOTE_PEDAL_HI_HAT;        //Pedal Hi-hat - A#5/Bb5 82

  pitch_for_trigger[7]  = GM_NOTE_LOW_TOM;             //79 //Low Tom - G5
  pitch_for_trigger[6]  = GM_NOTE_HIGH_TOM;            //High Tom - G#5/Ab5 80
  pitch_for_trigger[5]  = GM_NOTE_TAMBOURINE;          //Tambourine - A5 81

  pitch_for_trigger[11] = GM_NOTE_CRASH_CYMBAL_2;
  pitch_for_trigger[12] = GM_NOTE_SPLASH_CYMBAL;
  pitch_for_trigger[13] = GM_NOTE_VIBRA_SLAP;
  pitch_for_trigger[14] = GM_NOTE_RIDE_BELL;
  pitch_for_trigger[15] = GM_NOTE_RIDE_CYMBAL_1; 
}

int get_pitch_for_trigger(int trigger) {
  return pitch_for_trigger[trigger];
}

int get_trigger_for_pitch(int pitch) {
  for (int i = 0 ; i < NUM_TRIGGERS + NUM_ENVELOPES ; i++) {
    if (pitch_for_trigger[i]==pitch) return i;
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
  if (pitch==-1) {
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

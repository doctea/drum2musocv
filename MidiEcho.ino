// functions to help echo whats happening back to the host, eg if we want to MIDI record the euclidian patterns...

int pitch_for_trigger[NUM_TRIGGERS + NUM_ENVELOPES];

void initialise_pitch_for_triggers() {
  /*for (int i = 0 ; i < NUM_TRIGGERS + NUM_ENVELOPES ; i++) {
    if (i<NUM_TRIGGERS) {
      int p = 0;
      switch (i) {
        case MUSO_NOTE_GATE_1:            p = GM_NOTE_ELECTRIC_BASS_DRUM;  break; //Electric Bass Drum - C5 72
        case MUSO_NOTE_GATE_2:            p = GM_NOTE_SIDE_STICK:          break; //Side Stick - C#5/Db5 73
        case MUSO_NOTE_GATE_3;            p = GM_NOTE_HAND_CLAP:           break; //Hand Clap - D5 74
        case MUSO_NOTE_GATE_4:            p = GM_NOTE_ELECTRIC_SNARE:      break; //Electric Snare - D#5/Eb5 75
        case MUSO_NOTE_GATE_5:            p = GM_NOTE_CRASH_CYMBAL_1:      break; //Crash Cymbal 1 - F#5/Gb5 78
        
        // left-hand column (triggers 11-6 running down)
        case MUSO_NOTE_GATE_11:           p = GM_NOTE_CLOSED_HI_HAT:       break; //Closed Hi-hat - E5 76
        case MUSO_NOTE_GATE_10:           p = GM_NOTE_OPEN_HI_HAT:         break; //77 //Open Hi-hat - F5
        case MUSO_NOTE_GATE_9:            p = GM_NOTE_PEDAL_HI_HAT:        break; //Pedal Hi-hat - A#5/Bb5 82
  
        case MUSO_NOTE_GATE_8:            p = GM_NOTE_LOW_TOM:             break; //79 //Low Tom - G5
        case MUSO_NOTE_GATE_7:            p = GM_NOTE_HIGH_TOM:            break; //High Tom - G#5/Ab5 80
        case MUSO_NOTE_GATE_6:            p = GM_NOTE_TAMBOURINE:          break; //Tambourine - A5 81
        default:    p = 0; break;
      }
      pitch_for_trigger[i] = p;
    } else {
      pitch_for_trigger[i] = envelopes[i-NUM_TRIGGERS].
    }
  }*/
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

/*
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
      */

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
  int ext = get_pitch_for_trigger(trigger);
  //Serial.printf("########## GOT TRIGGER FOR INTERNAL TRIGGER %i - converted to external pitch %i!\r\n", trigger, ext); //p-MUSO_NOTE_MINIMUM));
  MIDIIN.sendNoteOn(ext, velocity, GM_CHANNEL_DRUMS);
}

void echo_douse_trigger(int trigger, int velocity) {
  int ext = get_pitch_for_trigger(trigger);
  //Serial.printf("########## GOT DOUSE FOR INTERNAL TRIGGER %i - converted to external pitch %i!\r\n", trigger, ext);
  MIDIIN.sendNoteOff(ext, 0,       GM_CHANNEL_DRUMS); 
}

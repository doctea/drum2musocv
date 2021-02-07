
int bass_held_notes[10] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };

#define BASS_HELD_NOTES_MAX (sizeof(bass_held_notes)/sizeof(bass_held_notes[0]))

int bass_held_notes_count = 0;

void bass_push_note (byte pitch) {
  for (int i = 0 ; i < BASS_HELD_NOTES_MAX ; i++) {
    if (bass_held_notes[i]==-1) { // free slot so add this new pitch
      bass_held_notes[i] = pitch;
      bass_held_notes_count = i+1;
      return;
    }
  }
  //bass_auto_note_held = true;
}

void bass_pop_note(byte pitch) {
  bool found = false;
  bool found_held = false;
  for (int i = 0 ; i < BASS_HELD_NOTES_MAX ; i++) {
    if (!found && bass_held_notes[i]==pitch) { // found the note that's just gone off
      found = true;
      bass_held_notes_count--;
    } else {
      found_held = true;
    }
    if (found && i+1 < BASS_HELD_NOTES_MAX) {
      bass_held_notes[i] = bass_held_notes[i+1];
    }
  }
  if (found) 
    bass_held_notes[BASS_HELD_NOTES_MAX-1] = -1;
  //bass_auto_note_held = true;
}

int get_bass_root_note() {
  if (bass_held_notes[0]!=-1) {
    return bass_held_notes[0];
  } else {
    return MIDI_BASS_ROOT_PITCH;
  }
}

static char debug_bass_string[30];
static const char* build_notes_held_string() {
  String s;

  bool found = false;
  for (int i = 0 ; i < bass_held_notes_count ; i++) {
    if (bass_held_notes[i]!=-1) {
      //sprintf(debug_bass_string, "%s, ", get_note_name(bass_held_notes[i]).c_str());
      s += get_note_name(bass_held_notes[i]);
      s += " ";
    }
  }
  sprintf(debug_bass_string, "%s", s.c_str());
  if (bass_held_notes_count>0)
    debug_bass_string[strlen(debug_bass_string)-1] = '\0';
  return (char *)debug_bass_string;
}

static const char* get_debug_bass_notes_held() {
  return (char *)debug_bass_string; //s.c_str();
  //return s.c_str();
}

void debug_bass_notes_held() {
  Serial.printf("held %i: ", bass_held_notes_count);
  Serial.println(get_debug_bass_notes_held());
}


bool is_bass_auto_note_held() {
  return (bass_held_notes[0]!=-1);
}

void bass_auto_note_on(byte pitch, byte vel) {
  //Serial.printf("got autobass note %i!", pitch);
  if (vel > 0) {
    //bass_root = pitch;
    //bass_auto_note_held = true;
    bass_push_note(pitch);
  }
  //debug_bass_notes_held();
  build_notes_held_string();
}

void bass_auto_note_off(byte pitch) {
  bass_pop_note(pitch);
  //if (pitch==bass_root)
  //  bass_auto_note_held = false;
  //debug_bass_notes_held();
  build_notes_held_string();
}

int bass_get_sequence_held_note(int position) {
  return bass_held_notes[position % bass_held_notes_count];
}

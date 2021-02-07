#ifndef CHANNELSTATE_INCLUDED
#define CHANNELSTATE_INCLUDED

//#include <Arduino.h>
#include "MidiSetup.hpp"
#include "MidiEcho.h"

class ChannelState {

private:
#define HELD_NOTES_MAX (sizeof(held_notes)/sizeof(held_notes[0]))
    int held_notes[10] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
    
    int held_notes_count = 0;
    bool note_held = false;

    // track pitches internally
    void push_note (byte pitch) {
      for (int i = 0 ; i < HELD_NOTES_MAX ; i++) {
        if (held_notes[i]==-1) { // free slot so add this new pitch
          held_notes[i] = pitch;
          held_notes_count = i+1;
          return;
        }
      }
      //auto_note_held = true;
      //debug_notes_held();
    }
    
    void pop_note(byte pitch) {
      bool found = false;
      bool found_held = false;
      for (int i = 0 ; i < HELD_NOTES_MAX ; i++) {
        if (!found && held_notes[i]==pitch) { // found the note that's just gone off
          found = true;
          held_notes_count--;
        } else {
          found_held = true;
        }
        if (found && i+1 < HELD_NOTES_MAX) {
          held_notes[i] = held_notes[i+1];
        }
      }
      if (found) 
        held_notes[HELD_NOTES_MAX-1] = -1;
      //auto_note_held = true;
      //debug_notes_held();
    }


    char debug_string[30];
    const char* build_notes_held_string() {
      String s;
    
      bool found = false;
      for (int i = 0 ; i < held_notes_count ; i++) {
        if (held_notes[i]!=-1) {
          //sprintf(debug_string, "%s, ", get_note_name(held_notes[i]).c_str());
          s += get_note_name(held_notes[i]);
          s += " ";
        }
      }
      sprintf(debug_string, "%s", s.c_str());
      if (held_notes_count>0)
        debug_string[strlen(debug_string)-1] = '\0';
      return (char *)debug_string;
    }

  
  public:  

    int get_root_note() {
      if (held_notes[0]!=-1) {
        return held_notes[0];
      } else {
        return MIDI_BASS_ROOT_PITCH;
      }
    }

    // is specific pitch currently held?
    bool is_note_held(int pitch) {
      for (int i = 0 ; i < HELD_NOTES_MAX ; i++) {
        if (held_notes[i]==pitch) 
          return true;
      }
      return false;
    }

    // is any note currently held?
    bool is_note_held() {
      return (held_notes[0]!=-1);
    }
    
    void handle_note_on(byte pitch, byte vel) {
      //Serial.printf("got autobass note %i!", pitch);
      if (vel > 0) {
        push_note(pitch);
      }
      //debug_notes_held();
      build_notes_held_string();
    }
    
    void handle_note_off(byte pitch) {
      pop_note(pitch);
      //if (pitch==root)
      //  auto_note_held = false;
      //debug_notes_held();
      build_notes_held_string();
    }
    
    int get_sequence_held_note(int position) {
      return held_notes[position % held_notes_count];
    }

    void debug_notes_held() {
      Serial.printf("held %i: ", held_notes_count);
      Serial.println(get_debug_notes_held());
    }

    const char* get_debug_notes_held() {
      return (char *)debug_string;
    }

};

#endif

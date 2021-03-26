#ifndef CHANNELSTATE_INCLUDED
#define CHANNELSTATE_INCLUDED

//#include <Arduino.h>
#include "MidiSetup.hpp"
#include "MidiEcho.h"

#define DEBUG_CHANNELSTATE  false

static int channelcount = 0;

class ChannelState {

private:
#define HELD_NOTES_MAX (sizeof(held_notes)/sizeof(held_notes[0]))

    int midi_root_pitch = MIDI_BASS_ROOT_PITCH;
    int held_notes_count = 0;
    bool note_held = false;

    // track pitches internally
    void push_note (byte pitch) {
      if (DEBUG_CHANNELSTATE) Serial.printf("channelstate push_note(%i)\r\n", pitch);
      for (int i = 0 ; i < HELD_NOTES_MAX ; i++) {
        if (held_notes[i]==-1) { // free slot so add this new pitch
          if (DEBUG_CHANNELSTATE) Serial.printf("   >adding note %i at %i\r\n", pitch, i);
          held_notes[i] = pitch;
          held_notes_count++;
          break;
        } else if (held_notes[i]>pitch) { // insert
          if (DEBUG_CHANNELSTATE) Serial.printf("   >inserting note %i at %i\r\n", pitch, i);
          //for (int x = i+1 ; x < HELD_NOTES_MAX ; x++) {
          for (int x = /*held_notes_count-1*/HELD_NOTES_MAX-1 ; x > i ; x--) {
            if (DEBUG_CHANNELSTATE) Serial.printf("     >moving note %i at %i to %i\r\n", held_notes[x-1], x-1, x);
            held_notes[x] = held_notes[x-1];
          }
          if (DEBUG_CHANNELSTATE) Serial.printf("     >and setting %i to %i\r\n", i, pitch);
          held_notes[i] = pitch;
          held_notes_count++;
          break;
        } else if (held_notes[i]==pitch) {
          break;  // dont duplicate notes
        }
      }

      // output debug info about multiple notes off sent
      if (DEBUG_CHANNELSTATE) Serial.printf("   >channel notes_held after push_note: [");
      for (int i = 0 ; i < 10 ; i++) {
        if (DEBUG_CHANNELSTATE) Serial.printf("%s ", get_note_name(held_notes[i]).c_str());
      }
      if (DEBUG_CHANNELSTATE) Serial.println("]");
      
      //auto_note_held = true;
      //debug_notes_held();
    }
    
    void pop_note(byte pitch) {
      if (DEBUG_CHANNELSTATE) Serial.printf("channelstate pop_note(%i)\r\n", pitch);
      /*bool found = false;
      bool found_held = false;
      for (int i = 0 ; i < HELD_NOTES_MAX ; i++) {
        if (!found && held_notes[i]==pitch) { // found the note that's just gone off
          found = true;
          held_notes_count--;
        } else {
          found_held = true;
        }
        if (found && i+1 < HELD_NOTES_MAX) {
          Serial.printf("   > found at %i, removing...\r\n", i);
          held_notes[i] = held_notes[i+1];
          //break;
        }
      }
      if (found) {
        held_notes[HELD_NOTES_MAX-1] = -1;
      } else {
        Serial.printf("     > didn't find pitch %i!\r\n", pitch);
      }*/

      for (int i = 0 ; i < HELD_NOTES_MAX ; i++) {
        if (held_notes[i]==pitch) {
          if (DEBUG_CHANNELSTATE) Serial.printf("   > found at %i, removing...\r\n", i);
          held_notes_count--;
          for (int x = i ; x < HELD_NOTES_MAX-1 ; x++) {
            held_notes[x] = held_notes[x+1];
          }
          held_notes[HELD_NOTES_MAX-1] = -1;
          //i--;
          break;
        }
      }
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
      if (DEBUG_CHANNELSTATE) sprintf(debug_string, "%s", s.c_str());
      if (held_notes_count>0)
        debug_string[strlen(debug_string)-1] = '\0';
      return (char *)debug_string;
    }
  
  
  public:  

    int held_notes[10] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };

  
    int get_root_note() {
      //Serial.printf("in get_root_note in ChannelState number #%i got: %s\r\n", chanindex, debug_string);
      if (is_note_held()) {
        /*// find the lowest held note to use as root
        int lowest = -1;
        for (int i = 0 ; i < HELD_NOTES_MAX ; i++) 
          if (held_notes[i]!=-1 && held_notes[i] < lowest)
            lowest = held_notes[i];
        return lowest;*/
        return held_notes[0];
      } else {
        //return MIDI_BASS_ROOT_PITCH;
        return midi_root_pitch;
      }
    }

    // returns true if value was changed
    bool set_midi_root_pitch(int pitch) {
      int last_pitch = midi_root_pitch;
      //if (midi_root_pitch==pitch) return;
      
      //harmony.kill_notes();
      midi_root_pitch = pitch;
      return last_pitch!=midi_root_pitch;
    }

    // is specific pitch currently held?
    bool is_note_held(int pitch) {
      if (DEBUG_CHANNELSTATE) Serial.printf("is_note_held: testing '%i' against: ", pitch);
      for (int i = 0 ; i < HELD_NOTES_MAX ; i++) {
        if (DEBUG_CHANNELSTATE) Serial.printf("%i ", held_notes[i]);
        if (held_notes[i]==pitch) {
          if (DEBUG_CHANNELSTATE) Serial.println("found");
          return true;
        }
      }
      if (DEBUG_CHANNELSTATE) Serial.print(" -- not found!!");
      return false;
    }

    // is any note currently held?
    bool is_note_held() {
      return (held_notes[0]!=-1);
    }
    
    void handle_note_on(byte pitch, byte vel = 127) {
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

    void handle_all_notes_off() {
      if (is_note_held()) {
        for (int i = 0 ; i < HELD_NOTES_MAX  ; i++) {
          if (held_notes[i]>-1)
            held_notes[i] = -1;
          else
            break;
        }
        held_notes_count = 0;
        build_notes_held_string();
      }
    }

    int *get_held_notes() {
      return held_notes;
    }
    
    int get_sequence_held_note(int position) {
      return held_notes[position % held_notes_count];
    }

    void debug_notes_held() {
      Serial.printf("held %i: ", held_notes_count);
      Serial.println(get_debug_notes_held());
    }

    const char* get_debug_notes_held() {
      build_notes_held_string();
      return (char *)debug_string;
    }

};

#endif

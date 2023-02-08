#include <Arduino.h>

#include <DebounceEvent.h>

#include "UI.h"
#include "Euclidian.h"
#include "Harmony.hpp"
#include "MidiOutput.hpp"

#define CC_CHANNEL_BITBOX_DRUMS_OUT 14    // set the MIDI channel to output the shadow drum triggers (default 11) 
#define CC_CHANNEL_GATE_OUT   15    // set the MIDI channel to output the Muso drum triggers (default 16)

int ui_last_action = ACTION_NONE;
short demo_mode = 0;
unsigned long button_pressed_at = 0;

#ifdef ENABLE_BUTTONS
void setup_buttons() {
  //button = 
  //pinMode (BUTTON_PIN_2, INPUT_PULLUP);
}

bool set_demo_mode(int mode);

bool first_ignored = false;
void handleButtonPressed(uint8_t pin, uint8_t event, uint8_t count, uint16_t length) {
    /*if (!first_ignored) {
      first_ignored = true;
      return;
    }*/
    /*Serial.print("Event : "); Serial.print(event);
    Serial.print(" Count : "); Serial.print(count);
    Serial.print(" Length: "); Serial.print(length);
    Serial.println();*/
    int previous_mode = demo_mode;

    bool should_kill = false;

    button_pressed_at = millis();
    ui_last_action = ACTION_NONE;

    if (pin==BUTTON_PIN_1) {      
      if (event==EVENT_RELEASED) { // && length<=250) {
        should_kill = set_demo_mode(demo_mode + count);
      }
      Serial.printf("pin %i: [event %i, count %i, length %i] - switched to demo_mode %i\r\n", pin, event, count, length, demo_mode);
    } else if (pin==BUTTON_PIN_2) {
        Serial.printf("pin %i: [event %i, count %i, length %i] - demo mode is %i\r\n", pin, event, count, length, demo_mode);
        if (demo_mode==MODE_EUCLIDIAN || demo_mode==MODE_EUCLIDIAN_MUTATION || demo_mode==MODE_EXPERIMENTAL || demo_mode==MODE_ARTSETC) {
          if (        event==EVENT_RELEASED && length>=2000 ) {
            Serial.printf(">>> Resetting euclidian sequences!\r\n");
            ui_last_action = ACTION_RESET_EUCLIDIAN;

            initialise_euclidian();
            
            harmony.reset_progression();
            harmony.reset_sequence();
            
          } else if ( event==EVENT_RELEASED && length<=500  ) {
            should_kill = euclidian_set_auto_play(!euclidian_auto_play);
            Serial.printf(">>> UI Setting auto-play in Euclidian mode to %c!\r\n", !euclidian_auto_play ? 'Y' : 'N');
          }
        }
    }

    if (should_kill) {
      kill_notes();
      kill_envelopes();
    }

}
#endif

// return true if need to kill playing notes due to change
bool set_demo_mode(int mode) {
  bool should_kill = false;
  int previous_mode = demo_mode;
  demo_mode = mode % NUM_DEMO_MODES;
  demo_mode = demo_mode % NUM_DEMO_MODES;
  
  bool reacted = demo_mode != previous_mode;

  Serial.printf("switched to demo_mode %i from %i\r\n", demo_mode, previous_mode);
    
  if (reacted) {
    ui_last_action = ACTION_MODE_CHANGE;
    if (bpm_internal_mode && previous_mode!=demo_mode) {
      //bool should_reset = ! ((previous_mode==MODE_EUCLIDIAN && demo_mode==MODE_EUCLIDIAN_MUTATION) || (demo_mode==MODE_EUCLIDIAN && previous_mode==MODE_EUCLIDIAN_MUTATION));  // don't reset if we've just switched between 1 and 2
      bool should_reset = previous_mode!=demo_mode && (previous_mode==MODE_RANDOM || demo_mode==MODE_RANDOM); // only reset if we just switched to or from random?
      if (should_reset) {
        should_kill = true;
        Serial.println("Resetting clock - set_demo_mode!");
        bpm_reset_clock(0);
      }
    }
    if (demo_mode==MODE_ARTSETC) {
      bpm_current = 30.0;
      max_euclidian_density = 1.0f;
    }

    if (demo_mode==MODE_STANDBY || demo_mode==MODE_RANDOM) {
      should_kill = true;
    }
  }

  return should_kill;
}


bool handle_ui_ccs(int channel, int number, int value) {
  if (channel!=GM_CHANNEL_DRUMS) return false;

  if (number==CC_DEMO_MODE) {
    if (set_demo_mode(value)) {
      kill_notes();
      kill_envelopes();
    }
    return true;
  } else if (number==CC_CHANNEL_BITBOX_DRUMS_OUT) {
    if (MIDI_CHANNEL_BITBOX_DRUMS_OUT==value) return true;

    Serial.printf("handle_ui_ccs received cc %i value %i\n", number, value);
    
    midi_kill_notes_bitbox_drums();
    //kill_envelopes(); // todo: only kill the envelopes going out to bitbox (ie shadow triggers)
    MIDI_CHANNEL_BITBOX_DRUMS_OUT = value;
    return true;
  } else if (number==CC_CHANNEL_GATE_OUT) {
    if (midi_channel_muso_gate==value) return true;
    
    midi_kill_notes_muso_drums();
    //kill_envelopes(); // todo: only kill the envelopes going out to muso (ie cv)
    midi_channel_muso_gate = value;
    return true;
  }

  return false;
}

DebounceEvent button1 = DebounceEvent(BUTTON_PIN_1, handleButtonPressed, BUTTON_PUSHBUTTON, 50);// | BUTTON_DEFAULT_LOW );// | BUTTON_SET_PULLUP);  // may need to change these if using different circuit;
DebounceEvent button2 = DebounceEvent(BUTTON_PIN_2, handleButtonPressed, BUTTON_PUSHBUTTON, 50); // | BUTTON_DEFAULT_HIGH | BUTTON_SET_PULLUP);  // may need to change these if using different circuit;

void update_buttons() {
  button1.loop();
  button2.loop();
}
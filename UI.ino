#ifdef ENABLE_BUTTONS

#include <DebounceEvent.h>

#include "UI.h"
#include "Euclidian.h"

#define CC_CHANNEL_BITBOX_OUT 14
#define CC_CHANNEL_GATE_OUT   15

DebounceEvent button1 = DebounceEvent(BUTTON_PIN_1, handleButtonPressed, BUTTON_PUSHBUTTON, 50);// | BUTTON_DEFAULT_LOW );// | BUTTON_SET_PULLUP);  // may need to change these if using different circuit;
DebounceEvent button2 = DebounceEvent(BUTTON_PIN_2, handleButtonPressed, BUTTON_PUSHBUTTON, 50); // | BUTTON_DEFAULT_HIGH | BUTTON_SET_PULLUP);  // may need to change these if using different circuit;

void setup_buttons() {
  //button = 
  //pinMode (BUTTON_PIN_2, INPUT_PULLUP);
}

void update_buttons() {
  button1.loop();
  button2.loop();
}


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
        if (demo_mode==MODE_EUCLIDIAN || demo_mode==MODE_EUCLIDIAN_MUTATION) {
          if (        event==EVENT_RELEASED && length>=2000 ) {
            Serial.printf(">>> Resetting euclidian sequences!\r\n");
            ui_last_action = ACTION_RESET_EUCLIDIAN;

            initialise_euclidian();
            
            harmony.reset_progression();
            harmony.reset_sequence();
            
          } else if ( event==EVENT_RELEASED && length<=500  ) {
            should_kill = euclidian_set_auto_play(!euclidian_auto_play);
            Serial.printf(">>> UI Setting auto-play in Euclidian mode to %c!\r\n", euclidian_auto_play ? 'Y' : 'N');
          }
        }
    }

    if (should_kill) {
      kill_notes();
      kill_envelopes();
    }

}

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
      bool should_reset = ! ((previous_mode==MODE_EUCLIDIAN && demo_mode==MODE_EUCLIDIAN_MUTATION) || (demo_mode==MODE_EUCLIDIAN && previous_mode==MODE_EUCLIDIAN_MUTATION));  // don't reset if we've just switched between 1 and 2
      if (should_reset) {
        should_kill = true;
        bpm_reset_clock(0);
      }
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
  } else if (number==CC_CHANNEL_BITBOX_OUT) {
    if (midi_channel_bitbox_out==value) return true;
    
    midi_kill_notes_bitbox();
    kill_envelopes(); // todo: only kill the envelopes going out to bitbox (ie shadow triggers)
    midi_channel_bitbox_out = value;
    return true;
  } else if (number==CC_CHANNEL_GATE_OUT) {
    if (midi_channel_muso_gate==value) return true;
    
    midi_kill_notes_muso_drums();
    kill_envelopes(); // todo: only kill the envelopes going out to muso (ie cv)
    midi_channel_muso_gate = value;
  }

  return false;
}

#endif

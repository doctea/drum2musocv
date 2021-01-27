#ifdef ENABLE_BUTTONS

#include <DebounceEvent.h>

#include "UI.h"
#include "Euclidian.h"

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

    bool should_reset = false;
    bool reacted = false;
    bool should_kill = false;

    button_pressed_at = millis();
    ui_last_action = ACTION_NONE;

    if (pin==BUTTON_PIN_1) {      
      if (event==EVENT_RELEASED) { // && length<=250) {
        demo_mode += count; 
        demo_mode = demo_mode % NUM_DEMO_MODES;
  
        reacted = true;
  
        Serial.printf("pin %i: [event %i, count %i, length %i] - switched to demo_mode %i\r\n", pin, event, count, length, demo_mode);
      } 
        
      if (reacted) {
        ui_last_action = ACTION_MODE_CHANGE;
        if (bpm_internal_mode && previous_mode!=demo_mode) {
          should_reset = ! ((previous_mode==MODE_EUCLIDIAN && demo_mode==MODE_EUCLIDIAN_MUTATION) || (demo_mode==MODE_EUCLIDIAN && previous_mode==MODE_EUCLIDIAN_MUTATION));  // don't reset if we've just switched between 1 and 2
          if (should_reset) {
            kill_notes();
            kill_envelopes();
            bpm_reset_clock(0);
          }
        }
    
        if (demo_mode==MODE_RANDOM || previous_mode==MODE_RANDOM) {
          if (last_played_pitch>-1)
            douse_trigger(last_played_pitch+MUSO_NOTE_MINIMUM, 0);
            //handleNoteOff(10, last_played_pitch, 0);
          kill_notes();
          kill_envelopes();
        } else if (demo_mode==MODE_STANDBY || demo_mode==MODE_RANDOM) {
          should_kill = true;
        }
      }
    } else if (pin==BUTTON_PIN_2) {
        Serial.printf("pin %i: [event %i, count %i, length %i] - demo mode is %i\r\n", pin, event, count, length, demo_mode);
        if (demo_mode==MODE_EUCLIDIAN || demo_mode==MODE_EUCLIDIAN_MUTATION) {
          if (        event==EVENT_RELEASED && length>=2000 ) {
            Serial.printf(">>> Resetting euclidian sequences!\r\n");
            ui_last_action = ACTION_RESET_EUCLIDIAN;

            initialise_euclidian();
          } else if ( event==EVENT_RELEASED && length<=500  ) {
            euclidian_auto_play = !euclidian_auto_play;
            if (bpm_internal_mode && !euclidian_auto_play) {
              should_kill = true;
            }
            Serial.printf(">>> Setting auto-play in Euclidian mode to %c!\r\n", euclidian_auto_play ? 'Y' : 'N');
          }
        }
    }

    if (should_kill) {
      douse_all_triggers(true);
      kill_notes();
      kill_envelopes();
    }

}
#endif

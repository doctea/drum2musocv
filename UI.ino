#ifdef BUTTON_PIN

DebounceEvent button = DebounceEvent(BUTTON_PIN, handleButtonPressed, BUTTON_PUSHBUTTON);// | BUTTON_DEFAULT_LOW );// | BUTTON_SET_PULLUP);  // may need to change these if using different circuit;

void setup_buttons() {
  //button = 
}

void update_buttons() {
  button.loop();
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
    bool reacted = false;
    if (event==EVENT_RELEASED && length<=250) {
      demo_mode ++; 
      demo_mode = demo_mode % NUM_DEMO_MODES;
      reacted = true;

      Serial.printf("pin %i: [event %i, count %i, length %i] - switched to demo_mode %i\r\n", pin, event, count, length, demo_mode);
    } else if (event==EVENT_RELEASED && length>250) {
      demo_mode += count;
      demo_mode = demo_mode % NUM_DEMO_MODES;
      reacted = true;
      
      Serial.printf("pin %i: [event %i, count %i, length %i] - switched to demo_mode %i\r\n", pin, event, count, length, demo_mode);
    }

    if (reacted) {
      if (last_played_pitch>0)
        handleNoteOff(10, last_played_pitch, 0);
      kill_notes();
      kill_envelopes();
    }

}
#endif

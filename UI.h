#ifndef UI_INCLUDED
#define UI_INCLUDED

// config 
#define BUTTON_PIN_1 A0
#define BUTTON_PIN_2 8


#define MODE_STANDBY            0
#define MODE_EUCLIDIAN          1
#define MODE_EUCLIDIAN_MUTATION 2
#define MODE_RANDOM             3
#define NUM_DEMO_MODES          4

#define UI_BUTTON_PRESSED_INDICATOR_INTERVAL  500 // duration to show on the pixels that a button has been pressed

#define ACTION_NONE             0
#define ACTION_MODE_CHANGE      1
#define ACTION_RESET_EUCLIDIAN  2

#define CC_DEMO_MODE  19

short demo_mode = 0;

unsigned long button_pressed_at = 0;

int ui_last_action = ACTION_NONE;


void setup_buttons();
void update_buttons();
bool handle_ui_ccs(int channel, int number, int value);


#endif

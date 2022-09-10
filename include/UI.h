#ifndef UI_INCLUDED
#define UI_INCLUDED

// config 
#define BUTTON_PIN_1 A0
#define BUTTON_PIN_2 8

#define MODE_STANDBY            0
#define MODE_EUCLIDIAN          1
#define MODE_EUCLIDIAN_MUTATION 2
#define MODE_EXPERIMENTAL       3
#define MODE_ARTSETC            4
#define MODE_RANDOM             5
#define NUM_DEMO_MODES          6

#define UI_BUTTON_PRESSED_INDICATOR_INTERVAL  500 // duration to show on the pixels that a button has been pressed

#define ACTION_NONE             0
#define ACTION_MODE_CHANGE      1
#define ACTION_RESET_EUCLIDIAN  2

#define CC_DEMO_MODE  19        // choose playback mode - 0=None, 1=Euclidian, 2=Euclidian with mutation, 3=Random triggers

extern short demo_mode;
extern unsigned long button_pressed_at;
extern int ui_last_action;

void setup_buttons();
void update_buttons();
bool handle_ui_ccs(int channel, int number, int value);


#endif

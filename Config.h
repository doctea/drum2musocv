#define SEEEDUINO // enable seeduino cortex m0+/Arduino Zero compatibility for FastLED (see Pixels.ino)

#ifdef SEEEDUINO
#define ARDUINO_SAMD_ZERO
#endif

// config settings
//#define TEST_TRIGGERS
#define ENABLE_PIXELS
//#define ENABLE_PIXELS_ADA // choose this or ENABLE_PIXELS_FASTLED
#define ENABLE_PIXELS_FASTLED

#define ENABLE_BUTTONS

#define CLOCK_TRIGGER_ENABLED

#define PIXEL_REFRESH   50  // number of milliseconds to wait between updating pixels (if enabled ofc)

//#define ENABLE_EEPROM     // untested, not available on SAMD platforms
//#define ENABLE_MIDI_ECHO  // now replaced with midiecho_enabled runtime-settable option

#define IDLE_TIMEOUT 5000 // five second timeout before going into 'idle mode' ie running own clock and displaying 'screensaver'

#define USB_NATIVE  // enable native usb support
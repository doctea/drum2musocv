#define SEEEDUINO // enable seeduino cortex m0+/Arduino Zero compatibility for FastLED (see Pixels.ino)

#define USB_NATIVE  // enable native usb support

#ifdef SEEEDUINO
#define ARDUINO_SAMD_ZERO
#endif

// config settings
//#define TEST_TRIGGERS
#define ENABLE_PIXELS
//#define ENABLE_PIXELS_ADA // choose this or ENABLE_PIXELS_FASTLED
#define ENABLE_PIXELS_FASTLED
#define PIXEL_REFRESH   50  // number of milliseconds to wait between updating pixels (if enabled ofc)

#define ENABLE_BUTTONS

#define ENABLE_CLOCK_TRIGGER

//#define ENABLE_EEPROM     // untested, not available on SAMD platforms
//#define ENABLE_MIDI_ECHO  // now replaced with midiecho_enabled runtime-settable option

#define IDLE_TIMEOUT 5000 // five second timeout after receiving midi before going into 'idle mode' ie running own clock and displaying 'screensaver'

// set the MIDIMUSO-CV12 configuration
//#define MUSO_MODE     MUSO_MODE_0B      // define this for single midimuso, all-gates mode
//#define MUSO_MODE     MUSO_MODE_0B      // define this for single midimuso, two-pitch-outputs mode
#define MUSO_MODE	MUSO_MODE_0B_AND_2A   // define this for dual midimuso, one in all-gates mode, one in two-pitch-outputs mode


// for clock settings
#define DEFAULT_CLOCK_TICK_RATIO    1      // use this to send standard MIDI clock at 24 PPQN
//#define DEFAULT_CLOCK_TICK_RATIO  PPQN   // or use this to only send 1 clock tick per beat

// for euclidian settings
#define DEFAULT_EUCLIDIAN_MUTATE_DENSITY true
#define DEFAULT_DURATION (STEPS_PER_BEAT / STEPS_PER_BEAT)

// for drum trigger output MIDI channels
#define DEFAULT_MUSO_GATE_CHANNEL           16   // channel to output triggers (ie gate triggers to notes on this channel)
#define DEFAULT_MIDI_CHANNEL_BITBOX_OUT     11   // for mirroring drum triggers to the bitbox pads

// for Harmony
#define DEFAULT_BASS_OCTAVE_OFFSET          -2
#define DEFAULT_BITBOX_KEYS_OCTAVE_OFFSET   2
#define DEFAULT_PAD_PITCH_OUT_OFFSET        0 //1
#define DEFAULT_PAD_ROOT_OUT_OFFSET         -1 //0 //0   //-2

// harmony output MIDI channels
#define DEFAULT_MIDI_CHANNEL_PAD_ROOT_OUT   1
#define DEFAULT_MIDI_CHANNEL_PAD_PITCH_OUT  2
#define DEFAULT_MIDI_CHANNEL_BITBOX_KEYS    3 // bass output, but shifted an octave
#define DEFAULT_MIDI_CHANNEL_BASS_OUT       4   //2     // channel to output bass notes on

// Harmony settings
#define DEFAULT_AUTO_PROGRESSION_ENABLED    false //true  // automatically play chords in progression order?
#define DEFAULT_BASS_ONLY_WHEN_NOTE_HELD    false // 
#define DEFAULT_SCALE                       0     // 0 = major, 1 = minor... 7 = hungarian minor
#define DEFAULT_AUTO_SCALE_ENABLED          true  //false   // true to increment scale on phrase change

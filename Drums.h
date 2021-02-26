#ifndef DRUMS_INCLUDED
#define DRUMS_INCLUDED

#define TRIGGER_IS_ON   true
#define TRIGGER_IS_OFF  false

#define GM_CHANNEL_DRUMS 10

#define GM_NOTE_MINIMUM 35
//#define GM_NOTE_SLAP 28   // extended?
#define GM_NOTE_ACOUSTIC_BASS_DRUM 35
#define GM_NOTE_ELECTRIC_BASS_DRUM 36
#define GM_NOTE_SIDE_STICK 37
#define GM_NOTE_ACOUSTIC_SNARE 38
#define GM_NOTE_HAND_CLAP 39
#define GM_NOTE_ELECTRIC_SNARE 40
#define GM_NOTE_LOW_FLOOR_TOM 41
#define GM_NOTE_CLOSED_HI_HAT 42
#define GM_NOTE_HIGH_FLOOR_TOM 43
#define GM_NOTE_PEDAL_HI_HAT 44
#define GM_NOTE_LOW_TOM 45
#define GM_NOTE_OPEN_HI_HAT 46
#define GM_NOTE_LOW_MID_TOM 47
#define GM_NOTE_HI_MID_TOM 48
#define GM_NOTE_CRASH_CYMBAL_1 49
#define GM_NOTE_HIGH_TOM 50
#define GM_NOTE_RIDE_CYMBAL_1 51
#define GM_NOTE_CHINESE_CYMBAL 52
#define GM_NOTE_RIDE_BELL 53
#define GM_NOTE_TAMBOURINE 54
#define GM_NOTE_SPLASH_CYMBAL 55
#define GM_NOTE_COWBELL 56
#define GM_NOTE_CRASH_CYMBAL_2 57
#define GM_NOTE_VIBRA_SLAP 58
#define GM_NOTE_RIDE_CYMBAL_2 59
#define GM_NOTE_HIGH_BONGO 60
#define GM_NOTE_LOW_BONGO 61
#define GM_NOTE_MUTE_HIGH_CONGA 62
#define GM_NOTE_OPEN_HIGH_CONGA 63
#define GM_NOTE_LOW_CONGA 64
#define GM_NOTE_HIGH_TIMBALE 65
#define GM_NOTE_LOW_TIMBALE 66
#define GM_NOTE_HIGH_AGOGO 67
#define GM_NOTE_LOW_AGOGO 68
#define GM_NOTE_CABASA 69
#define GM_NOTE_MARACAS 70
#define GM_NOTE_SHORT_WHISTLE 71
#define GM_NOTE_LONG_WHISTLE 72
#define GM_NOTE_SHORT_GUIRO 73
#define GM_NOTE_LONG_GUIRO 74
#define GM_NOTE_CLAVES 75
#define GM_NOTE_HIGH_WOODBLOCK 76
#define GM_NOTE_LOW_WOODBLOCK 77
#define GM_NOTE_MUTE_CUICA 78
#define GM_NOTE_OPEN_CUICA 79
#define GM_NOTE_MUTE_TRIANGLE 80
#define GM_NOTE_OPEN_TRIANGLE 81
#define GM_NOTE_MAXIMUM 81


// the midimuso-cv12 gate and cv output mappings - taken from http://midimuso.co.uk/wp-content/uploads/2017/08/CV_12_ORAC_Manual.pdf
#define MUSO_GATE_CHANNEL   16   // channel to output triggers (ie gate triggers to notes on this channel)
#define MUSO_NOTE_MINIMUM   60
#define MUSO_NOTE_GATE_1    60
#define MUSO_NOTE_GATE_2    61
#define MUSO_NOTE_GATE_3    62
#define MUSO_NOTE_GATE_4    63
#define MUSO_NOTE_GATE_5    64
#define MUSO_NOTE_GATE_6    65
#define MUSO_NOTE_GATE_7    66
#define MUSO_NOTE_GATE_8    67
#define MUSO_NOTE_GATE_9    68
#define MUSO_NOTE_GATE_10   69
#define MUSO_NOTE_GATE_11   70

#define NUM_TRIGGERS        (MUSO_NOTE_GATE_11 - MUSO_NOTE_MINIMUM + 1)
#define MUSO_NOTE_MAXIMUM   (MUSO_NOTE_MINIMUM + NUM_TRIGGERS)

// the CC values that the midimuso translates into the CV outputs
#define MUSO_CC_CV_1  1
#define MUSO_CC_CV_2  7
#define MUSO_CC_CV_3  11
#define MUSO_CC_CV_4  71
#define MUSO_CC_CV_5  74
#define MUSO_USE_PITCH_FOR  MUSO_CC_CV_5

#define MUSO_CV_CHANNEL     1   // channel to output CV CC's (ie envelopes to MidiMuso on this channel)

// tracking what triggers are currently active, for the sake of pixel output 
int trigger_status[NUM_TRIGGERS];

#endif

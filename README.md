# drum2musocv
Arduino sketch to be used in conjunction with USBMidiKlik (https://github.com/TheKikGen/USBMidiKliK, to provide USB MIDI connection) to adapt the MidiMuso CV-12 (http://midimuso.co.uk/index.php/cv-12/) to use General Midi note numbers on MIDI Channel 10, for use as a drum machine.

Also generates triggerable envelopes (works for two envelopes, in progress...)

Based on the example from the FortySevenEffects MIDI library https://github.com/FortySevenEffects/arduino_midi_library

# TODO

Figure out how to properly sync the MIDI clock to the envelopes/LFOs

Improve the existing envelopes + add a few more

## Future plans

More envelope types

Adjustable envelope parameters

Modulation per-stage or overall

MIDI clock, envelope length sync and LFOs

LED indicators of trigger received etc

Trigger other devices with CV via relays (ie circuit bent stuff).  Route MIDI to other devices?  Extra CVs/triggers direct from the Arduino?  Shitty sample recorder/player?!

Dedicated CCs to modulate the envelopes

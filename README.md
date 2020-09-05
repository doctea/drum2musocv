# drum2musocv

Arduino sketch to be used in conjunction with USBMidiKlik (https://github.com/TheKikGen/USBMidiKliK, to provide USB MIDI connection) to adapt the MidiMuso CV-12 (http://midimuso.co.uk/index.php/cv-12/) to use General Midi note numbers on MIDI Channel 10, for use as a drum machine.

This allows you to play the modular drum machine using external drum pads, or eg take advantage of FL Studio's note-naming in the piano roll.

Also generates triggerable envelopes with AHDSR (attack, hold, decay, sustain, release) stages.  Mapped to the 'Cymbal Crash 2', 'Cymbal Splash' and 'Vibra-slap' GM drum notes, outputting on the muso's CV outs #2, #3 and #4 respectively (midimuso CC 7, 11 and 71).

Indicates triggers and envelope levels via an 8-LED RGB strip using the FastLED library.

Uses the FortySevenEffects MIDI library https://github.com/FortySevenEffects/arduino_midi_library

# TODO

Add more envelopes, configurable to enable/disable so the CV outputs can be used as CC->CV, envelopes or LFOs

Better syncing of the MIDI clock to the envelopes (?)

LFOs

TODO: make modulation sync more useful... set upper/lower limits to modulation, elapsed-based scaling of modulation, only modulate during eg RELEASE stage

## Future plans / ideas

More envelope stages (delay?)

Make modulation configurable per-stage

Trigger other devices with CV via relays (ie circuit bent stuff).  Route MIDI/signals to other devices?  Extra CVs/triggers direct from the Arduino?  Shitty sample recorder/player?!

Hi-hat/crash choke 

LED indicators of trigger received etc (already does RGB strip but maybe someone would like standard LED drivers or more stuff.... matrix display could be cool)

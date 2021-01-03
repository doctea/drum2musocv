# drum2musocv

Arduino sketch adapting the MidiMuso CV-12 (http://midimuso.co.uk/index.php/cv-12/) to use General Midi note numbers on MIDI Channel 10, for easy use as a drum machine from a DAW.  

This allows you to play the modular drum machine using external drum pads, or eg take advantage of FL Studio's note-naming in the piano roll.

Also generates 5 triggerable envelopes with AHDSR (attack, hold, decay, sustain, release) stages.  Mapped to the 'Cymbal Crash 2', 'Cymbal Splash', 'Vibra-slap', 'Ride Bell' and 'Ride Cymbal 1' GM drum notes, outputting on the muso's CV outs #1, #2, #3, #4 and #5 respectively (midimuso CC 1, 7, 11, 71 and 74).

Indicates triggers and envelope levels via a 16-LED RGB Neopixel strip using the FastLED library.

Can be used in conjunction with USBMidiKlik (https://github.com/TheKikGen/USBMidiKliK) to provide USB MIDI, or can use native USB on boards that support it.

Uses the FortySevenEffects MIDI library https://github.com/FortySevenEffects/arduino_midi_library (with alternative experimental support for the Adafruit NeoPixel library).

Now also supports the Seeeduino Cortex M0+ board, presumably also works with Arduino Zero etc.

Includes a template for FL Studio to make controlling the general and envelope settings easy.

NEW: experimental generative euclidian rhythm generator

# TODO

Not convinced the envelopes are entirely working as intended so need to check & fix this

Make CC config options to able to enable/disable so the CV outputs can be used as CC, envelopes or LFOs per-project

LFOs

TODO: make modulation sync more useful... set upper/lower limits to modulation, elapsed-based scaling of modulation, only modulate during eg RELEASE stage

TODO: Make euclidian sequences changeable on the fly/configurable

BUG: there is a 'click' at end of envelope releases

TODO: find out whether an envelope is being held open or whether the ringing i'm getting is actually a problem with module

TODO: find out whether my output on '74'/Ride Cymbal 1 is broken due to code, panel mislabelling, or a problem with my midimuso-cv

## Future plans / ideas

More envelope stages (delay?)

Other 'modules' to trigger other devices with CV via relays (ie circuit bent stuff).  Route MIDI/signals to other devices?  Extra CVs/triggers direct from the Arduino?  Shitty sample recorder/player?!

Hi-hat/crash choke (cut open hat gate when pedal hat is triggered?)

Funkier colours / colour blending fx for the pixels

Save config options (RGB settings, envelope settings etc) to the flash memory and allow configuration via sysex / CCs.
	Started adding this, but isn't supported on SAMD platforms, so aborted (untested code remains)

### Done list

Make euclidian sequences work off midi clock, fix any bpm issues.
Make BPM guesser only work off the last 4 steps, to handle live changes of tempo better (done, but still needs 3 beats before it becomes accurate...?)
Fix problem where BPM guesser doesn't reset when stopped? - done i think
Fix problem with lights staying on when sequencer stopped? - seems to be resolved
first beat is missing when starting playback.. seems to think it starts from step 1?. -done for both playback and internal, i think !
echo euclidian output back to host so that we can record the midi or reuse it on other instruments etc
Better syncing of envelopes to midi clock ?
Make modulation configurable per-stage, eg only wobble during release

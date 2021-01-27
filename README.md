# drum2musocv

Arduino sketch adapting the MidiMuso CV-12 (http://midimuso.co.uk/index.php/cv-12/) to use General Midi note numbers on MIDI Channel 10, for easy use as a drum machine from a DAW.  

This allows you to play the modular drum machine using external drum pads, or eg take advantage of FL Studio's note-naming in the piano roll.

Also generates 5 triggerable envelopes with AHDSR (attack, hold, decay, sustain, release) stages.  Mapped to the 'Cymbal Crash 2', 'Cymbal Splash', 'Vibra-slap', 'Ride Bell' and 'Ride Cymbal 1' GM drum notes, outputting on the muso's CV outs #1, #2, #3, #4 and #5 respectively (midimuso CC 1, 7, 11, 71 and 74).

Indicates triggers and envelope levels via a 16-LED RGB Neopixel strip using the FastLED library.

Can be used in conjunction with USBMidiKlik (https://github.com/TheKikGen/USBMidiKliK) to provide USB MIDI, or can use native USB on boards that support it.

Uses the FortySevenEffects MIDI library https://github.com/FortySevenEffects/arduino_midi_library (with alternative experimental support for the Adafruit NeoPixel library).

Now also supports the Seeeduino Cortex M0+ board, presumably also works with Arduino Zero etc.

Includes a template for FL Studio to make controlling the general and envelope settings easy.

Has an experimental generative euclidian rhythm generator with optional mutation mode, so you can play with your synth without loading a DAW.

NEW: added a 'bass' input/output MIDI channel and corresponding Euclidian rhythm track, so it'll autoplay beat & bass rhythms (using it with my Neutron but could also be used with a 303-alike).

Temporary hack: uses the pitch bend output instead of the CV output that corresponds to CC 74, because mine seems to be broken.  (could use this in future to add an extra envelope/CV out or LFO output..)

# Controls

 - Two buttons, button 1 on pin A0 and button 2 on pin 8
 - Press button 1 = cycle through demo modes (standby -> Euclidian -> Euclidian with mutation -> random -> back to start)
 - When in a Euclidian mode:
   - Press button 2 = enable/disable Euclidian generation (to shut it up but to keep mode)
   - Hold button 2 for > 2 seconds & release = reset Euclidian patterns to initial default. (LEDs will light up red momentarily)
 - When any button is pressed or released, LEDs will light up violet, current mode indicated by a blue LED on first row of pixels, autoplaying status indicated by red/green on first pixel)

# MIDI parameters

(incomplete, need to add the notes corresponding to the Muso triggers for bass, snare, ch/oh etc, CCs corresponding to the envelope parameters, and other settings eg clock sync)

| MIDI type   | MIDI channel | MIDI number    | Purpose                    |
| ----------  | ------------ | -------------- | -------------------------- | 
| Note on/off | 10           | Cymbal Crash 2 | Trigger envelope on CV 1   |
| Note        | 10           | Cymbal Splash  | Trigger envelope on CV 2   |
| Note        | 10           | Vibra Slap     | Trigger envelope on CV 3   |
| Note        | 10           | Ride Bell      | Trigger envelope on CV 4   |
| Note        | 10           | Ride Cymbal    | Trigger envelope on CV 5*  |
| Note        | 10           | ..GM drums..   | Trigger Muso triggers      |
| Note+CC etc | 8            | any            | Resend on channel 2 (bass) |

* actually this currently outputs on the Pitch Bend output, as my Muso output seems to be broken

# Requirements

 - need to apply patch from https://github.com/arjanmels/debounceevent/commit/c26419a5a2eb83c07bcb69e8073cecd7453c53bf.patch to the DebounceEvent library (removes use of delay() by the library)


# TODO

 - Make CC config options to able to enable/disable so the CV outputs can be used as CCs, envelopes or LFOs per-project

 - LFOs

 - TODO: make modulation sync more featureful... set upper/lower limits to modulation, elapsed-based scaling of modulation

 - TODO: Make euclidian sequences changeable on the fly/configurable, saveable

 - TODO: find out whether my output on '74'/Ride Cymbal 1 is broken due to code, panel mislabelling, or a problem with my midimuso-cv

 - Add more physical buttons to provide greater control over modes / 

 - Make a KiCad circuit / PCB / panel to integrate LEDs and outputs behind a panel
 
 - Make Euclidian bass capable of changing root note / arpeggiate / chord sequences
 
 - Replace DebounceEvent library with one that doesn't need patching

## Future plans / ideas

 - More envelope stages (delay?)

 - Other 'modules' to trigger other devices with CV via relays (ie circuit bent stuff).  Extra CVs/triggers direct from the Arduino?  Shitty sample recorder/player?!

 - Hi-hat/crash choke (cut open hat gate when pedal hat is triggered?)

 - Funkier colours / colour blending fx for the pixels

 - Save config options (RGB settings, envelope settings, euclidian patterns etc) to the flash memory and allow configuration via sysex / CCs.
	 - Started adding this, but isn't supported on SAMD platforms, so aborted (untested code remains)
	 
 - Add extra output on the pitch bend output

## Requirements 

 - need to apply patch from https://github.com/arjanmels/debounceevent/commit/c26419a5a2eb83c07bcb69e8073cecd7453c53bf.patch to the DebounceEvent library (removes use of delay() by the library)

### Done list

 - Make euclidian sequences work off midi clock, fix any bpm issues.
 - Make BPM guesser only work off the last 4 steps, to handle live changes of tempo better (done, but still needs 3 beats before it becomes accurate...?)
 - Fix problem where BPM guesser doesn't reset when stopped? - done i think
 - Fix problem with lights staying on when sequencer stopped? - seems to be resolved
 - First beat is missing when starting playback.. seems to think it starts from step 1?. -done for both playback and internal, i think !
 - Echo euclidian output back to host so that we can record the midi or reuse it on other instruments etc
 - Better syncing of envelopes to midi clock ?
 - Make modulation configurable per-stage, eg only wobble during release
 - BUG: there is a 'click' at end of envelope releases
 - TODO: find out whether an envelope is being held open or whether the ringing i'm getting is actually a problem with module
 - Not convinced the envelopes are entirely working as intended so need to check & fix this
 - Make modulation sync work again
 - Route MIDI on specfic input channel to specific output channel, so can play eg Neutron through same interface
 - LEDs for indication of mode
 
 ----
 
 If you use or are interested in this project then it would be great to hear from you!

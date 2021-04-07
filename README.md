# drum2musocv

Arduino sketch adapting the MidiMuso CV-12 (http://midimuso.co.uk/index.php/cv-12/) to use General Midi note numbers on MIDI Channel 10, for easy use as a drum machine from a DAW.  

Now targets the Arduino Zero / Seeeduino Cortex M0+ boards, probably possible to make it work on a Uno again though with some changes + testing.  (interested in this?  let me know)

This allows you to play the modular drum machine using external drum pads, or eg take advantage of FL Studio's note-naming in the piano roll.

Could also be used to add Euclidian rhythms to any MIDI device (drum machine etc) if reconfigured to use different output note mappings.

When isn't receiving an external clock input, runs off its own internal clock at the last detected BPM.

Generates 5 triggerable envelopes with AHDSR (attack, hold, decay, sustain, release) stages.  Mapped to the 'Cymbal Crash 2', 'Cymbal Splash', 'Vibra-slap', 'Ride Bell' and 'Ride Cymbal 1' GM drum notes for input, outputting on the muso's CV outs #1, #2, #3, #4 and #5 respectively (via midimuso CC 1, 7, 11, 71 and 74 - with recongfiguring could be used as triggerable CC envelopes for any device).

Indicates triggers and envelope levels via a 16-LED RGB Neopixel strip using the FastLED library.

Includes a template for FL Studio to make controlling the general and envelope settings easy.

Has a Euclidian rhythm generator with optional mutation mode, so you can play with your synth without loading a DAW.  Can configure tracks on/off, change mutation settings, etc, from DAW.  In mutation mode, mutates the rhythm every 2 bars.

Plays a "drum fill" for the last bar of every phrase.

A 'bass' input/output MIDI channel and corresponding Euclidian rhythm track, so it'll autoplay beat & bass rhythms (using it with my Neutron but could also be used with a 303-alike or anything really..).  Notes played in via MIDI determine which notes to use for arpeggiation/chords.
WIP (kinda working but needs some expanding): autoplay chords or arpeggiator on a second device eg Bitbox or another synth.

Temporary hack: uses the pitch bend output instead of the CV output that corresponds to CC 74, because mine seems to be broken.  (could use this in future to add an extra envelope/CV out or LFO output..)

Echoes the MIDI back to the host so that you can record the rhythms for re-use or to route to softsynths or other MIDI devices.

Outputs 4xClock triggers using a CD74HC4067 multiplexor (5ms latency between outputs if multiple triggered simultaneously) - every beat, every upbeat, every bar, every phrase.  Uses Arduino data pins 2,3,4,5 to set the multiplexor output.

# Controls

 - Two buttons, button 1 on pin A0 and button 2 on pin 8
 - Press button 1 = cycle through demo modes (standby -> Euclidian -> Euclidian with mutation -> random -> back to start)
 - When in a Euclidian mode:
   - Press button 2 = enable/disable Euclidian generation (to shut it up but to keep mode)
   - Hold button 2 for > 2 seconds & release = reset Euclidian patterns to initial default. (LEDs will light up red momentarily)
 - When any button is pressed or released, LEDs will light up violet, current mode indicated by a blue LED on first row of pixels, autoplaying status indicated by red/green on last pixel)

# Routing of features

A diagram to help me understand how everything is routed:-

![Routing diagram](drum2musocv_routing_diagram.svg)


# MIDI parameters

(TODO: incomplete, need to add the notes corresponding to the Muso triggers for bass drum, snare, ch/oh etc, CCs corresponding to the envelope parameters, and other settings eg clock sync)

 - see the [CHANNEL_AND_CC_LIST.txt](CHANNEL_AND_CC_LIST.txt) document for an autogenerated list of the existing mappings

## MIDI Inputs

| MIDI type   | MIDI channel | MIDI number    | Purpose                    |
| ----------  | ------------ | -------------- | -------------------------- | 
| Note on/off | 10           | Cymbal Crash 2 | Trigger envelope on CV 1   |
| Note        | 10           | Cymbal Splash  | Trigger envelope on CV 2   |
| Note        | 10           | Vibra Slap     | Trigger envelope on CV 3   |
| Note        | 10           | Ride Bell      | Trigger envelope on CV 4   |
| Note        | 10           | Ride Cymbal    | Trigger envelope on CV 5*  |
| Note        | 10           | ..GM drums..   | Trigger Muso triggers      |
| CC          | 10           | 32 to 48       | Enable/disable Euclidian track |
| Note+CC etc | 8            | any            | Bass synth - Resend on channel 2 |
| Note        | 9            | any            | Bass synth - Euclidian track arpeggiates held chord on chan 2+3 (see below) |

## MIDI Outputs

| Type         	| MIDI channel | MIDI number   | Purpose |
| ------------- | ------------ | ------------- | ------- |
| Notes/CC/etc 	| 4 	| any | Output to bass synth | TODO: forward modulation as aftertouch/velocity/modwheel etc |
| Notes/CC/etc  | 3     | any | Output bass notes 2 octaves higher, for playing melodies / pads | 
| Notes/CC/etc  | 1     | any | Output to Muso Pitch 1 |
| Notes/CC/etc  | 2     | any | Output to Muso Pitch 2 |
| Notes   	| 11    | 36-51 (C2-D#3) | 'shadow' copy of notes sent to midimuso, starting at C2 so they work with default bitbox pads |
| Notes 	| 16 	| 60-70 (C4-?) | outputs to MIDI Muso CV-12, gates |
| CV + pitch 	| 1 	| 1,7,11,71,74+pitch | outputs to MIDI Muso, CV/pitch outputs |

* *actually this currently outputs on the Pitch Bend output, as my Muso output seems to be broken - configure with MUSO_USE_PITCH_FOR in Drums.h 

# Requirements

 - Use the Seeeduino Zero board profile so that Serial has printf() - install boards from URL https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json and add boards.txt to eg C:\Users\<YOUR_USERNAME>\AppData\Local\Arduino15\packages\Seeeduino\hardware\samd\1.8.1\boards.txt - can edit the USB device name from here
 - Can be used in conjunction with USBMidiKlik (https://github.com/TheKikGen/USBMidiKliK) to provide USB MIDI, or can use native USB on boards that support it.
 - Uses the FortySevenEffects MIDI library https://github.com/FortySevenEffects/arduino_midi_library (with alternative experimental support for the Adafruit NeoPixel library).
 - DebounceEvent uses delay(), so need to apply patch from https://github.com/arjanmels/debounceevent/commit/c26419a5a2eb83c07bcb69e8073cecd7453c53bf.patch to fix stutter when buttons are pressed


# FL Studio DAW Control Surface preset

 - *Euclidian settings.fst* is an FL Studio Patcher preset for controlling the Euclidian settings (screenshot below).
 - *Drum2MusoCV - Bamblweeny.fst* is an FL Studio MIDI Out preset for controlling the drum triggers and envelope settings. 

![Euclidian settings FL preset screenshot](DAW%20templates/Euclidian%20settings.png)



# TODO / future plans + ideas

 - Replace DebounceEvent library with one that doesn't need patching

 - Make CC config options to able to enable/disable so the CV outputs can be used as CCs, envelopes or LFOs per-project

 - LFOs

 - TODO: make modulation sync more featureful... set upper/lower limits to modulation, elapsed-based scaling of modulation

 - TODO: Make euclidian sequences changeable on the fly/configurable, saveable

 - TODO: find out whether my output on '74'/Ride Cymbal 1 is broken due to code, panel mislabelling, or a problem with my midimuso-cv

 - Add more physical buttons to provide greater control over modes  

 - Make a KiCad circuit / PCB / panel to integrate LEDs and outputs behind a panel
 
 - Make Euclidian bass capable of changing root note / arpeggiate / chord sequences
   - features to do this.  currently arps over held chord
   - make this so can switch between modes...
 
 - Replace DebounceEvent library with one that doesn't need patching

 - Euclidian fills on last bar of phrase.  Multiply the track parameters to increase/decrease density?

 - Latency/flam/swing on euclidian tracks.  Adjust the euclidian loop to check a copy of received_ticks that is adjusted by the appropriate latency
	- ie if latency is set to -6, then remove 6 from received_ticks before tests is on bar etc
		- gonna need to restructure slightly for this, or maybe just make some defines for testing like IS_ON_BAR(ticks), IS_ON_STEP(ticks)?
		- will need to turn the loops inside out and iterate over tracks before checking if theyre on beat

 - Tied notes, per-track or just for bass.  to do this do we just avoid sending note off for last note until new note has been sent..? off/always/random/pattern?

 - Cut bass by kick option

 - Completely edit euclidian track parameters over MIDI... with feedback to control surface in daw for saving?  Trigger all outputs simultaneously/groups if have two recording groups

 - Shadow triggers with MIDI to trigger Bitbox.  output options - global/per track - muso/bitbox/both.  always/random/alternate/pattern (+euclidian)?  add this to fire_trigger & douse_trigger?
	- more broadly this is turning into a need to have more of a structure for outputs, for sending to host/midi over multiple channels and can be turned on and off easily
	- Allow to configure the octave-shifted melody/pad outputs on Channel 3:
		- configuration of octave shift
		- option to arpeggiate on None / Channel 2 / Channel 3 / Both
		- option to play full-chord / arp / root-only on Channel 3

 - set some of the envelopes to act as envelopes synchronous with some of the triggers, for applying modulation to hits.
	- turn one into a bar-synced LFO

 - Bitbox recording mode? ie send all triggers necessary to record a sample.  buttons to trigger recording?

 - More 'predictable' random streams... like use 'different' random streams for picking numbers at different stages of mutation, so that can add feature to mutate more tracks simultaneously but keep the existing patterns the same
	- so keep the selection of patterns chosen to change the same, each pattern's mutations the same -- actually not sure if i'm on the right track here.  come back when less tired!

 - More envelope stages (delay?)

 - Other 'modules' to trigger other devices with CV via relays (ie circuit bent stuff).  Extra CVs/triggers direct from the Arduino?  Shitty sample recorder/player?!

 - Hi-hat/crash choke (cut open hat gate when pedal hat is triggered?)

 - Funkier colours / colour blending fx for the pixels

 - Save config options (RGB settings, envelope settings, euclidian patterns etc) to the flash memory and allow configuration via sysex / CCs.
	 - Started adding this, but isn't supported on SAMD platforms, so aborted (untested code remains)
	 - support FRAM over SPI?
	 
 - Add extra output on the pitch bend output (currently using this to replace my broken output cc74)


### Done list

 - Euclidian fills on last bar of phrase.  Multiply the track parameters to increase/decrease density?
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
 - Shadow triggers with MIDI to trigger Bitbox
   - done: basic shadow of triggers+envelopes on the default bitbox pad trigger notes, sending on channel 11
 
 ----
 
 If you use or are interested at all in this project then it would be great to hear from you!

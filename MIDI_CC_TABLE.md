| Chan | File | CC | Name | Comment |
| ---- | ---- | -- | ---- | ------- |
`10` | `include/Harmony.hpp` | `2` | `CC_BASS_MELODY_MODE` | ` output 0: set mode to use for the bass output - 0=None, 1=Single note, 2=Chord, 3=Arpeggiate chord` |
`10` | `include/Harmony.hpp` | `3` | `CC_PAD_PITCH_MELODY_MODE` | ` output 3: set mode to use for the pitch output - 0=None, 1=Single note, 2=Chord, 3=Arpeggiate chord` |
`10` | `include/Harmony.hpp` | `4` | `CC_PAD_PITCH_OCTAVE_OFFSET` | ` output 3: set octave offset (-2 to +3)` |
`10` | `include/Harmony.hpp` | `5` | `CC_CHANNEL_PAD_PITCH` | ` output 3: set the MIDI channel to output the pads pitch on (default 2)` |
`10` | `include/Harmony.hpp` | `6` | `CC_BASS_SET_TIE_ON` | ` output 0: set which steps the Euclidian bass should tie on` |
`10` | `include/Euclidian.h` | `7` | `CC_EUCLIDIAN_MUTATE_DENSITY` | ` automatically mutate density on/off` |
`10` | `include/Euclidian.h` | `8` | `CC_EUCLIDIAN_CLAP_FLAM` | ` enable/disable clap flam, 0=off` |
`10` | `include/Euclidian.h` | `9` | `CC_EUCLIDIAN_HIHAT_SHUFF` | ` enable/disable hihat shuffle, 0=off` |
`10` | `include/Harmony.hpp` | `10` | `CC_CHANNEL_PAD_ROOT` | ` output 2: set the MIDI channel to output the pad root on (default 1)` |
`10` | `include/Harmony.hpp` | `11` | `CC_MELODY_ROOT` | ` set the MIDI note to use as the root pitch, 48=C4` |
`10` | `include/Harmony.hpp` | `12` | `CC_CHANNEL_BASS_OUT` | ` output 0: set the MIDI channel to output the bass on (default 4)` |
`10` | `include/Harmony.hpp` | `13` | `CC_CHANNEL_BITBOX_KEYS` | ` output 1: value received on this CC sets the MIDI channel to output the chords on (default 3)` |
`10` | `include/Euclidian.h` | `16` | `CC_EUCLIDIAN_SET_AUTO_PLAY` | ` enable/disable autoplaying on internal BPM` |
`10` | `include/Harmony.hpp` | `17` | `CC_BASS_SET_ARP_MODE` | ` cc to set the bass arp mode` |
`10` | `include/Harmony.hpp` | `18` | `CC_HARMONY_ONLY_NOTE_HELD` | ` cc to set bass to only play in external mode if note is held` |
`10` | `include/UI.h` | `19` | `CC_DEMO_MODE` | ` choose playback mode - 0=None, 1=Euclidian, 2=Euclidian with mutation, 3=Random triggers` |
`10` | `include/Euclidian.h` | `20` | `CC_EUCLIDIAN_SET_MUTATE_MODE` | ` 0 = NONE, 1 = SUBTLE, 2 = TOTAL` |
`10` | `include/MidiEcho.h` | `21` | `CC_MIDIECHO_ENABLED` | ` enable echoing of output back to DAW over USB` |
`10` | `include/Euclidian.h` | `22` | `CC_EUCLIDIAN_SEED_MODIFIER` | ` euclidian random "bank" X` |
`10` | `include/Euclidian.h` | `23` | `CC_EUCLIDIAN_SEED_MODIFIER_2` | ` euclidian random "bank" Y` |
`10` | `include/Euclidian.h` | `24` | `CC_EUCLIDIAN_RESET_BEFORE_MUTATE` | ` reset sequences to 0 before mutation?` |
`10` | `include/Euclidian.h` | `25` | `CC_EUCLIDIAN_SET_MINIMUM_PATTERN` | ` lowest number pattern to automutate` |
`10` | `include/Euclidian.h` | `26` | `CC_EUCLIDIAN_SET_MAXIMUM_PATTERN` | ` highest number pattern to automutate` |
`10` | `include/Euclidian.h` | `27` | `CC_EUCLIDIAN_SEED_USE_PHRASE` | ` increment seed according to BPM phrase (ie automutate)` |
`10` | `include/Euclidian.h` | `28` | `CC_EUCLIDIAN_FILLS` | ` enable/disable fills on last bar of phrase, 0=off` |
`10` | `include/Harmony.hpp` | `29` | `CC_HARMONY_MELODY_MODE` | ` output 1: value received on this CC sets the mode to use for the chords output - 0=None, 1=Single note, 2=Chord, 3=Arpeggiate chord` |
`10` | `include/Harmony.hpp` | `30` | `CC_HARMONY_MUTATE_MODE` | ` overall harmony mutation mode, 0=None, 1=Randomise` |
`10` | `include/Harmony.hpp` | `31` | `CC_AUTO_PROGRESSION` | ` enable/disable playing auto chord progression` |
`10` | `include/Euclidian.h` | `32` | `Euclidian on/off for pattern 0` | `` |
`10` | `include/Euclidian.h` | `33` | `Euclidian on/off for pattern 1` | `` |
`10` | `include/Euclidian.h` | `34` | `Euclidian on/off for pattern 2` | `` |
`10` | `include/Euclidian.h` | `35` | `Euclidian on/off for pattern 3` | `` |
`10` | `include/Euclidian.h` | `36` | `Euclidian on/off for pattern 4` | `` |
`10` | `include/Euclidian.h` | `37` | `Euclidian on/off for pattern 5` | `` |
`10` | `include/Euclidian.h` | `38` | `Euclidian on/off for pattern 6` | `` |
`10` | `include/Euclidian.h` | `39` | `Euclidian on/off for pattern 7` | `` |
`10` | `include/Euclidian.h` | `40` | `Euclidian on/off for pattern 8` | `` |
`10` | `include/Euclidian.h` | `41` | `Euclidian on/off for pattern 9` | `` |
`10` | `include/Euclidian.h` | `42` | `Euclidian on/off for pattern 10` | `` |
`10` | `include/Euclidian.h` | `43` | `Euclidian on/off for pattern 11` | `` |
`10` | `include/Euclidian.h` | `44` | `Euclidian on/off for pattern 12` | `` |
`10` | `include/Euclidian.h` | `45` | `Euclidian on/off for pattern 13` | `` |
`10` | `include/Euclidian.h` | `46` | `Euclidian on/off for pattern 14` | `` |
`10` | `include/Euclidian.h` | `47` | `Euclidian on/off for pattern 15` | `` |
`10` | `include/Euclidian.h` | `48` | `Euclidian on/off for pattern 16` | `` |
`10` | `include/Euclidian.h` | `49` | `Euclidian on/off for pattern 17` | `` |
`10` | `include/Euclidian.h` | `50` | `Euclidian on/off for pattern 18` | `` |
`10` | `include/Euclidian.h` | `51` | `Euclidian on/off for pattern 19` | `` |
`10` | `include/Envelopes.h` | `64` | `Envelope 0 Attack` | `Attack time` |
`10` | `include/Envelopes.h` | `65` | `Envelope 0 Hold` | `Hold after attack time` |
`10` | `include/Envelopes.h` | `66` | `Envelope 0 Decay` | `Decay time` |
`10` | `include/Envelopes.h` | `67` | `Envelope 0 Sustain` | `Sustain volume` |
`10` | `include/Envelopes.h` | `68` | `Envelope 0 Release` | `Release time` |
`10` | `include/Envelopes.h` | `69` | `Envelope 0 HD Vibrato` | `Hold-Decay phase Vibrato sync rate` |
`10` | `include/Envelopes.h` | `70` | `Envelope 0 SR Vibrato` | `Sustain-Release phase Vibrato sync rate` |
`10` | `include/Envelopes.h` | `71` | `Envelope 0 Trigger on` | `Trigger/LFO settings: 0->19 = trigger #, 20 = off, 32->51 = trigger #+loop, 64->83 = trigger #+invert, 96->115 = trigger #+loop+invert` |
`10` | `include/Envelopes.h` | `72` | `Envelope 1 Attack` | `Attack time` |
`10` | `include/Envelopes.h` | `73` | `Envelope 1 Hold` | `Hold after attack time` |
`10` | `include/Envelopes.h` | `74` | `Envelope 1 Decay` | `Decay time` |
`10` | `include/Envelopes.h` | `75` | `Envelope 1 Sustain` | `Sustain volume` |
`10` | `include/Envelopes.h` | `76` | `Envelope 1 Release` | `Release time` |
`10` | `include/Envelopes.h` | `77` | `Envelope 1 HD Vibrato` | `Hold-Decay phase Vibrato sync rate` |
`10` | `include/Envelopes.h` | `78` | `Envelope 1 SR Vibrato` | `Sustain-Release phase Vibrato sync rate` |
`10` | `include/Envelopes.h` | `79` | `Envelope 1 Trigger on` | `Trigger/LFO settings: 0->19 = trigger #, 20 = off, 32->51 = trigger #+loop, 64->83 = trigger #+invert, 96->115 = trigger #+loop+invert` |
`10` | `include/Envelopes.h` | `80` | `Envelope 2 Attack` | `Attack time` |
`10` | `include/Envelopes.h` | `81` | `Envelope 2 Hold` | `Hold after attack time` |
`10` | `include/Envelopes.h` | `82` | `Envelope 2 Decay` | `Decay time` |
`10` | `include/Envelopes.h` | `83` | `Envelope 2 Sustain` | `Sustain volume` |
`10` | `include/Envelopes.h` | `84` | `Envelope 2 Release` | `Release time` |
`10` | `include/Envelopes.h` | `85` | `Envelope 2 HD Vibrato` | `Hold-Decay phase Vibrato sync rate` |
`10` | `include/Envelopes.h` | `86` | `Envelope 2 SR Vibrato` | `Sustain-Release phase Vibrato sync rate` |
`10` | `include/Envelopes.h` | `87` | `Envelope 2 Trigger on` | `Trigger/LFO settings: 0->19 = trigger #, 20 = off, 32->51 = trigger #+loop, 64->83 = trigger #+invert, 96->115 = trigger #+loop+invert` |
`10` | `include/Envelopes.h` | `88` | `Envelope 3 Attack` | `Attack time` |
`10` | `include/Envelopes.h` | `89` | `Envelope 3 Hold` | `Hold after attack time` |
`10` | `include/Envelopes.h` | `90` | `Envelope 3 Decay` | `Decay time` |
`10` | `include/Envelopes.h` | `91` | `Envelope 3 Sustain` | `Sustain volume` |
`10` | `include/Envelopes.h` | `92` | `Envelope 3 Release` | `Release time` |
`10` | `include/Envelopes.h` | `93` | `Envelope 3 HD Vibrato` | `Hold-Decay phase Vibrato sync rate` |
`10` | `include/Envelopes.h` | `94` | `Envelope 3 SR Vibrato` | `Sustain-Release phase Vibrato sync rate` |
`10` | `include/Envelopes.h` | `95` | `Envelope 3 Trigger on` | `Trigger/LFO settings: 0->19 = trigger #, 20 = off, 32->51 = trigger #+loop, 64->83 = trigger #+invert, 96->115 = trigger #+loop+invert` |
`10` | `include/Envelopes.h` | `96` | `Envelope 4 Attack` | `Attack time` |
`10` | `include/Envelopes.h` | `97` | `Envelope 4 Hold` | `Hold after attack time` |
`10` | `include/Envelopes.h` | `98` | `Envelope 4 Decay` | `Decay time` |
`10` | `include/Envelopes.h` | `99` | `Envelope 4 Sustain` | `Sustain volume` |
`10` | `include/Envelopes.h` | `100` | `Envelope 4 Release` | `Release time` |
`10` | `include/Envelopes.h` | `101` | `Envelope 4 HD Vibrato` | `Hold-Decay phase Vibrato sync rate` |
`10` | `include/Envelopes.h` | `102` | `Envelope 4 SR Vibrato` | `Sustain-Release phase Vibrato sync rate` |
`10` | `include/Envelopes.h` | `103` | `Envelope 4 Trigger on` | `Trigger/LFO settings: 0->19 = trigger #, 20 = off, 32->51 = trigger #+loop, 64->83 = trigger #+invert, 96->115 = trigger #+loop+invert` |
`10` | `include/Harmony.hpp` | `105` | `CC_AUTO_CHORD_TYPE` | ` enable/disable playing automatic chord types (ie stacking triads)` |
`10` | `include/Harmony.hpp` | `106` | `CC_AUTO_CHORD_INVERSION` | ` enable/disable playing automatic chord inversions` |
`10` | `include/Harmony.hpp` | `107` | `CC_MELODY_OCTAVE_OFFSET` | ` output 1: octave offset for melody, 0=-2, 1=-1, 2=0, 3=+1, 4=+2, 5=+3` |
`10` | `include/Harmony.hpp` | `108` | `CC_MELODY_SCALE` | ` choose scale to use, 0=major, 1=natural minor, 2=melodic minor, 3=harmonic minor, 4=lydian, 5=whole tone, 6=blues` |
`10` | `include/Harmony.hpp` | `109` | `CC_MELODY_AUTO_SCALE` | ` enable/disable automatic changing of scale every phrase` |
`10` | `include/MidiInput.hpp` | `110` | `CC_SYNC_RATIO` | ` sync ratio of the envelope modulation in ticks, default 24` |
`10` | `include/MidiInput.hpp` | `111` | `CC_CLOCK_TICK_RATIO` | ` send a clock tick for every X clock ticks, default 1` |
`10` | `include/Harmony.hpp` | `112` | `CC_PAD_ROOT_MELODY_MODE` | ` output 2: set mode to use for the chords output - 0=None, 1=Single note, 2=Chord, 3=Arpeggiate chord` |
`10` | `include/Harmony.hpp` | `113` | `CC_PAD_ROOT_OCTAVE_OFFSET` | ` output 2: octave offset for melody, 0=-2, 1=-1, 2=0, 3=+1, 4=+2, 5=+3` |
`10` | `include/Euclidian.h` | `114` | `CC_EUCLIDIAN_DENSITY` | ` experimental: global density of euclidian patterns` |
`10` | `include/Drums.h` | `123` | `MIDI_CC_ALL_NOTES_OFF` | `` |
| ---- | ---- | -- | ---- | ------- |
`11` | `include/Envelopes.h` | `64` | `Envelope 5 Attack` | `Attack time (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `65` | `Envelope 5 Hold` | `Hold after attack time (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `66` | `Envelope 5 Decay` | `Decay time (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `67` | `Envelope 5 Sustain` | `Sustain volume (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `68` | `Envelope 5 Release` | `Release time (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `69` | `Envelope 5 HD Vibrato` | `Hold-Decay phase Vibrato sync rate (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `70` | `Envelope 5 SR Vibrato` | `Sustain-Release phase Vibrato sync rate (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `71` | `Envelope 5 Trigger on` | `Trigger/LFO settings: 0->19 = trigger #, 20 = off, 32->51 = trigger #+loop, 64->83 = trigger #+invert, 96->115 = trigger #+loop+invert (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `72` | `Envelope 6 Attack` | `Attack time (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `73` | `Envelope 6 Hold` | `Hold after attack time (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `74` | `Envelope 6 Decay` | `Decay time (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `75` | `Envelope 6 Sustain` | `Sustain volume (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `76` | `Envelope 6 Release` | `Release time (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `77` | `Envelope 6 HD Vibrato` | `Hold-Decay phase Vibrato sync rate (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `78` | `Envelope 6 SR Vibrato` | `Sustain-Release phase Vibrato sync rate (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `79` | `Envelope 6 Trigger on` | `Trigger/LFO settings: 0->19 = trigger #, 20 = off, 32->51 = trigger #+loop, 64->83 = trigger #+invert, 96->115 = trigger #+loop+invert (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `80` | `Envelope 7 Attack` | `Attack time (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `81` | `Envelope 7 Hold` | `Hold after attack time (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `82` | `Envelope 7 Decay` | `Decay time (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `83` | `Envelope 7 Sustain` | `Sustain volume (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `84` | `Envelope 7 Release` | `Release time (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `85` | `Envelope 7 HD Vibrato` | `Hold-Decay phase Vibrato sync rate (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `86` | `Envelope 7 SR Vibrato` | `Sustain-Release phase Vibrato sync rate (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `87` | `Envelope 7 Trigger on` | `Trigger/LFO settings: 0->19 = trigger #, 20 = off, 32->51 = trigger #+loop, 64->83 = trigger #+invert, 96->115 = trigger #+loop+invert (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `88` | `Envelope 8 Attack` | `Attack time (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `89` | `Envelope 8 Hold` | `Hold after attack time (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `90` | `Envelope 8 Decay` | `Decay time (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `91` | `Envelope 8 Sustain` | `Sustain volume (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `92` | `Envelope 8 Release` | `Release time (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `93` | `Envelope 8 HD Vibrato` | `Hold-Decay phase Vibrato sync rate (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `94` | `Envelope 8 SR Vibrato` | `Sustain-Release phase Vibrato sync rate (Extended pitch envelopes on Chan 11)` |
`11` | `include/Envelopes.h` | `95` | `Envelope 8 Trigger on` | `Trigger/LFO settings: 0->19 = trigger #, 20 = off, 32->51 = trigger #+loop, 64->83 = trigger #+invert, 96->115 = trigger #+loop+invert (Extended pitch envelopes on Chan 11)` |
| ---- | ---- | -- | ---- | ------- |

----

Done.

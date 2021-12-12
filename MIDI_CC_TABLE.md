| Chan | File | CC | Name | Comment |
| ---- | ---- | -- | ---- | ------- |
`10` | `Harmony.hpp` | `2` | `CC_BASS_MELODY_MODE` | ` output 0: set mode to use for the bass output - 0=None, 1=Single note, 2=Chord, 3=Arpeggiate chord` |
`10` | `Harmony.hpp` | `3` | `CC_PAD_PITCH_MELODY_MODE` | ` output 3: set mode to use for the pitch output - 0=None, 1=Single note, 2=Chord, 3=Arpeggiate chord` |
`10` | `Harmony.hpp` | `4` | `CC_PAD_PITCH_OCTAVE_OFFSET` | ` output 3: set octave offset (-2 to +3)` |
`10` | `Harmony.hpp` | `5` | `CC_CHANNEL_PAD_PITCH` | ` output 3: set the MIDI channel to output the pads pitch on (default 2)` |
`10` | `Harmony.hpp` | `6` | `CC_BASS_SET_TIE_ON` | ` output 0: set which steps the Euclidian bass should tie on` |
`10` | `Euclidian.h` | `8` | `CC_EUCLIDIAN_CLAP_FLAM` | ` enable/disable clap flam, 0=off` |
`10` | `Euclidian.h` | `9` | `CC_EUCLIDIAN_HIHAT_SHUFF` | ` enable/disable hihat shuffle, 0=off` |
`10` | `Harmony.hpp` | `10` | `CC_CHANNEL_PAD_ROOT` | ` output 2: set the MIDI channel to output the pad root on (default 1)` |
`10` | `Harmony.hpp` | `11` | `CC_MELODY_ROOT` | ` set the MIDI note to use as the root pitch, 48=C4` |
`10` | `Harmony.hpp` | `12` | `CC_CHANNEL_BASS_OUT` | ` output 0: set the MIDI channel to output the bass on (default 4)` |
`10` | `Harmony.hpp` | `13` | `CC_CHANNEL_BITBOX_KEYS` | ` output 1: set the MIDI channel to output the chords on (default 3)` |
`10` | `UI.ino` | `14` | `CC_CHANNEL_BITBOX_DRUMS_OUT` | ` set the MIDI channel to output the shadow drum triggers (default 11)` |
`10` | `UI.ino` | `15` | `CC_CHANNEL_GATE_OUT` | ` set the MIDI channel to output the Muso drum triggers (default 16)` |
`10` | `Euclidian.h` | `16` | `CC_EUCLIDIAN_SET_AUTO_PLAY` | ` enable/disable autoplaying on internal BPM` |
`10` | `Harmony.hpp` | `17` | `CC_BASS_SET_ARP_MODE` | ` cc to set the bass arp mode` |
`10` | `Harmony.hpp` | `18` | `CC_HARMONY_ONLY_NOTE_HELD` | ` cc to set bass to only play in external mode if note is held` |
`10` | `UI.h` | `19` | `CC_DEMO_MODE` | ` choose playback mode - 0=None, 1=Euclidian, 2=Euclidian with mutation, 3=Random triggers` |
`10` | `Euclidian.h` | `20` | `CC_EUCLIDIAN_SET_MUTATE_MODE` | ` 0 = NONE, 1 = SUBTLE, 2 = TOTAL` |
`10` | `MidiEcho.h` | `21` | `CC_MIDIECHO_ENABLED` | ` enable echoing of output back to DAW over USB` |
`10` | `Euclidian.h` | `22` | `CC_EUCLIDIAN_SEED_MODIFIER` | ` euclidian random "bank" X` |
`10` | `Euclidian.h` | `23` | `CC_EUCLIDIAN_SEED_MODIFIER_2` | ` euclidian random "bank" Y` |
`10` | `Euclidian.h` | `24` | `CC_EUCLIDIAN_RESET_BEFORE_MUTATE` | ` reset sequences to 0 before mutation?` |
`10` | `Euclidian.h` | `25` | `CC_EUCLIDIAN_SET_MINIMUM_PATTERN` | ` lowest number pattern to automutate` |
`10` | `Euclidian.h` | `26` | `CC_EUCLIDIAN_SET_MAXIMUM_PATTERN` | ` highest number pattern to automutate` |
`10` | `Euclidian.h` | `27` | `CC_EUCLIDIAN_SEED_USE_PHRASE` | ` increment seed according to BPM phrase (ie automutate)` |
`10` | `Euclidian.h` | `28` | `CC_EUCLIDIAN_FILLS` | ` enable/disable fills on last bar of phrase, 0=off` |
`10` | `Harmony.hpp` | `29` | `CC_HARMONY_MELODY_MODE` | ` output 1: set mode to use for the chords output - 0=None, 1=Single note, 2=Chord, 3=Arpeggiate chord` |
`10` | `Harmony.hpp` | `30` | `CC_HARMONY_MUTATE_MODE` | ` overall harmony mutation mode, 0=None, 1=Randomise` |
`10` | `Harmony.hpp` | `31` | `CC_AUTO_PROGRESSION` | ` enable/disable playing auto chord progression` |
`10` | `Euclidian.h` | `32` | `Euclidian on/off for pattern 0` | `` |
`10` | `Euclidian.h` | `33` | `Euclidian on/off for pattern 1` | `` |
`10` | `Euclidian.h` | `34` | `Euclidian on/off for pattern 2` | `` |
`10` | `Euclidian.h` | `35` | `Euclidian on/off for pattern 3` | `` |
`10` | `Euclidian.h` | `36` | `Euclidian on/off for pattern 4` | `` |
`10` | `Euclidian.h` | `37` | `Euclidian on/off for pattern 5` | `` |
`10` | `Euclidian.h` | `38` | `Euclidian on/off for pattern 6` | `` |
`10` | `Euclidian.h` | `39` | `Euclidian on/off for pattern 7` | `` |
`10` | `Euclidian.h` | `40` | `Euclidian on/off for pattern 8` | `` |
`10` | `Euclidian.h` | `41` | `Euclidian on/off for pattern 9` | `` |
`10` | `Euclidian.h` | `42` | `Euclidian on/off for pattern 10` | `` |
`10` | `Euclidian.h` | `43` | `Euclidian on/off for pattern 11` | `` |
`10` | `Euclidian.h` | `44` | `Euclidian on/off for pattern 12` | `` |
`10` | `Euclidian.h` | `45` | `Euclidian on/off for pattern 13` | `` |
`10` | `Euclidian.h` | `46` | `Euclidian on/off for pattern 14` | `` |
`10` | `Euclidian.h` | `47` | `Euclidian on/off for pattern 15` | `` |
`10` | `Euclidian.h` | `48` | `Euclidian on/off for pattern 16` | `` |
`10` | `Euclidian.h` | `49` | `Euclidian on/off for pattern 17` | `` |
`10` | `Euclidian.h` | `50` | `Euclidian on/off for pattern 18` | `` |
`10` | `Euclidian.h` | `51` | `Euclidian on/off for pattern 19` | `` |
`10` | `Envelopes.h` | `64` | `Envelope 0 ATTACK` | `Chan 10` |
`10` | `Envelopes.h` | `65` | `Envelope 0 HOLD` | `Chan 10` |
`10` | `Envelopes.h` | `66` | `Envelope 0 DECAY` | `Chan 10` |
`10` | `Envelopes.h` | `67` | `Envelope 0 SUSTAIN` | `Chan 10` |
`10` | `Envelopes.h` | `68` | `Envelope 0 RELEASE` | `Chan 10` |
`10` | `Envelopes.h` | `69` | `Envelope 0 HD_VIB` | `Chan 10` |
`10` | `Envelopes.h` | `70` | `Envelope 0 SR_VIB` | `Chan 10` |
`10` | `Envelopes.h` | `71` | `Envelope 0 TRIGGER_ON_MIDI_CHANNEL` | `Chan 10` |
`10` | `Envelopes.h` | `72` | `Envelope 1 ATTACK` | `Chan 10` |
`10` | `Envelopes.h` | `73` | `Envelope 1 HOLD` | `Chan 10` |
`10` | `Envelopes.h` | `74` | `Envelope 1 DECAY` | `Chan 10` |
`10` | `Envelopes.h` | `75` | `Envelope 1 SUSTAIN` | `Chan 10` |
`10` | `Envelopes.h` | `76` | `Envelope 1 RELEASE` | `Chan 10` |
`10` | `Envelopes.h` | `77` | `Envelope 1 HD_VIB` | `Chan 10` |
`10` | `Envelopes.h` | `78` | `Envelope 1 SR_VIB` | `Chan 10` |
`10` | `Envelopes.h` | `79` | `Envelope 1 TRIGGER_ON_MIDI_CHANNEL` | `Chan 10` |
`10` | `Envelopes.h` | `80` | `Envelope 2 ATTACK` | `Chan 10` |
`10` | `Envelopes.h` | `81` | `Envelope 2 HOLD` | `Chan 10` |
`10` | `Envelopes.h` | `82` | `Envelope 2 DECAY` | `Chan 10` |
`10` | `Envelopes.h` | `83` | `Envelope 2 SUSTAIN` | `Chan 10` |
`10` | `Envelopes.h` | `84` | `Envelope 2 RELEASE` | `Chan 10` |
`10` | `Envelopes.h` | `85` | `Envelope 2 HD_VIB` | `Chan 10` |
`10` | `Envelopes.h` | `86` | `Envelope 2 SR_VIB` | `Chan 10` |
`10` | `Envelopes.h` | `87` | `Envelope 2 TRIGGER_ON_MIDI_CHANNEL` | `Chan 10` |
`10` | `Envelopes.h` | `88` | `Envelope 3 ATTACK` | `Chan 10` |
`10` | `Envelopes.h` | `89` | `Envelope 3 HOLD` | `Chan 10` |
`10` | `Envelopes.h` | `90` | `Envelope 3 DECAY` | `Chan 10` |
`10` | `Envelopes.h` | `91` | `Envelope 3 SUSTAIN` | `Chan 10` |
`10` | `Envelopes.h` | `92` | `Envelope 3 RELEASE` | `Chan 10` |
`10` | `Envelopes.h` | `93` | `Envelope 3 HD_VIB` | `Chan 10` |
`10` | `Envelopes.h` | `94` | `Envelope 3 SR_VIB` | `Chan 10` |
`10` | `Envelopes.h` | `95` | `Envelope 3 TRIGGER_ON_MIDI_CHANNEL` | `Chan 10` |
`10` | `Envelopes.h` | `96` | `Envelope 4 ATTACK` | `Chan 10` |
`10` | `Envelopes.h` | `97` | `Envelope 4 HOLD` | `Chan 10` |
`10` | `Envelopes.h` | `98` | `Envelope 4 DECAY` | `Chan 10` |
`10` | `Envelopes.h` | `99` | `Envelope 4 SUSTAIN` | `Chan 10` |
`10` | `Envelopes.h` | `100` | `Envelope 4 RELEASE` | `Chan 10` |
`10` | `Envelopes.h` | `101` | `Envelope 4 HD_VIB` | `Chan 10` |
`10` | `Envelopes.h` | `102` | `Envelope 4 SR_VIB` | `Chan 10` |
`10` | `Envelopes.h` | `103` | `Envelope 4 TRIGGER_ON_MIDI_CHANNEL` | `Chan 10` |
`10` | `Harmony.hpp` | `105` | `CC_AUTO_CHORD_TYPE` | ` enable/disable playing automatic chord types (ie stacking triads)` |
`10` | `Harmony.hpp` | `106` | `CC_AUTO_CHORD_INVERSION` | ` enable/disable playing automatic chord inversions` |
`10` | `Harmony.hpp` | `107` | `CC_MELODY_OCTAVE_OFFSET` | ` output 1: octave offset for melody, 0=-2, 1=-1, 2=0, 3=+1, 4=+2, 5=+3` |
`10` | `Harmony.hpp` | `108` | `CC_MELODY_SCALE` | ` choose scale to use, 0=major, 1=natural minor, 2=melodic minor, 3=harmonic minor, 4=lydian, 5=whole tone, 6=blues` |
`10` | `Harmony.hpp` | `109` | `CC_MELODY_AUTO_SCALE` | ` enable/disable automatic changing of scale every phrase` |
`10` | `MidiInput.hpp` | `110` | `CC_SYNC_RATIO` | ` sync ratio of the envelope modulation in ticks, default 24` |
`10` | `MidiInput.hpp` | `111` | `CC_CLOCK_TICK_RATIO` | ` send a clock tick for every X clock ticks, default 1` |
`10` | `Harmony.hpp` | `112` | `CC_PAD_ROOT_MELODY_MODE` | ` output 2: set mode to use for the chords output - 0=None, 1=Single note, 2=Chord, 3=Arpeggiate chord` |
`10` | `Harmony.hpp` | `113` | `CC_PAD_ROOT_OCTAVE_OFFSET` | ` output 2: octave offset for melody, 0=-2, 1=-1, 2=0, 3=+1, 4=+2, 5=+3` |
`10` | `Euclidian.h` | `114` | `CC_EUCLIDIAN_DENSITY` | ` experimental: global density of euclidian patterns` |
`10` | `Drums.h` | `123` | `MIDI_CC_ALL_NOTES_OFF` | `` |
| ---- | ---- | -- | ---- | ------- |
`11` | `Envelopes.h` | `64` | `Envelope 5 ATTACK` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `65` | `Envelope 5 HOLD` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `66` | `Envelope 5 DECAY` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `67` | `Envelope 5 SUSTAIN` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `68` | `Envelope 5 RELEASE` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `69` | `Envelope 5 HD_VIB` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `70` | `Envelope 5 SR_VIB` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `71` | `Envelope 5 TRIGGER_ON_MIDI_CHANNEL` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `72` | `Envelope 6 ATTACK` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `73` | `Envelope 6 HOLD` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `74` | `Envelope 6 DECAY` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `75` | `Envelope 6 SUSTAIN` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `76` | `Envelope 6 RELEASE` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `77` | `Envelope 6 HD_VIB` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `78` | `Envelope 6 SR_VIB` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `79` | `Envelope 6 TRIGGER_ON_MIDI_CHANNEL` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `80` | `Envelope 7 ATTACK` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `81` | `Envelope 7 HOLD` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `82` | `Envelope 7 DECAY` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `83` | `Envelope 7 SUSTAIN` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `84` | `Envelope 7 RELEASE` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `85` | `Envelope 7 HD_VIB` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `86` | `Envelope 7 SR_VIB` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `87` | `Envelope 7 TRIGGER_ON_MIDI_CHANNEL` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `88` | `Envelope 8 ATTACK` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `89` | `Envelope 8 HOLD` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `90` | `Envelope 8 DECAY` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `91` | `Envelope 8 SUSTAIN` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `92` | `Envelope 8 RELEASE` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `93` | `Envelope 8 HD_VIB` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `94` | `Envelope 8 SR_VIB` | `Extended pitch envelopes on Chan 11` |
`11` | `Envelopes.h` | `95` | `Envelope 8 TRIGGER_ON_MIDI_CHANNEL` | `Extended pitch envelopes on Chan 11` |
| ---- | ---- | -- | ---- | ------- |

----

Done.

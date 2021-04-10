| File | CC | Name | Comment |
| ---- | -- | ---- | ------- |
| `Harmony.hpp` | `5` | `CC_CHANNEL_PAD_PITCH` | ` set the MIDI channel to output the pads pitch on (default 2)` |
| `Harmony.hpp` | `6` | `CC_BASS_SET_TIE_ON` | ` set which steps the Euclidian bass should tie on` |
| `Euclidian.h` | `8` | `CC_EUCLIDIAN_CLAP_FLAM` | ` enable/disable clap flam, 0=off` |
| `Euclidian.h` | `9` | `CC_EUCLIDIAN_HIHAT_SHUFF` | ` enable/disable hihat shuffle, 0=off` |
| `Harmony.hpp` | `10` | `CC_CHANNEL_PAD_ROOT` | ` set the MIDI channel to output the pad root on (default 1)` |
| `Harmony.hpp` | `11` | `CC_MELODY_ROOT` | ` set the MIDI note to use as the root pitch, 48=C4` |
| `Harmony.hpp` | `12` | `CC_CHANNEL_BASS_OUT` | ` set the MIDI channel to output the bass on (default 4)` |
| `Harmony.hpp` | `13` | `CC_CHANNEL_BITBOX_KEYS` | ` set the MIDI channel to output the chords on (default 3)` |
| `UI.ino` | `14` | `CC_CHANNEL_BITBOX_OUT` | ` set the MIDI channel to output the shadow drum triggers (default 11)` |
| `UI.ino` | `15` | `CC_CHANNEL_GATE_OUT` | ` set the MIDI channel to output the Muso drum triggers (default 16)` |
| `Euclidian.h` | `16` | `CC_EUCLIDIAN_SET_AUTO_PLAY` | ` enable/disable autoplaying on internal BPM` |
| `Harmony.hpp` | `17` | `CC_BASS_SET_ARP_MODE` | ` cc to set the bass arp mode` |
| `Harmony.hpp` | `18` | `CC_BASS_ONLY_NOTE_HELD` | ` cc to set bass to only play in external mode if note is held` |
| `UI.h` | `19` | `CC_DEMO_MODE` | ` choose playback mode - 0=None, 1=Euclidian, 2=Euclidian with mutation, 3=Random triggers` |
| `Euclidian.h` | `20` | `CC_EUCLIDIAN_SET_MUTATE_MODE` | ` 0 = NONE, 1 = SUBTLE, 2 = TOTAL` |
| `MidiEcho.h` | `21` | `CC_MIDIECHO_ENABLED` | ` enable echoing of output back to DAW over USB` |
| `Euclidian.h` | `22` | `CC_EUCLIDIAN_SEED_MODIFIER` | ` euclidian random "bank" X` |
| `Euclidian.h` | `23` | `CC_EUCLIDIAN_SEED_MODIFIER_2` | ` euclidian random "bank" Y` |
| `Euclidian.h` | `24` | `CC_EUCLIDIAN_RESET_BEFORE_MUTATE` | ` reset sequences to 0 before mutation?` |
| `Euclidian.h` | `25` | `CC_EUCLIDIAN_SET_MINIMUM_PATTERN` | ` lowest number pattern to automutate` |
| `Euclidian.h` | `26` | `CC_EUCLIDIAN_SET_MAXIMUM_PATTERN` | ` highest number pattern to automutate` |
| `Euclidian.h` | `27` | `CC_EUCLIDIAN_SEED_USE_PHRASE` | ` increment seed according to BPM phrase (ie automutate)` |
| `Euclidian.h` | `28` | `CC_EUCLIDIAN_FILLS` | ` enable/disable fills on last bar of phrase, 0=off` |
| `Harmony.hpp` | `29` | `CC_HARMONY_MELODY_MODE` | ` set mode to use for the chords output - 0=None, 1=Single note, 2=Chord, 3=Arpeggiate chord` |
| `Harmony.hpp` | `30` | `CC_HARMONY_MUTATE_MODE` | ` harmony mutation mode, 0=None, 1=Randomise` |
| `Harmony.hpp` | `31` | `CC_AUTO_PROGRESSION` | ` enable/disable playing auto chord progression` |
| `Euclidian.h` | `32` | `Euclidian on/off for pattern 0` | `` |
| `Euclidian.h` | `33` | `Euclidian on/off for pattern 1` | `` |
| `Euclidian.h` | `34` | `Euclidian on/off for pattern 2` | `` |
| `Euclidian.h` | `35` | `Euclidian on/off for pattern 3` | `` |
| `Euclidian.h` | `36` | `Euclidian on/off for pattern 4` | `` |
| `Euclidian.h` | `37` | `Euclidian on/off for pattern 5` | `` |
| `Euclidian.h` | `38` | `Euclidian on/off for pattern 6` | `` |
| `Euclidian.h` | `39` | `Euclidian on/off for pattern 7` | `` |
| `Euclidian.h` | `40` | `Euclidian on/off for pattern 8` | `` |
| `Euclidian.h` | `41` | `Euclidian on/off for pattern 9` | `` |
| `Euclidian.h` | `42` | `Euclidian on/off for pattern 10` | `` |
| `Euclidian.h` | `43` | `Euclidian on/off for pattern 11` | `` |
| `Euclidian.h` | `44` | `Euclidian on/off for pattern 12` | `` |
| `Euclidian.h` | `45` | `Euclidian on/off for pattern 13` | `` |
| `Euclidian.h` | `46` | `Euclidian on/off for pattern 14` | `` |
| `Euclidian.h` | `47` | `Euclidian on/off for pattern 15` | `` |
| `Euclidian.h` | `48` | `Euclidian on/off for pattern 16` | `` |
| `Euclidian.h` | `49` | `Euclidian on/off for pattern 17` | `` |
| `Euclidian.h` | `50` | `Euclidian on/off for pattern 18` | `` |
| `Euclidian.h` | `51` | `Euclidian on/off for pattern 19` | `` |
| `Envelopes.h` | `64` | `Envelope 0 ATTACK` | `` |
| `Envelopes.h` | `65` | `Envelope 0 HOLD` | `` |
| `Envelopes.h` | `66` | `Envelope 0 DECAY` | `` |
| `Envelopes.h` | `67` | `Envelope 0 SUSTAIN` | `` |
| `Envelopes.h` | `68` | `Envelope 0 RELEASE` | `` |
| `Envelopes.h` | `69` | `Envelope 0 HD_VIB` | `` |
| `Envelopes.h` | `70` | `Envelope 0 SR_VIB` | `` |
| `Envelopes.h` | `71` | `Envelope 0 RESERVED` | `` |
| `Envelopes.h` | `72` | `Envelope 1 ATTACK` | `` |
| `Envelopes.h` | `73` | `Envelope 1 HOLD` | `` |
| `Envelopes.h` | `74` | `Envelope 1 DECAY` | `` |
| `Envelopes.h` | `75` | `Envelope 1 SUSTAIN` | `` |
| `Envelopes.h` | `76` | `Envelope 1 RELEASE` | `` |
| `Envelopes.h` | `77` | `Envelope 1 HD_VIB` | `` |
| `Envelopes.h` | `78` | `Envelope 1 SR_VIB` | `` |
| `Envelopes.h` | `79` | `Envelope 1 RESERVED` | `` |
| `Envelopes.h` | `80` | `Envelope 2 ATTACK` | `` |
| `Envelopes.h` | `81` | `Envelope 2 HOLD` | `` |
| `Envelopes.h` | `82` | `Envelope 2 DECAY` | `` |
| `Envelopes.h` | `83` | `Envelope 2 SUSTAIN` | `` |
| `Envelopes.h` | `84` | `Envelope 2 RELEASE` | `` |
| `Envelopes.h` | `85` | `Envelope 2 HD_VIB` | `` |
| `Envelopes.h` | `86` | `Envelope 2 SR_VIB` | `` |
| `Envelopes.h` | `87` | `Envelope 2 RESERVED` | `` |
| `Envelopes.h` | `88` | `Envelope 3 ATTACK` | `` |
| `Envelopes.h` | `89` | `Envelope 3 HOLD` | `` |
| `Envelopes.h` | `90` | `Envelope 3 DECAY` | `` |
| `Envelopes.h` | `91` | `Envelope 3 SUSTAIN` | `` |
| `Envelopes.h` | `92` | `Envelope 3 RELEASE` | `` |
| `Envelopes.h` | `93` | `Envelope 3 HD_VIB` | `` |
| `Envelopes.h` | `94` | `Envelope 3 SR_VIB` | `` |
| `Envelopes.h` | `95` | `Envelope 3 RESERVED` | `` |
| `Envelopes.h` | `96` | `Envelope 4 ATTACK` | `` |
| `Envelopes.h` | `97` | `Envelope 4 HOLD` | `` |
| `Envelopes.h` | `98` | `Envelope 4 DECAY` | `` |
| `Envelopes.h` | `99` | `Envelope 4 SUSTAIN` | `` |
| `Envelopes.h` | `100` | `Envelope 4 RELEASE` | `` |
| `Envelopes.h` | `101` | `Envelope 4 HD_VIB` | `` |
| `Envelopes.h` | `102` | `Envelope 4 SR_VIB` | `` |
| `Envelopes.h` | `103` | `Envelope 4 RESERVED` | `` |
| `Harmony.hpp` | `105` | `CC_AUTO_CHORD_TYPE` | ` enable/disable playing automatic chord types (ie stacking triads)` |
| `Harmony.hpp` | `106` | `CC_AUTO_CHORD_INVERSION` | ` enable/disable playing automatic chord inversions` |
| `Harmony.hpp` | `107` | `CC_MELODY_OCTAVE_OFFSET` | ` octave offset for melody, 0=-2, 1=-1, 2=0, 3=+1, 4=+2, 5=+3` |
| `Harmony.hpp` | `108` | `CC_MELODY_SCALE` | ` choose scale to use, 0=major, 1=natural minor, 2=melodic minor, 3=harmonic minor, 4=lydian, 5=whole tone, 6=blues` |
| `Harmony.hpp` | `109` | `CC_MELODY_AUTO_SCALE` | ` enable/disable automatic changing of scale every phrase` |
| `MidiInput.hpp` | `110` | `CC_SYNC_RATIO` | ` sync ratio of the envelope modulation in ticks, default 24` |
| `MidiInput.hpp` | `111` | `CC_CLOCK_TICK_RATIO` | ` send a clock tick for every X clock ticks, default 1` |
| `Drums.h` | `123` | `MIDI_CC_ALL_NOTES_OFF` | `` |

#ifndef MIDIECHO_INCLUDED
#define MIDIECHO_INCLUDED

int get_pitch_for_trigger(int trigger);
int get_trigger_for_pitch(int pitch);
void echo_fire_trigger(int trigger, int velocity);
void echo_douse_trigger(int trigger, int velocity);

String get_note_name(int pitch);

#endif

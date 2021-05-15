#!/bin/sh

grep "^#define" * | grep CC > CHANNEL_AND_CC_LIST.txt

echo >> CHANNEL_AND_CC_LIST.txt

grep "^#define" * | grep "MIDI_CHANNEL" >> CHANNEL_AND_CC_LIST.txt

echo "Result written to file:\n"
cat CHANNEL_AND_CC_LIST.txt

#!/bin/sh

grep "^#define" src/* | grep CC > CHANNEL_AND_CC_LIST.txt
grep "^#define" include/* | grep CC > CHANNEL_AND_CC_LIST.txt

echo >> CHANNEL_AND_CC_LIST.txt

grep "^#define" src/* | grep "MIDI_CHANNEL" >> CHANNEL_AND_CC_LIST.txt
grep "^#define" include/* | grep "MIDI_CHANNEL" >> CHANNEL_AND_CC_LIST.txt

echo >> CHANNEL_AND_CC_LIST.txt

grep "^#define" src/* | grep "TRIGGER_CHANNEL" >> CHANNEL_AND_CC_LIST.txt
grep "^#define" include/* | grep "TRIGGER_CHANNEL" >> CHANNEL_AND_CC_LIST.txt

echo "Result written to file:\n"
cat CHANNEL_AND_CC_LIST.txt

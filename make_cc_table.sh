#!/bin/sh
./build_cc_list.sh
./build_cc_table.py < CHANNEL_AND_CC_LIST.txt  > MIDI_CC_TABLE.md

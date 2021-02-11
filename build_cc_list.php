#!/bin/sh

grep "^#define" * | grep CC > CHANNEL_AND_CC_LIST.md

echo >> CHANNEL_AND_CC_LIST.md

grep "^#define" * | grep "_CHANNEL_" >> CHANNEL_AND_CC_LIST.md

echo "Result written to file:\n"
cat CHANNEL_AND_CC_LIST.md

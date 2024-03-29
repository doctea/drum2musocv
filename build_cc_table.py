#!/bin/python3
import fileinput
from pprint import pprint

CHANNEL = 10
CHANNEL_EXTENDED = 11

NUM_ENVS = 5
NUM_ENVS_EXTENDED = 9
ENV_SPAN = 8
NUM_PATTERNS = 20

table = {}
table[CHANNEL] = {}
table[CHANNEL_EXTENDED] = {}

env_controls = {
    'Attack': 'Attack time',
    'Hold': 'Hold after attack time',
    'Decay': 'Decay time',
    'Sustain': 'Sustain volume',
    'Release': 'Release time',
    'HD Vibrato': 'Hold-Decay phase Vibrato sync rate',
    'SR Vibrato': 'Sustain-Release phase Vibrato sync rate',
    'Trigger on': 'Trigger/LFO settings: 0->19 = trigger #, 20 = off, 32->51 = trigger #+loop, 64->83 = trigger #+invert, 96->115 = trigger #+loop+invert'
}

for line in fileinput.input():
    #print ("got line %s" % line)
    x = line.rstrip().split('//', 1)
    fp = x[0]
    comment = x[1] if len(x)>1 else ''
    x = fp.split() #maxsplit=1)
    #pprint(x)
    if len(x)==0:
        continue
    file = x[0].replace(':#define','')
    defname = x[1] #.replace("#define","")
    if defname=="MUSO_USE_PITCH_FOR" or (' CC_' not in line and '_CC' not in line) or 'CC_CV' in line: #defname:
        continue
    if defname=="CC_EUCLIDIAN_ACTIVE_STATUS_END":
        # do something clever here to build the list
        for e in range(int(cc), int(cc)+NUM_PATTERNS):
            table[CHANNEL][e] = { 'defname': 'Euclidian on/off for pattern %s' % (e-int(cc)), 'file': file, 'comment': '' }
        continue
    if defname=="ENV_CC_START":
        c = int(x[2])
        for e in range(NUM_ENVS):
            for k,v in env_controls.items():
                table[CHANNEL][c] = { 'defname': 'Envelope %s %s' % (e, k), 'file': file, 'comment': v } #'Chan %s'%CHANNEL }
                if e < NUM_ENVS_EXTENDED - NUM_ENVS: # do double-duty of completing channel 11 table too
                    table[CHANNEL_EXTENDED][c] = { 'defname': 'Envelope %s %s' % (e+NUM_ENVS, k), 'file': file, 'comment': "%s (Extended pitch envelopes on Chan %s)" % (v, CHANNEL_EXTENDED ) }
                c += 1
        continue
    #print ("got temp defname %s" % defname)
    #s = defname.split(":")
    #defname = s[1]
    #file = s[0]
    cc = x[2]

    #print ("got %s = %s (%s)" % (defname, int(cc), comment))
    table[CHANNEL][int(cc)] = { 'defname': defname, 'file': file, 'comment': comment}

#print ("table:")
#pprint(table)


print ("| Chan | File | CC | Name | Comment |")
print ("| ---- | ---- | -- | ---- | ------- |")
for chan in table.keys():
    for key in sorted(table[chan]):
        #print (key, table[key])
        print ("`%s` | `%s` | `%s` | `%s` | `%s` |" % (chan, table[chan][key]['file'], key, table[chan][key]['defname'], table[chan][key]['comment']))
    print ("| ---- | ---- | -- | ---- | ------- |")


print ("\n----\n")
print ("Done.")

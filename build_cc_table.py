#!/bin/python3
import fileinput
from pprint import pprint

NUM_ENVS = 5
ENV_SPAN = 8
NUM_PATTERNS = 20

table = {}

env_controls = [ 'ATTACK', 'HOLD', 'DECAY', 'SUSTAIN', 'RELEASE', 'HD_VIB', 'SR_VIB', 'RESERVED' ]

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
            table[e] = { 'defname': 'Euclidian on/off for pattern %s' % (e-int(cc)), 'file': file, 'comment': '' }
        continue
    if defname=="ENV_CC_START":
        c = int(x[2])
        for e in range(NUM_ENVS):
            for s in range(ENV_SPAN):
                table[c] = { 'defname': 'Envelope %s %s' % (e, env_controls[s]), 'file': file, 'comment': '' }
                c += 1
        continue
    #print ("got temp defname %s" % defname)
    #s = defname.split(":")
    #defname = s[1]
    #file = s[0]
    cc = x[2]

    #print ("got %s = %s (%s)" % (defname, int(cc), comment))
    table[int(cc)] = { 'defname': defname, 'file': file, 'comment': comment}


#pprint(table)


print ("| File | CC | Name | Comment |")
print ("| ---- | -- | ---- | ------- |")
for key in sorted(table):
    #print (key, table[key])
    print ("| `%s` | `%s` | `%s` | `%s` |" % (table[key]['file'], key, table[key]['defname'], table[key]['comment']))

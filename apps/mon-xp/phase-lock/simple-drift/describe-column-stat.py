#!/usr/bin/env python

import sys
import pandas

if len(sys.argv) != 1:
    print "usage: %s < col-data" % (sys.argv[0],)
    sys.exit(1)

d = sys.stdin.read()

try:
    d = d.strip()
    d = d.split('\n')
    d = map(float, d)
except:
    print "error, cannot convert datafile"
    sys.exit(1)

d = pandas.DataFrame(d)
print d.describe()

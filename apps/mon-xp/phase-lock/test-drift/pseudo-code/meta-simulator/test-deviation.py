import sys

# simply display the progression of the skipped/executed/total and instant deviation ratio with the legacy drift implementation.

deviation = float(sys.argv[1])

executed = 0
skipped  = 0
total    = 0

while True:
    if ((1.0 - deviation) * total) > skipped:
        skipped  += 1
        s = "S"
    else:
        executed += 1
        s = "e"
    total += 1

    #print "%s: Skipped=%d Executed=%d Tot=%d ins-dev=%3.3f target-dev=%3.3f" % (s, skipped, executed, total, float(executed) / total, deviation)
    sys.stdout.write(s)


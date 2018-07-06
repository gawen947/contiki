import sys

def common_time():
    nod1_up = False
    nod2_up = False

    total_time  = None
    synced_time = 0
    nod1_time   = 0
    nod2_time   = 0

    synced_period_start = None
    nod1_period_start   = None
    nod2_period_start   = None

    nb_nod1_up = 0
    nb_nod2_up = 0
    nb_both_up = 0

    for line in sys.stdin:
        line=line.split(' ')

        try:
            nid=line[3]
            simtime=int(line[2].split('=')[1])
        except:
            continue

        # Register start time
        if not total_time:
            total_time = simtime

        # Keep track of blink state and record individual mote active time
        if nid == "NID=1":
            nod1_up = not nod1_up
            if nod1_up:
                nb_nod1_up += 1
                nod1_period_start = simtime
            else:
                nod1_time += simtime - nod1_period_start
        elif nid == "NID=2":
            nod2_up = not nod2_up
            if nod2_up:
                nb_nod2_up += 1
                nod2_period_start = simtime
            else:
                nod2_time += simtime - nod2_period_start

        # Register synced periods
        if nod1_up and nod2_up:
            nb_both_up += 1
            synced_period_start = simtime
        elif synced_period_start:
            synced_time += simtime - synced_period_start
            synced_period_start = None
    total_time = simtime - total_time

    print "# All time given in microsecs simulation time."
    print
    print "synced_time (amount of time when the LEDs are up on the two motes):", synced_time
    print "nod1_time (amount of time the LED is up on the first mote):", nod1_time
    print "nod2_time (amount of time the LED is up on the second mote):", nod2_time
    print "total_time (total blink time (excluding mote boot)):", total_time
    print
    print "s/t (ratio of synced time over total time):", (float(synced_time) / total_time)
    print "s/n1 (ratio of synced time over mote 1 active time):", (float(synced_time) / nod1_time)
    print "s/n2 (ratio of synced time over mote 2 active time):", (float(synced_time) / nod2_time)
    print
    print "nod1_up (number of times mote 1 switched to an active state (i.e. tried to send)):", nb_nod1_up
    print "nod2_up (number of times mote 2 switched to an active state (i.e. tried to send)):", nb_nod2_up
    print "both_up (both motes were active at the same time (i.e. succesfull transmission)):", nb_both_up
    print "1to2_PDR (PDR for constant transmission from mote 1 to 2):", (float(nb_both_up) / nb_nod1_up)
    print "2to1_PDR (PDR for constant transmission from mote 2 to 1):", (float(nb_both_up) / nb_nod2_up)

if len(sys.argv) != 2:
    print "usage: dual-led (common-time)"
    sys.exit(1)

target=sys.argv[1]
if target == "common-time":
    common_time()
elif target == "PDR":
    PDR()
else:
    print "error: unknown analyze type"
    sys.exit(1)

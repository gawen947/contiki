# Problem with this implementation:
#
# We ask to execute for one microsec
# but the implementation can execute
# for longer than that during sleep.
#
# If we have a new event from another
# node during this interval, we cannot
# register it to the node because it
# already consumed all those microsecs.

""" Cooja """
def execute(t, duration):
    new_t = MSPSim.step(duration) + t
    scheduleNextExecution(new_t)

""" MSPSim """
def cycles_to_micros(c):
    return (1000000 * c) / dcoFrq

def step(duration):
    old_c = cycles

    if sleeping:
        cycles = wakeup
        return cycles_to_micros(cycles - old_c)

    deadline = cycles + (duration * dcoFrq) / 1000000

    while(cycles < deadline):
        if sleeping:
            cycles  = MIN(deadline, wakeup)
        else:
            cycles += emulateOP()

    return cycles_to_micros(cycles - old_c)

""" Cooja """
PARAM(deviation)

old_t   = 0
skipped = 0
total   = 0
def execute(t):
    if (1 - deviation) * total > skipped:
        """
        We executed too much frames.
        We must skip the next frame
        to reach the requested ratio.
        """
        skipped += 1
        old_t   += 1

        # scheduler only takes the nearest scheduled
        # execution into account.
        scheduleNextExecution(t + 1)

    new_t = MSPSim.stepOneMicro(t - old_t) + t + 1 # µs
    scheduleNextExecution(new_t) # execute() will be called at new_t
    old_t = t

    total += 1

""" MSPSim """
cycles = 0
def stepOneMicro(jumpMicros):
    micros   += jumpMicros
    deadline  = ((micros + 1) * dcoFrq ) / 1000000

    while (cycles < deadline):
        if sleeping:
            cycles  = MIN(deadline, wakeup)
        else:
            cycles += emulateOP()

    if sleeping:
        return (1000000 * (wakeup - cycles)) / dcoFrq
    return 0

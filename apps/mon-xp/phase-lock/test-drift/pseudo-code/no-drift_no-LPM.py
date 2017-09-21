""" Cooja """
old_t = 0
def execute(t):
    new_t = MSPSim.stepOneMicro(t - old_t) + t + 1 # µs
    scheduleNextExecution(new_t) # execute() will be called at new_t
    old_t = t

""" MSPSim """
cycles = 0
def stepOneMicro(jumpMicros):
    micros   += jumpMicros
    deadline  = ((micros + 1) * dcoFrq ) / 1000000

    while (cycles < deadline):
        cycles += emulateOP()

    return 0

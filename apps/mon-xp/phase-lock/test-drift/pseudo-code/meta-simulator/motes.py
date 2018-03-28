import math

import cooja

class LegacyCoojaMote(cooja.Mote):
    """
    Classical mote execution with deviation
    parameter as found in Cooja.
    """

    def __init__(self, simulation, mspSim, ID):
        cooja.Mote.__init__(self, simulation, mspSim, ID)
        self.deviation = 1.0
        self.total     = 0
        self.skipped   = 0
        self.old_t     = 0

    def setDeviation(self, deviation):
        self.deviation = deviation

    def moteExecute(self, t, duration):
        if duration != 1:
            raise Exception("except duration of 1 for simple model")

        if (1.0 - self.deviation) * self.total > self.skipped:
            """
            We executed too much frames.
            We must skip the next frame
            to reach the requested ratio.
            """
            self.skipped += 1
            self.old_t   += 1

            # scheduler only takes the nearest scheduled
            # execution into account.
            self.simulation.scheduleNextExec(t + 1, self)

        new_t = self.mspsim.stepOneMicro(t - self.old_t, 1) + t + 1
        self.simulation.scheduleNextExec(new_t, self)
        self.old_t = t

        self.total += 1

    def moteInterrupt(self, t, irq):
        self.mspsim.interrupt(irq)

# Does not work in real Cooja/MSPSim with:
# simple-drift, sim=3600s, seed=1234, timer=32768cyc, d=0.8
class Newdrift_1_CoojaMote(cooja.Mote):
    """
    Same mote as Cooja but with a new
    implementation of the drift parameter.
    """

    def __init__(self, simulation, mspSim, ID):
        cooja.Mote.__init__(self, simulation, mspSim, ID)
        self.deviation         = 1.0
        self.invDeviation      = 1.0 / self.deviation
        self.old_t             = 0
        self.jumpError         = 0.
        self.executeDeltaError = 0.

    def setDeviation(self, deviation):
        self.deviation = deviation

    def moteExecute(self, t, duration):
        jump = t - self.old_t
        exactJump = jump * self.deviation
        jump      = int(math.floor(exactJump))

        self.jumpError += exactJump - jump

        # We permit ourselves a larger error
        # to ensure we stay behind the limits
        # of MPSSim regarding cycles bounds checks.
        if self.jumpError > 1.0:
            jump += 1
            self.jumpError -= 1.0

        executeDelta = self.mspsim.stepOneMicro(jump, duration) + duration

        exactExecuteDelta = executeDelta * self.invDeviation
        executeDelta = int(math.floor(exactExecuteDelta))

        self.executeDeltaError += exactExecuteDelta - executeDelta

        if self.executeDeltaError > 1.0:
            executeDelta += 1
            self.executeDeltaError -= 1.0

        self.simulation.scheduleNextExec(t + executeDelta, self)
        self.old_t = t

    def moteInterrupt(self, t, irq):
        self.mspsim.interrupt(irq)

# Does not work in real Cooja/MSPSim with:
# simple-drift, sim=3600s, seed=1234, timer=32768cyc, d=0.6
class Newdrift_2_CoojaMote(cooja.Mote):
    """
    Same mote as Cooja but with a new
    implementation of the drift parameter.
    """

    def __init__(self, simulation, mspSim, ID):
        cooja.Mote.__init__(self, simulation, mspSim, ID)
        self.deviation         = 1.0
        self.invDeviation      = 1.0 / self.deviation
        self.old_t             = 0
        self.jumpError         = 0.
        self.executeDeltaError = 0.

    def setDeviation(self, deviation):
        self.deviation = deviation

    def moteExecute(self, t, duration):
        jump = t - self.old_t
        exactJump = jump * self.deviation
        jump      = int(math.floor(exactJump))

        self.jumpError += exactJump - jump

        # We permit ourselves a larger error
        # to ensure we stay behind the limits
        # of MPSSim regarding cycles bounds checks.
        if self.jumpError > 1.0:
            jump += 1
            self.jumpError -= 1.0

        executeDelta = self.mspsim.stepOneMicro(jump, duration) + duration

        exactExecuteDelta = executeDelta * self.invDeviation
        executeDelta = int(math.floor(exactExecuteDelta))

        self.executeDeltaError += exactExecuteDelta - executeDelta

        if self.executeDeltaError > 1.0:
            # We report the error on jump to avoid double rounding errors
            self.jumpError += deviation
            self.executeDeltaError -= 1.0

        self.simulation.scheduleNextExec(t + executeDelta, self)
        self.old_t = t

    def moteInterrupt(self, t, irq):
        self.mspsim.interrupt(irq)

class Newdrift_3_CoojaMote(cooja.Mote):
    """
    Same mote as Cooja but with a new
    implementation of the drift parameter.
    """

    def __init__(self, simulation, mspSim, ID):
        cooja.Mote.__init__(self, simulation, mspSim, ID)
        self.deviation         = 1.0
        self.invDeviation      = 1.0 / self.deviation
        self.old_t             = 0
        self.jumpError         = 0.
        self.executeDeltaError = 0.

    def setDeviation(self, deviation):
        self.deviation = deviation

    def moteExecute(self, t, duration):
        jump = t - self.old_t
        exactJump = jump * self.deviation
        jump      = int(math.floor(exactJump))

        self.jumpError += exactJump - jump

        # We permit ourselves a larger error
        # to ensure we stay behind the limits
        # of MPSSim regarding cycles bounds checks.
        if self.jumpError > 1.0:
            jump += 1
            self.jumpError -= 1.0

        executeDelta = self.mspsim.stepOneMicro(jump, duration) + duration

        exactExecuteDelta = executeDelta * self.invDeviation
        executeDelta = int(math.floor(exactExecuteDelta))

        # For some reason (yet unknown) it's better and it works
        # by not reporting the error on floor(executeDelta).

        self.simulation.scheduleNextExec(t + executeDelta, self)
        self.old_t = t

    def moteInterrupt(self, t, irq):
        self.mspsim.interrupt(irq)

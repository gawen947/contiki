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

import random

import cooja

DCO_FREQ=4000000

# IRQ definition. Note that interruption occur
# only for external events (from the simulation)
# and this does not include timers which generate
# internal events in MSPSim.
RADIO_IRQ=0

class Firmware(object):
    """
    Represent a default firmware which controls the behavior
    of the mote.
    """

    def __init__(self, mspsim, config):
        self.mspsim = mspsim

    def interrupt(self, irq):
        raise Exception("cannot interrupt base type Firmware")

    def execute(self):
        raise Exception("cannot execute base type Firmware")

    def emulateOP(self):
        self.execute()
        return random.randint(1,6)

class MSPSim(object):
    """
    Represent a simulator for the mote and its internal components.
    The behavior of the mote itself is controlled by the firmware
    which generate random internal and external events for the
    simulation just like a real binary firmware in the real MSPSim.
    """

    def __init__(self, firmwareClass, firmwareConfig, simulation):
        self.simulation = simulation
        self.micros     = 0
        self.cycles     = 0
        self.wakeup     = 0
        self.sleeping   = False
        self.mote       = None

        self.firmware   = firmwareClass(self, firmwareConfig)

        # IRQ lines status
        # When a IRQ line is enabled and
        # an interrupt received, the CPU
        # is awoken and the interruption
        self.jumpOffset = 2
        # sent back to the firmware.
        # Thus enabling/disabling an IRQ
        # line is the standard way to
        # enable/disable a device.
        self.irqLines = { RADIO_IRQ : True }

    def setMote(self, mote):
        self.mote = mote

    def stepOneMicro(self, jump, duration):
        self.micros += jump

        # check jump length
        deadline = (self.micros * DCO_FREQ) / 1000000
        if self.sleeping:
            if deadline > self.wakeup:
                raise Exception("jump beyond next wakeup (deadline=%d, wakeup=%d)" % (deadline, self.wakeup))
        elif (deadline > self.cycles):
            raise Exception("jump beyond current cycles (deadline=%d, cycles=%d)" % (deadline, self.cycles))

        deadline = ((self.micros + duration) * DCO_FREQ) / 1000000

        while (self.cycles < deadline):
            if self.sleeping:
                self.cycles = min(deadline, self.wakeup)
                if self.cycles >= self.wakeup:
                    self.sleeping = False
            else:
                self.cycles += self.emulateOP()

        if self.sleeping:
            return (1000000 * (self.wakeup - self.cycles)) / DCO_FREQ
        return 0

    def sleep(self, duration):
        self.sleeping = True
        self.wakeup   = self.cycles + duration

    def emulateOP(self):
        return self.firmware.emulateOP()

    def interrupt(self, irq):
        # Only accept registered interrupts
        if irq not in self.irqLines:
            return

        # Disable sleep and transmit to firmware
        if self.irqLines[irq]:
            self.sleeping = False
            self.firmware.interrupt(irq)

    def radioState(self):
        return self.irqLines[RADIO_IRQ]

    def radioSet(self, state):
        self.irqLines[RADIO_IRQ] = state

    def radioSend(self):
        if not self.mote:
            raise Exception("mote not set")
        if self.irqLines[RADIO_IRQ]:
            self.mote.radioSend()

    def getCycles(self):
        return self.cycles

    def isSleeping(self):
        return self.sleeping

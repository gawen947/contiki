from functools import total_ordering

import heapq
import random
import mspsim

DURATION = 1

# Random layout generation
# Each node messages are delayed
# by a number of microseconds choosen
# randomly (at creation) in the interval
# below.
MIN_RADIO_DELAY=10
MAX_RADIO_DELAY=1000

class SimulationEvent(object): pass
class ExecEvent(SimulationEvent): pass
class RadioEvent(SimulationEvent): pass

class Mote(object):
    """
    Base class representing a mote in Cooja.
    You have to give an implementation for execute()/interrupt().
    """

    def __init__(self, simulation, mspsim, ID):
        self.simulation = simulation
        self.mspsim = mspsim
        self.ID     = ID
        self.t      = 0

    def execute(self, t, duration):
        self.t = t # Update time at each event
        self.moteExecute(t, duration)

    def interrupt(self, t, irq):
        self.t = t # Update time at each event
        self.moteInterrupt(t, irq)

    def moteExecute(self, t, duration):
        raise Exception("cannot execute base type Mote")

    def moteInterrupt(self, t, irq):
        raise Exception("cannot interrupt base type Mote")

    def radioSend(self):
        self.simulation.broadcastRadio(self.t, self)

    def getID(self):
        return self.ID

    def __eq__(self, other):
        return other.ID == self.ID

    def __ne__(self, other):
        return other.ID != self.ID

    def __repr__(self):
        return "Mote(%d)" % (self.ID,)

class Simulation(object):
    """
    Represent a Cooja simulation.
    You create the motes using their class and an
    associated firmware. After all nodes have been
    added, you can start the simulation.

    The startSimulation() method does not return until
    the simulation ends. That is when no more events
    are available on the simulation event queue.
    Although this should never happen as nodes never
    stop their execution and always generate execution
    events for the simulation.

    It also ends when the simulation time goes beyond the
    optional limit passed at the creation of the simulation.

    A virtual random layout is generated for the simulation.
    This can be controlled from the seed parameter and
    the MIN/MAX_RADIO_DELAY constant above.
    """

    def __init__(self, seed=None, limit=None):
        self.started    = False
        self.limit      = limit
        self.eventQueue = [] # heap
        self.eventSet   = set()
        self.motes      = []
        self.layout     = []

        random.seed(seed)

    def createMote(self, moteClass, firmwareClass, firmwareConfig):
        if self.started:
            raise Exception("cannot add mote, simulation started")
        mspSim = mspsim.MSPSim(firmwareClass, firmwareConfig, self)
        mote   = moteClass(self, mspSim, len(self.motes))
        mspSim.setMote(mote)
        self.motes.append(mote)
        self.layout.append(random.randint(MIN_RADIO_DELAY, MAX_RADIO_DELAY))

        return mote

    def numberOfMotes(self):
        if not self.started:
            raise Exception("cannot get number of motes, simulation not started")
        return len(self.motes)

    def startSimulation(self):
        if self.started:
            raise Exception("cannot start simulation, already started")
        self.started = True

        # Initialize the scheduler
        for mote in self.motes:
            self.scheduleNextExec(0, mote)

        # Scheduler loop
        while True:
            try:
                event = heapq.heappop(self.eventQueue)
                evTime, evType, evMote = event
                self.eventSet.remove(event)

                if self.limit and evTime > self.limit:
                    return
            except IndexError:
                raise Exception("no more events")

            if evType == ExecEvent:
                self.processExecEvent(evTime, evMote)
            elif evType == RadioEvent:
                self.processRadioEvent(evTime, evMote)
            else:
                raise Exception("unknown event")

    def processRadioEvent(self, evTime, evSourceMote):
        for mote in self.motes:
            if evSourceMote != mote:
                mote.interrupt(evTime, mspsim.RADIO_IRQ)

    def processExecEvent(self, evTime, evMote):
        evMote.execute(evTime, DURATION)

    def pushEvent(self, evTime, evType, evMote):
        event = (evTime, evType, evMote)
        if event in self.eventSet:
            return # ignore duplicates
        self.eventSet.add(event)
        heapq.heappush(self.eventQueue, event)

    def scheduleNextExec(self, t, mote):
        self.pushEvent(t, ExecEvent, mote)

    def broadcastRadio(self, t, mote):
        try:
            radioDelay = self.layout[mote.getID()]
        except IndexError:
            raise Exception("non-existent mote")

        self.pushEvent(t + radioDelay, RadioEvent, mote)

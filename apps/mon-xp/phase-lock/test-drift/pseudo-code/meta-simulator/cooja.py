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

@total_ordering
class SimulationEvent(object):
    """
    The simulation class schedules the execution of the motes
    and propagates simulation event (such as radio packet or
    physical interaction) to the motes.

    Each event is represented as an instance of this class (or subclass).
    """

    def __init__(self, t, targetMotes):
        self.t           = t
        self.targetMotes = targetMotes

    def process(self):
        for mote in self.targetMotes:
            self.processMote(mote)

    def processMote(self, mote):
        raise Exception("cannot process base type SimulationEvent")

    def __eq__(self, other):
        return (self.t == other.t) and \
               (set(self.targetMotes) == set(other.targetMotes)) and \
               (type(self) == type(other))

    def __ne__(self, other):
        return (self.t != other.t) or \
               (set(self.targetMotes) != set(other.targetMotes)) or \
               (type(self) != type(other))

    def __lt__(self, other):
        return self.t < other.t

class ExecEvent(SimulationEvent):
    """
    Event representing the execution of a mote.
    """

    def __repr__(self):
        motes = map(lambda m: m.getID(), self.targetMotes)
        return "ExecEvent(t=%d,motes=%s)" % (self.t, motes)

    def processMote(self, mote):
        mote.execute(self.t, DURATION)

class InterruptEvent(SimulationEvent):
    """
    Interruption event from the simulation.
    """

    def __init__(self, t, targetMotes, irq = 0):
        SimulationEvent.__init__(self, t, targetMotes)
        self.irq = irq

    def __repr__(self):
        motes = map(lambda m: m.getID(), self.targetMotes)
        return "InterruptionEvent(t=%d,motes=%s,irq=%d)" % (self.t, motes, self.irq)

    def processMote(self, mote):
        mote.interrupt(self.t, self.irq)

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
                if len(self.eventQueue) > 20:
                    for e in self.eventQueue:
                        print e
                    return
                if self.limit and event.t > self.limit:
                    return
            except IndexError:
                raise Exception("no more events")
            event.process()

    def pushEvent(self, event):
        if event in self.eventQueue:
            return # ignore duplicates
        heapq.heappush(self.eventQueue, event)

    def scheduleNextExec(self, t, mote):
        event = ExecEvent(t, [mote])
        self.pushEvent(event)

    def broadcastRadio(self, t, mote):
        try:
            radioDelay = self.layout[mote.getID()]
        except IndexError:
            raise Exception("non-existent mote")

        targetMotes = [target for target in self.motes if target != mote]

        event = InterruptEvent(t + radioDelay, targetMotes, irq = mspsim.RADIO_IRQ)
        self.pushEvent(event)

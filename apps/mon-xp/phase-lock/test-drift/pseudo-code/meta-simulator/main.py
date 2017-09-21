import cooja
import firmwares
import motes

MICROSEC=1000000
REF_FRQ=4.0 # MHz

firmwareConfig = firmwares.RandomFirmwareConfig(min_exec=1000, max_exec=20000,
                                                min_sleep=4000, max_sleep=100000,
                                                send_probability=0.3)
class Result(object):
    def __init__(self, simTime, cycTime, frqMHz, obsDev, reqDev):
        self.simTime = simTime
        self.cycTime = cycTime
        self.frqMHz  = frqMHz
        self.obsDev  = obsDev
        self.reqDev  = reqDev

    def __repr__(self):
        return "{simTime=%3.3fs,cycTime=%3.3fMcyc,frq=%3.3fMHz,obsDev=%3.3f%%,reqDev=%3.3f%%}" \
                % (float(self.simTime) / MICROSEC,
                   float(self.cycTime) / 1e6,
                   self.frqMHz,
                   self.obsDev * 100.,
                   self.reqDev * 100.)

def run_sim(time, nMotes, deviation):
    # setup simulation and motes
    simulation = cooja.Simulation(seed=1234, limit = time * MICROSEC)

    simMotes = []
    for i in range(nMotes):
        mote = simulation.createMote(motes.LegacyCoojaMote, firmwares.RandomFirmware, firmwareConfig)
        mote.setDeviation(deviation)
        simMotes.append(mote)

    simulation.startSimulation()

    # compute observed deviation
    results = []
    for mote in simMotes:
        simTime = mote.t
        cycTime = mote.mspsim.cycles
        frqMHz  = float(cycTime) / simTime
        obsDev  = frqMHz / REF_FRQ

        results.append(Result(simTime, cycTime, frqMHz, obsDev, deviation))

    return results

for dev in (0.1, 0.5, 0.7, 1.0):
    rs = run_sim(30, 1, dev)
    for r in rs:
        print r

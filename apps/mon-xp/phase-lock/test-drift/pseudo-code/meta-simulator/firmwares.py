import random

import mspsim

class NullFirmware(mspsim.Firmware):
    """
    Does nothing, ignore interrupts, never go to sleep.
    """

    def interrupt(self, mspsim, irq):
        pass

    def execute(self, mspsim):
        pass

class RandomFirmwareConfig(object):
    def __init__(self, min_exec, max_exec, min_sleep, max_sleep, send_probability):
        self.min_exec  = min_exec
        self.max_exec  = max_exec
        self.min_sleep = min_sleep
        self.max_sleep = max_sleep
        self.send_probability = send_probability

class NoSleepRandomFirmwareConfig(object):
    def __init__(self, min_exec, max_exec, send_probability):
        self.min_exec  = min_exec
        self.max_exec  = max_exec
        self.send_probability = send_probability

class RandomFirmware(mspsim.Firmware):
    """
    Execute for a fixed number of cycles.
    Then has a probability to send a packet.
    Then goes to sleep for a fixed number of cycles.

    [ radio on and execution ][ send possibility ][ radio off and sleep ]
    """

    def __init__(self, mspSim, config):
        mspsim.Firmware.__init__(self, mspSim, config)

        self.executed = 0

        if type(config) != RandomFirmwareConfig:
            raise Exception("invalid configuration class")

        if config.send_probability > 1.0 or config.send_probability < 0.0:
            raise Exception("invalid probability")

        self.min_exec         = config.min_exec
        self.max_exec         = config.max_exec
        self.min_sleep        = config.min_sleep
        self.max_sleep        = config.max_sleep
        self.send_probability = config.send_probability

        self.exec_oldcyc = 0
        self.exec_period = random.randint(self.min_exec, self.max_exec)

        self.stateFun = self.stateFunExecute

    def interrupt(self, irq):
        pass

    def execute(self):
        self.stateFun()

    def stateFunExecute(self):
        delta = self.mspsim.getCycles() - self.exec_oldcyc
        if delta > self.exec_period:
            self.stateFun = self.stateFunSend

    def stateFunSend(self):
        # Send possibility
        p = random.uniform(0.0, 1.0)
        if self.send_probability < p:
            self.mspsim.radioSend()

        # Shutdown radio and sleep
        sleep_period = random.randint(self.min_sleep, self.max_sleep)
        self.mspsim.radioSet(False)
        self.mspsim.sleep(sleep_period)

        self.stateFun = self.stateFunSleep

    def stateFunSleep(self):
        # Change state when out of sleep
        if not self.mspsim.isSleeping():
            self.stateFun    = self.stateFunExecute
            self.exec_oldcyc = self.mspsim.getCycles()
            self.exec_period = random.randint(self.min_exec, self.max_exec)
            self.mspsim.radioSet(True)

class NoSleepRandomFirmware(mspsim.Firmware):
    """
    Execute for a fixed number of cycles.
    Then has a probability to send a packet.

    [ execution ][ send possibility ]
    """

    def __init__(self, mspSim, config):
        mspsim.Firmware.__init__(self, mspSim, config)

        self.executed = 0

        if type(config) != NoSleepRandomFirmwareConfig:
            raise Exception("invalid configuration class")

        if config.send_probability > 1.0 or config.send_probability < 0.0:
            raise Exception("invalid probability")

        self.min_exec         = config.min_exec
        self.max_exec         = config.max_exec
        self.send_probability = config.send_probability

        self.exec_oldcyc = 0
        self.exec_period = random.randint(self.min_exec, self.max_exec)

        self.stateFun = self.stateFunExecute

    def interrupt(self, irq):
        pass

    def execute(self):
        self.stateFun()

    def stateFunExecute(self):
        delta = self.mspsim.getCycles() - self.exec_oldcyc
        if delta > self.exec_period:
            self.stateFun = self.stateFunSend

    def stateFunSend(self):
        # Send possibility
        p = random.uniform(0.0, 1.0)
        if self.send_probability < p:
            self.mspsim.radioSend()

        # Shutdown radio and sleep
        self.exec_oldcyc = self.mspsim.getCycles()
        self.exec_period = random.randint(self.min_exec, self.max_exec)
        self.stateFun = self.stateFunExecute

import os
from cooja_tools import *

init_cooja_settings(os.path.realpath("../../../"))

server = SkyMoteType("balanced")
client = SkyMoteType("ping")

topology = Topology.get_tree25_topology(server, client)

simulation = CoojaSimulation(topology, timeout=30)

simulation.run(verbose=True)

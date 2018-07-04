Here we design simple T-Mote Sky applications to test the drift parameter outside Contiki.
We then display the observed deviation as we do with the test-drift experiment.

To realize this XP we proceed in 4 steps:

 1) [simple]  Just compile on T-Mote Sky (and LED blink?).
 2) [monitor] Integrate the monitor with symlinks.
 3) [xp.sh]   Beacon deviation, analyse and show results as it is done in the test-drift XP.
 4) [monitor-sleep]   Try the same experiment with sleep machanisms.

There are also other experiments:

 5) [xp-overshoot.sh] Measure the overshoot with MSPSim instrumentalisation.
 6) [monitor-led] Simulate a simple RDC MAC layer with blinking LEDs (same application as used on real node)
 7) [xp-led.sh] Compare the drift of two motes. Simulate a basic RDC with a blinking LED. Count the number of overlaping ON LEDs as a virtual-PDR.

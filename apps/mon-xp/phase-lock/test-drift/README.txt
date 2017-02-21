Here we try to see if it is possible to use the deviation parameter in Cooja
to adjust the MSP clock to simulate the clock frequency deviation.

To do so, each 5 seconds we generate a state events that records:
 CPU cycles elapsed since node started
 simulation time at the moment of record

Random mote delay is disabled for all motes. So in all experiments, the mote
start at the same time.

What we observe (see results-7200s_1run_drifts and results-300s_10run_drifts) is that:
 1) With a deviation of 1.0 (100%), we have 3.94173 cycles / µs. We observe the same result for longer durations (upto 8 hours).
    We also observe the same result with and without RDC enabled (that is with nullrdc and ContikiMAC).
    This mean a "cycle" frequency of ~3.94 MHz even though the frequency of the Sky mote is supposed to be 16 MHz
    (to recall

 2) We have a consistent result across runs with different PRNG seed.

 3) Below 1.0, we have a curve of decreasing frequency that seems to tends to 3 MHz at the limit.
    This frequency goes lower as we increase the XP time.

From this we will try to answer several questions here:

 Q1) What is the MCLK frequency for the Sky Mote in Cooja?
 Q2) Why do we measure 3.94173 MHz for MCLK in Cooja?
 Q3) What is changed by the deviation attribute in Cooja?

Q1) What is the MCLK frequency for the Sky Mote in Cooja?
It depends of the MCLK configuration (resistor sel, clock sel, ... (see [MSPSim] BasicClockModule), 
but it goes max to 5-8MHz and Cooja set it up to ~4 MHz.

Q2) Why do we measure 3.94173 MHz for MCLK in Cooja?
That's the actual DCO (after DCO sync) configured and checked in MSPSim with a println() in MSP430Core::setDCOFrq().

Q3) What is changed by the deviation attribute in Cooja?
The deviation is a parameter between 0 and 1.0, value beyonds those boundaries have no effect.
It skip execution frame until the ratio skipped/executed frame reaches the desired deviation.
So it should affect MCLK.

From there another *BIG* question: What if the CPU goes to sleep?
The CPU regularly goes to sleep when using the timers on MSPSim.
When we explicitely disable sleep mode in the Sky mote, we have
the correct frequency value for DCO (drift * DCOfreq).

Also the interval between two probes (which should be 5 seconds)
follows the drift ratio (PERIOD = PERIOD / drift).

I checked the DCO sync w/wo drift and w/wo LPM. We always sync
on the same values. Also check w/wo DCO sync.

The time between measurement follows the ratio of DCO freq.

Another short question: what changes ACLK / XT1CLK in MSPSim?
How does this relate to stepMicros(),emulateOP() and DCO?

In summary:

Sleep?    Cooja-Drift  ACLK-ratio MCLK-ratio
NO        10%          10%        10%
YES       10%          86%        86%
YES       50%          98%        98%

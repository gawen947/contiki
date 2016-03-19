Description
===========

Experiment a process that locks the poll from the radio layer to the netstack process.
This experiment is separated in two images.

1) "ping" that just regularly pings (broadcast rime) the other node.
2) "lock" that receives the pings (on the radio layer) but a process tries to lock the scheduler and the netstack process (clear the watchdog to force this).
3) "fair" that received the pings (on the radio layer) and the main process just wait for ~1000s (event wait) in a while loop.

We use a Rime broadcast message as we want to avoid RPL and incidental traffic.

Results
=======

ping->fair:

ISR -> POLL -> READ -> CCA -> CCA -> RDC -> OFF

Happens for each broadcasted packet.

Time: POLL -> OFF

 4841 cyc.
 1.24 ms



ping->lock

ISR -> POLL -> CCA -> CCA -> OFF

Happens only ONCE then ISR is disabled.

Time: POLL -> OFF

 474968 cyc.
 121.66 ms

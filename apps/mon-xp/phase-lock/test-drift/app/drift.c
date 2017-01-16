/* Copyright (c) 2016, David Hauweele <david@hauweele.net>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "contiki.h"
#include "lib/random.h"
#include "net/netstack.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"
#include "dev/serial-line.h"

#include "mon/monitor.h"
#include "mon/context.h"
#include "mon/context/control.h"

#define PERIOD 5 /* seconds */

PROCESS(drift_process, "drift process");
AUTOSTART_PROCESSES(&drift_process);

/* regularly record events to record CPU time evolution
   and compute CPU clock drift. */
PROCESS_THREAD(drift_process, ev, data)
{
  static struct etimer period_timer;

  PROCESS_BEGIN();
  PROCESS_PAUSE();

  etimer_set(&period_timer, CLOCK_SECOND * PERIOD);
  while(1) {
    PROCESS_WAIT_EVENT(); /* wait for the timer */

    if(ev == PROCESS_EVENT_TIMER) {
      if(data == &period_timer) {
        etimer_reset(&period_timer);
        monitor_record(MON_CT_CONTROL, MON_ENT_TEST, 1);
      }
    }
  }

  PROCESS_END();
}

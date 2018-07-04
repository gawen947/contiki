/* Copyright (c) 2018, David Hauweele <david@hauweele.net>
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

#include <msp430.h>

#include "dco-sync.h"
#include "events.h"
#include "config.h"

void enable_timer(void)
{
  TACCR0   = CONFIG_PERIOD;
  TACCR1   = 32768; /* FIXME: do we need TACCR1 */
  TACCTL0  = CCIE; /* enable capture/compare interruption */
  TACTL    = TACLR | TASSEL0 | MC1 /* | TAIE */; /* timer configured at ACLK/1 up mode. */
}

void disable_timer(void)
{
  TACTL = 0;
}

/* For some reason up mode (MC0) doesn't work at all.
   So I use continuous mode (MC1) and reset the counter on each interrupt.
   see:

   https://github.com/contiki-ng/mspsim/pull/29/commits/0118e6f1e2375e78e41fc0fe16ff26d2b6392007
 */

/* See msp430/isr_compat.h.
   In this case the MSPGCC way.
   TAIFG is in TIMERA1_VECTOR (not TIMERA0). */
void __attribute__((interrupt (TIMERA0_VECTOR))) timer_a0(void)
{
  static int blink_count    = CONFIG_BLINK_PERIOD;
  static int dco_sync_count = CONFIG_DCO_SYNC_PERIOD;

  /* We need to reset the interrupt or it will be triggered constantly.
     This is only needed for TIMERA1_VECTOR though. */
  /*
  volatile unsigned reset_flag = TAIV;
  (void)reset_flag;
  */

  if(blink_count-- <= 0) {
    event_BLINK();
    blink_count = CONFIG_BLINK_PERIOD;
  }
#ifdef CONFIG_MCLK_FROM_DCO_SYNC
  if(CONFIG_DCO_SYNC_PERIOD && dco_sync_count-- <= 0) {
    disable_timer();
    dco_sync();
    dco_sync_count = CONFIG_DCO_SYNC_PERIOD;
    enable_timer();
  }
#endif

  /* FIXME: LFXTO1, OFIFG */

  /* we cannot reset the timer with TAR = 0, it does not work,
     instead with increment it continuously. */
  TACCR0 += CONFIG_PERIOD;
}

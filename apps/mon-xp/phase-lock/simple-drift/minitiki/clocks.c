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

void __attribute__((interrupt (NMI_VECTOR))) nmi(void)
{
  /* NMI catches OFIFG and LFXT1OF. */
#ifdef CONFIG_EVENT_OFIFG
  if(IFG1 & OFIFG)
    event_OFIFG(true);
#endif
#ifdef CONFIG_EVENT_LFXT1OF
# ifdef BCSCTL3
  if(BCSCTL3 & LFXT1OF)
    event_LFXT1OF(true);
# else
#  error "platform does not support LFXT1OF"
# endif
#endif

  /* The Oscillator Fault flag is not cleared by default.
     It must be cleared otherwise MCLK would be sourced
     from the DCO. */
  IFG1 &= ~OFIFG;
}

void configure_clocks(void)
{
#ifdef CONFIG_LFXT1_EXTERNAL
# ifdef BCSCTL3
  /* use an external clock (that his not a crystal) on XIN/XOUT */
  _BIC_SR(OSCOFF);
  BCSCTL3 |= LFXT1S_3;
  BCSCTL3 &= ~(XCAP1 | XCAP0 ); /* internal capacity = 1 pF */
# endif
#endif

#if defined(CONFIG_MCLK_FROM_DCO_SYNC)
  dco_sync();
#elif defined(CONFIG_MCLK_FROM_LFXT1)
  /* select LFXT1 as (S)MCLK source */
  BCSCTL2 |= SELM_3;
  BCSCTL2 |= SELS;
#else
# error "unsupported clock source for MCLK"
#endif

  /* enable oscillator fault interrupts */
  IE1 |= OFIE;
}

/* Copyright (c) 2017, David Hauweele <david@hauweele.net>
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

#include "config.h"

#ifdef F_CPU
# define MSP430_CPU_SPEED F_CPU
#else
# define MSP430_CPU_SPEED 2457600UL
#endif

#ifdef CONFIG_MCLK_FROM_DCO_SYNC
void dco_sync(void)
{
  /* Taken directly from Contiki DCO sync code for the MSP430 f1xxx (and reformatted). */
#define DELTA    ((MSP430_CPU_SPEED) / (32768 / 8))

  unsigned int compare, oldcapture = 0;
  unsigned int i;

  /* ACLK is devided by 4. RSEL=6 no division for MCLK and
     SSMCLK. XT2 is off. */
  BCSCTL1 = 0xa4;

  /* Init FLL to desired frequency using the 32762Hz crystal
     DCO frequency = 2.4576 MHz.
     Gwn: This is not FLL, we just configure MCLK and SMCLK to DCOCLK
          With div=1 and internal resistor. */
  BCSCTL2 = 0x00;

  BCSCTL1 |= DIVA1 + DIVA0; /* ACLK = LFXT1CLK/8 */

  /* Delay for XTAL to settle */
  for(i = 0xffff; i > 0; i--)
    __no_operation();

  CCTL2 = CCIS0 + CM0 + CAP;     /* Define CCR2, CAP, ACLK */
  TACTL = TASSEL1 + TACLR + MC1; /* SMCLK, continous mode */

  while(1) {
    /* Wait until capture occured! */
    while((CCTL2 & CCIFG) != CCIFG);

    CCTL2 &= ~CCIFG;                /* Capture occured, clear flag */
    compare = CCR2;                 /* Get current captured SMCLK */
    compare = compare - oldcapture; /* SMCLK difference */
    oldcapture = CCR2;              /* Save current captured SMCLK */

    if(DELTA == compare)
      /* if equal, leave "while(1)" */
      break;
    else if(DELTA < compare) {
      /* DCO is too fast, slow it down */
      DCOCTL--;

      /* Did DCO role under? */
      if(DCOCTL == 0xff)
        BCSCTL1--;
    }
    else {
      /* -> Select next lower RSEL */
      DCOCTL++;

      /* Did DCO role over? */
      if(DCOCTL == 0x00)
        /* -> Select next higher RSEL  */
        BCSCTL1++;
    }
  }

  CCTL2 = 0; /* Stop CCR2 function */
  TACTL = 0; /* Stop Timer_A */

  BCSCTL1 &= ~(DIVA1 + DIVA0); /* remove /8 divisor from ACLK again */
}
#endif

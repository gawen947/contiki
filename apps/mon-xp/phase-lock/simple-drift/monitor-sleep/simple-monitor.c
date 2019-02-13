#include <msp430.h>

#include "context/control.h"
#include "dco-sync.h"
#include "monitor.h"
#include "context.h"
#include "memmon.h"

#ifndef TIMER
# warning "Default timer interval"
# define TIMER 32768
#endif /* TIMER */

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
  /* We need to reset the interrupt or it will be triggered constantly.
     This is only needed for TIMERA1_VECTOR though. */
  /*
  volatile unsigned reset_flag = TAIV;
  (void)reset_flag;
  */

  P5OUT ^= BIT4; /* blink LED1 */

  /* we cannot reset the timer with TAR = 0, it does not work,
     instead we increment it continuously. */
  TACCR0 += TIMER;

  /* Each record would report both the simulation (in microsec) and CPU time (in cycles).
     This call is 27 cyc. long (iirc). */
  mon_record(MON_CT_CONTROL, MON_ENT_TEST, 1);
}

int main()
{
  WDTCTL = WDTPW | WDTHOLD; /* stop watchdog timer */
  P5DIR |= BIT4;            /* configure LED1 (P5.4 on TMote Sky) */
  P5OUT |= BIT4;            /* start with LED1 high */

  P5DIR |= BIT5;            /* also configure LED2 */
  P5OUT |= BIT5;            /* start in same state */

  dco_sync();
  monitor_init();

  TACCR0   = TIMER;
  TACCR1   = 32768;
  TACCTL0  = CCIE; /* enable capture/compare interruption */
  TACTL    = TACLR | TASSEL0 | MC1 /* | TAIE */; /* timer configured at ACLK/1 up mode. */
#ifdef DISABLE_SLEEP
# warning "Sleep mode disabled."
  _BIS_SR(GIE); /* No sleep, we just enable interrupt */
#else
# warning "Sleep mode enabled."
  _BIS_SR(GIE | SCG0 | SCG1 | CPUOFF); /* LPM3 */
#endif

  /* We ensure that we are in the sleep mode,
     if we weren't the LED2 would be activated. */
  while(1) {
    unsigned int i, j;
    P5OUT ^= BIT5; /* toggle LED2 */
    for(j = 0 ; j < 32 ; j++) {
      for(i = 0 ; i < 32768 ; i++)
        __no_operation();
    }
  }

  return 0;
}

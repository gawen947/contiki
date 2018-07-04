#include <msp430.h>

#include "context/control.h"
#include "dco-sync.h"
#include "monitor.h"
#include "context.h"
#include "memmon.h"
#include "utils.h"

#include "events.h"
#include "config.h"

void enable_hz(void)
{
  TACCR0   = CONFIG_PERIOD;
  TACCR1   = 32768; /* FIXME: do we need TACCR1 */
  TACCTL0  = CCIE; /* enable capture/compare interruption */
  TACTL    = TACLR | TASSEL0 | MC1 /* | TAIE */; /* timer configured at ACLK/1 up mode. */
}

void disable_hz(void)
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
  if(CONFIG_DCO_SYNC_PERIOD && dco_sync_count-- <= 0) {
    disable_hz();
    dco_sync();
    dco_sync_count = CONFIG_DCO_SYNC_PERIOD;
    enable_hz();
  }

  /* FIXME: LFXTO1, OFIFG */

  /* we cannot reset the timer with TAR = 0, it does not work,
     instead with increment it continuously. */
  TACCR0 += CONFIG_PERIOD;
}

int main()
{
  WDTCTL = WDTPW | WDTHOLD; /* stop watchdog timer */

  dco_sync();
  configure_events();
  enable_hz();

  /* go to sleep (LPM3) */
  _BIS_SR(GIE | SCG0 | SCG1 | CPUOFF);

  /* We ensure that we are in the sleep mode,
     if we weren't the LED2 would be activated. */
  while(1) {
    unsigned int i, j;

    event_NOSLEEP();

    for(j = 0 ; j < 32 ; j++) {
      for(i = 0 ; i < 32768 ; i++)
        __no_operation();
    }
  }

  return 0;
}

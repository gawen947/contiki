#include <msp430.h>

#include "context/control.h"
#include "dco-sync.h"
#include "monitor.h"
#include "context.h"
#include "memmon.h"
#include "utils.h"

#include "timer.h"
#include "events.h"
#include "config.h"

int main()
{
  WDTCTL = WDTPW | WDTHOLD; /* stop watchdog timer */

#ifdef CONFIG_LFXT1_EXTERNAL
# ifdef BCSCTL3
  _BIC_SR(OSCOFF);
  BCSCTL3 |= LFXT1S_3;
  BCSCTL3 &= ~(XCAP1 | XCAP0); /* internal capacity = 1 pF */
# else
#  error "cannot configure LFXT1 source on this platform"
# endif
#endif
#if defined(CONFIG_MCLK_FROM_DCO_SYNC)
  dco_sync();
#elif defined(CONFIG_MCLK_FROM_LFXT1)
  /* This code should workd for both MSP430 x2xx and x1xx family. */
  BCSCTL2 |= SELM_3; /* Select LFXT1 as MCLK source */
  BCSCTL2 &= ~DIVM_3;
  BCSCTL2 |= SELS;
#endif
  configure_events();
  enable_timer();

  /* go to sleep (LPM3) */
  _BIS_SR(GIE | SCG0 | SCG1 | CPUOFF);

  /* We ensure that we are in the sleep mode,
     and that all events are handled from the
     timer interrupt otherwize the event NOSLEEP
     would be triggered. */
  while(1) {
    event_NOSLEEP();
    while(1);
  }

  return 0;
}

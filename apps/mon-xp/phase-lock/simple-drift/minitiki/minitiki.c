#include <msp430.h>

#include "context/control.h"
#include "dco-sync.h"
#include "monitor.h"
#include "context.h"
#include "memmon.h"
#include "utils.h"

#include "timer.h"
#include "clocks.h"
#include "events.h"
#include "config.h"

int main()
{
  WDTCTL = WDTPW | WDTHOLD; /* stop watchdog timer */

  configure_clocks();
  configure_events();
  enable_timer();

#ifndef DISABLE_SLEEP
  /* go to sleep (LPM3) */
  _BIS_SR(GIE | SCG0 | SCG1 | CPUOFF);
#endif

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

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

  dco_sync();
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

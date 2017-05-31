#include <msp430.h>

#include "dco-sync.h"
#include "monitor.h"
#include "context.h"
#include "memmon.h"

#define BUSY_CYCLES_B 1024
#define BUSY_CYCLES_A (unsigned int)-1

static void busy()
{
  unsigned int n,m;
  for(m = BUSY_CYCLES_B ; m ; m--) {
    for(n = BUSY_CYCLES_A ; n ; n--)
      __no_operation();
  }
}

int main()
{
  WDTCTL = WDTPW | WDTHOLD; /* stop watchdog timer */
  P5DIR |= BIT4;            /* configure LED1 (P5.4 on TMote Sky) */

  dco_sync();
  monitor_init();

  while(1) {
    P5OUT ^= BIT4; /* blink led */
    busy();
    mon_record(MON_CT_CONTROL, 0, 1);
  }

  return 0;
}

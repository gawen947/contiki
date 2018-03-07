#include <msp430.h>

#include "context/control.h"
#include "dco-sync.h"
#include "monitor.h"
#include "context.h"
#include "memmon.h"

/* See msp430/isr_compat.h.
   In this case the MSPGCC way.
   TAIFG is in TIMERA1_VECTOR (not TIMERA0). */
void __attribute__((interrupt (TIMERA1_VECTOR))) timer_a0(void)
{
  /* We need to reset the interrupt or it will be triggered constantly. */
  volatile unsigned reset_flag = TAIV;
  (void)reset_flag;

  P5OUT ^= BIT4; /* blink led */

  mon_record(MON_CT_CONTROL, MON_ENT_TEST, 1);
}

int main()
{
  WDTCTL = WDTPW | WDTHOLD; /* stop watchdog timer */
  P5DIR |= BIT4;            /* configure LED1 (P5.4 on TMote Sky) */
  P5OUT |= BIT4;            /* start with LED1 off (?) */

  P5DIR |= BIT5;            /* also configure LED2 */
  P5OUT |= BIT5;            /* start in same state */

  dco_sync();
  monitor_init();

  /* Timer configured at ACLK/2 continuous mode
     should start counting right now. Each interrupt
     happens every 4 seconds. */
  TACTL = TACLR | TASSEL0 | ID0 | MC1 | TAIE;
  _BIS_SR(GIE | SCG0 | SCG1 | CPUOFF); /* LPM3 */

  /* We ensure that we are in the sleep mode,
     if we weren't the LED2 would be activated. */
  while(1)
    P5OUT &= ~BIT5;

  return 0;
}

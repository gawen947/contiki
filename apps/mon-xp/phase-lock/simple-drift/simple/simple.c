#include <msp430.h>

#define BUSY_CYCLES (unsigned int)-1

static void busy()
{
  unsigned int n;
  for(n = BUSY_CYCLES ; n ; n--)
    __no_operation();
}

int main()
{
  WDTCTL = WDTPW | WDTHOLD; /* stop watchdog timer */
  P5DIR |= BIT4;            /* configure LED1 (P5.4 on TMote Sky) */

  while(1) {
    P5OUT ^= BIT4; /* blink led */
    busy();
  }

  return 0;
}

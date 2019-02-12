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

/* Configure the clock source for MCLK.
   That is either from the DCO regularly synced with ACLK.
   Or directly from ACLK. */
#define CONFIG_MCLK_FROM_DCO_SYNC 1
/* #define CONFIG_MCLK_FROM_LFXT1  1 */
/* #define CONFIG_LFXT1_EXTERNAL   1 */

/* Number of timer cycles (see CONFIG_PERIOD) to issue a DCO sync.
   Use 0 to disable regular DCO sync beyond the first one. */
#define CONFIG_DCO_SYNC_PERIOD 30

/* Number of timer cycles (see CONFIG_PERIOD) to toggle de LED state. */
#define CONFIG_BLINK_PERIOD 1

/* Frequency of the sleep and blinking led timer.
   From 1 to 32768 (which is one second). */
#define CONFIG_PERIOD 32768

/* Specify how to report event changes.
   Either by changing the state of a pin
   or generating a monitor event. */
#define CONFIG_EVENT_SYSTEM_PIN     1
#define CONFIG_EVENT_SYSTEM_MONITOR 1

/* Output the ACLK clock. */
/* #define CONFIG_EVENT_ACLK 1 */
#define CONFIG_EVENT_ACLK_PORT             P1
#define CONFIG_EVENT_ACLK_PIN              BIT1

/* Output the MCLK clock. */
/* #define CONFIG_EVENT_MCLK 1 */
#define CONFIG_EVENT_MCLK_PORT             P1
#define CONFIG_EVENT_MCLK_PIN              BIT1

/* Output the blinking LED. */
#define CONFIG_EVENT_BLINK                 1
#define CONFIG_EVENT_BLINK_PORT            P5
#define CONFIG_EVENT_BLINK_PIN             BIT4

/* Output the LFXT1OF oscillator fault. */
/* #define CONFIG_EVENT_LFXT1OF 1 */
#define CONFIG_EVENT_LFXT1OF_PORT          P1
#define CONFIG_EVENT_LFXT1OF_PIN           BIT1

/* Output the OFIFG oscillator fault. */
/* #define CONFIG_EVENT_OFIFG 1 */
#define CONFIG_EVENT_OFIFG_PORT            P1
#define CONFIG_EVENT_OFIFG_PIN             BIT1

/* Output the NOSLEEP oscillator fault. */
/* #define CONFIG_EVENT_NOSLEEP               1 */
#define CONFIG_EVENT_NOSLEEP_PORT          P5
#define CONFIG_EVENT_NOSLEEP_PIN           BIT5

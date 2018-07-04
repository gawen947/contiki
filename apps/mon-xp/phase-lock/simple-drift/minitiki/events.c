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

#include <msp430.h>
#include <stdbool.h>

#include "context/control.h"
#include "context/mon-xp.h"
#include "monitor.h"
#include "context.h"
#include "memmon.h"
#include "config.h"
#include "utils.h"

void configure_events(void)
{
#ifdef CONFIG_EVENT_SYSTEM_PIN
# ifdef CONFIG_EVENT_ACLK
  PDIR(CONFIG_EVENT_ACLK_PORT) |= CONFIG_EVENT_ACLK_PIN;
  PSEL(CONFIG_EVENT_ACLK_PORT) |= CONFIG_EVENT_ACLK_PIN;
# endif
# ifdef CONFIG_EVENT_MCLK
  PDIR(CONFIG_EVENT_MCLK_PORT) |= CONFIG_EVENT_MCLK_PIN;
  PSEL(CONFIG_EVENT_MCLK_PORT) |= CONFIG_EVENT_MCLK_PIN;
# endif
# ifdef CONFIG_EVENT_BLINK
  PDIR(CONFIG_EVENT_BLINK_PORT) |= CONFIG_EVENT_BLINK_PIN;
  POUT(CONFIG_EVENT_BLINK_PORT) &= ~CONFIG_EVENT_BLINK_PIN;
# endif
# ifdef CONFIG_EVENT_LFXT1OF
  PDIR(CONFIG_EVENT_LFXT1OF_PORT) |= CONFIG_EVENT_LFXT1OF_PIN;
  POUT(CONFIG_EVENT_LFXT1OF_PORT) &= ~CONFIG_EVENT_LFXT1OF_PIN;
# endif
# ifdef CONFIG_EVENT_OFIFG
  PDIR(CONFIG_EVENT_OFIFG_PORT) |= CONFIG_EVENT_OFIFG_PIN;
  POUT(CONFIG_EVENT_OFIFG_PORT) &= ~CONFIG_EVENT_OFIFG_PIN;
# endif
# ifdef CONFIG_EVENT_NOSLEEP
  PDIR(CONFIG_EVENT_NOSLEEP_PORT) |= CONFIG_EVENT_NOSLEEP_PIN;
  POUT(CONFIG_EVENT_NOSLEEP_PORT) &= ~CONFIG_EVENT_NOSLEEP_PIN;
# endif
#endif

#ifdef CONFIG_EVENT_SYSTEM_MONITOR
  monitor_init();
#endif
}

void event_BLINK(void)
{
#ifdef CONFIG_EVENT_BLINK
# ifdef CONFIG_EVENT_SYSTEM_PIN
  POUT(CONFIG_EVENT_BLINK_PORT) ^= CONFIG_EVENT_BLINK_PIN;
# endif
# ifdef CONFIG_EVENT_SYSTEM_MONITOR
  record_event(MON_XP_BLINK);
# endif
#endif
}

void event_LFXT1OF(bool state)
{
#ifdef CONFIG_EVENT_LFXT1OF
# ifdef CONFIG_EVENT_SYSTEM_PIN
  if(state)
    POUT(CONFIG_EVENT_LFXT1OF_PORT) ^= CONFIG_EVENT_LFXT1OF_PIN;
  else
    POUT(CONFIG_EVENT_LFXT1OF_PORT) &= ~CONFIG_EVENT_LFXT1OF_PIN;
# endif
# ifdef CONFIG_EVENT_SYSTEM_MONITOR
  if(state)
    record_event(MON_XP_LFXT1OF);
# endif
#endif
}

void event_OFIFG(bool state)
{
#ifdef CONFIG_EVENT_OFIFG
# ifdef CONFIG_EVENT_SYSTEM_PIN
  if(state)
    POUT(CONFIG_EVENT_OFIFG_PORT) ^= CONFIG_EVENT_OFIFG_PIN;
  else
    POUT(CONFIG_EVENT_OFIFG_PORT) &= ~CONFIG_EVENT_OFIFG_PIN;
# endif
# ifdef CONFIG_EVENT_SYSTEM_MONITOR
  if(state)
    record_event(MON_XP_OFIFG);
# endif
#endif
}

void event_NOSLEEP(void)
{
#ifdef CONFIG_EVENT_NOSLEEP
# ifdef CONFIG_EVENT_SYSTEM_PIN
  POUT(CONFIG_EVENT_NOSLEEP_PORT) |= CONFIG_EVENT_NOSLEEP_PIN;
# endif
# ifdef CONFIG_EVENT_SYSTEM_MONITOR
  record_event(MON_XP_NOSLEEP);
# endif
#endif
}

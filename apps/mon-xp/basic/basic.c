/*
 * Copyright (c) 2015, David Hauweele <david@hauweele.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A simple example to test the monitor
 * \author
 *         David Hauweele <david@hauweele.net>
 */

#include <stdio.h>

#include "contiki.h"
#include "mon/monitor.h"
#include "mon/context.h"
#include "mon/context/control.h"

#define STATE 0x2222

/*---------------------------------------------------------------------------*/
PROCESS(example_monitor_process, "Test monitor process");
AUTOSTART_PROCESSES(&example_monitor_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_monitor_process, ev, data)
{
  static char info[] = "foobar";

  PROCESS_BEGIN();

  /* create entity */
  monitor_create(MON_CT_CONTROL, MON_ENT_TEST);
  monitor_info(MON_CT_CONTROL, MON_ENT_TEST, info, sizeof(info));

  /* record an event */
  monitor_record(MON_CT_CONTROL, MON_ENT_TEST, STATE);

  /* destroy entity */
  monitor_destroy(MON_CT_CONTROL, MON_ENT_TEST);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
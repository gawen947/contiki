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
 *         Monitor state changes across Contiki.
 * \author
 *         David Hauweele <david@hauweele.net>
 */

#include <stdint.h>

#include "memmon.h"
#include "context.h"
#include "context/control.h"
#include "monitor.h"

/* This function should be called in the platform's main. It allows
   the monitor device to take endianness into account and reduce the
   duration of the record functions from the timestamps. */
void monitor_init(void)
{
  uint16_t info = 0xd00d;

  /* Initialize the monitoring device. */
  mon_init();

  /* Check endianness. */
  mon_record(MON_CT_CONTROL, MON_ENT_CAL, MON_ST_CHECK);

  /* Check record duration. */
  mon_record(MON_CT_CONTROL, MON_ENT_CAL, 0);

  /* Check info duration. We do this two times to know additional number
     of cycles per byte in the info buffer.
     But first a dummy call to ensure uniform calling.
     Otherwise we have an extra operation to fetch the
     function pointer and the cycle count would be wrong. */
  mon_info(MON_CT_CONTROL, MON_ENT_CAL, &info, 1);
  mon_info(MON_CT_CONTROL, MON_ENT_CAL, &info, 1);
  mon_info(MON_CT_CONTROL, MON_ENT_CAL, &info, 2);
}

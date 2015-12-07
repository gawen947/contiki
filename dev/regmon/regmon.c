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
 *         A fast monitor device in MSPSIM that lies directly in the CPU.
 * \author
 *         David Hauweele <david@hauweele.net>
 */

#if !defined(__MSP430__)
# error "require MSP430 on modified MSPSIM"
#endif

#include <stdint.h>
#include <msp430.h>

#include "mon/monitor.h"
#include "dev/regmon/regmon.h"
#include "net/ip/uip.h"

#define msp_htons(v) ({ int _r; asm volatile ("swpb %0" : "=r" (_r) : "r" (v)); _r; })

static void record(int context, int entity, int state)
{
  MONCTX = context;
  MONENT = entity;
  MONSTI = state;

  /* trigger */
  MONCTL |= 0x100;
}

static void info(int context, int entity, const void *info, int len)
{
  MONCTX = context;
  MONENT = entity;
  MONSTI = (unsigned int)info;

  /* set len and trigger */
  MONCTL |= len | 0x300;
}

static void init(void) {}

const struct monitor_driver regmon_driver = {
  "regmon",
  record,
  info,
  init
};

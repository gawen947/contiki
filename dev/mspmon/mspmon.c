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
 *         A 2 wires monitor device on the MSP430.
 * \author
 *         David Hauweele <david@hauweele.net>
 */

#if !defined(__MSP430__)
# error "require MSP430"
#endif

#include <stdint.h>
#include <msp430.h>

#include "mon/monitor.h"
#include "dev/mspmon/mspmon.h"
#include "net/ip/uip.h"

/* Rename macro for easier expressions. */
#define POUT MONITOR_MSPMON_POUT
#define PDIR MONITOR_MSPMON_PDIR
#define DATA_PIN MONITOR_MSPMON_DATA
#define CLK_PIN  MONITOR_MSPMON_CLK
#define DATA (1 << DATA_PIN)
#define CLK  (1 << CLK_PIN)

static void send_u16(uint16_t value)
{
  uint16_t bit;

  for(bit = -1 ; bit ; bit >>=1, value <<= 1) {
    /* data */
    POUT = (POUT & ~DATA) | \
           ((value >> (15 - DATA_PIN)) & DATA);

    /* clock */
    POUT ^= CLK;
  }
}

static void send_u8(uint16_t value)
{
  static uint8_t bit;

  for(bit = -1 ; bit ; bit >>= 1, value <<= 1) {
    /* data */
    POUT = (POUT & ~DATA) | \
           ((value >> (7 - DATA_PIN)) & DATA);

    /* clock */
    POUT ^= CLK;
  }
}

static void record(int context, int entity, int state)
{
  send_u16(context);
  send_u16(entity);
  send_u16(state);
}

static void info(int context, int entity, const void *info, int len)
{
  send_u16(context);
  send_u16(entity);
  send_u16(MON_ST_INFO);

  send_u8(len);
  while(len--)
    send_u8(*(uint8_t *)info++);
}

static void init(void)
{
  MONITOR_MSPMON_PDIR |= (1 << MONITOR_MSPMON_DATA) | \
                         (1 << MONITOR_MSPMON_CLK);
}

const struct monitor_driver mspmon_driver = {
  "mspmon",
  record,
  info,
  init
};

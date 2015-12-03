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
 *         A monitor that uses UART to record events.
 * \author
 *         David Hauweele <david@hauweele.net>
 */

#include <stdint.h>

#include "mon/monitor.h"
#include "mon/uartmon.h"
#include "net/ip/uipopt.h"
#include "dev/serial-line.h"

#if CONTIKI_TARGET_Z1
# include "dev/uart0.h"
# define uart_writeb uart0_writeb
# define uart_set_input uart0_set_input
#else
# include "dev/uart1.h"
# define uart_writeb uart1_writeb
# define uart_set_input uart1_set_input
#endif

static void send_u16(uint16_t value)
{
#if UIP_BYTE_ORDER == BIG_ENDIAN
  uart_writeb(value & 0xff);
  uart_writeb(value >> 8);
#else /* LITTLE_ENDIAN */
  uart_writeb(value >> 8);
  uart_writeb(value & 0xff);
#endif
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

  uart_writeb(len);
  while(len--)
    uart_writeb(*(uint8_t *)info++);
}

void init(void)
{
  uart_set_input(serial_line_input_byte);
  serial_line_init();
}

const struct monitor_driver uartmon_driver = {
  "uartmon",
  record,
  info,
  init
};

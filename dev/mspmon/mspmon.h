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

#ifndef MSPMON_H_
#define MSPMON_H_

#include "mon/monitor.h"
#include "contiki-conf.h"

#ifdef MONITOR_MSPMON_PORT_CONF
#define MONITOR_MSPMON_PORT MONITOR_MSPMON_PORT_CONF
#else /* MONITOR_MSPMON_PORT_CONF */
#define MONITOR_MSPMON_PDIR P1DIR
#define MONITOR_MSPMON_POUT P1OUT
#endif /* MONITOR_MSPMON_PORT_CONF */

#ifdef MONITOR_MSPMON_DATA_CONF
#define MONITOR_MSPMON_DATA MONITOR_MSPMON_DATA_CONF
#else /* MONITOR_MSPMON_DATA_CONF */
#define MONITOR_MSPMON_DATA 2
#endif /* MONITOR_MSPMON_DATA_CONF */

#ifdef MONITOR_MSPMON_CLK_CONF
#define MONITOR_MSPMON_CLK MONITOR_MSPMON_CLK_CONF
#else /* MONITOR_MSPMON_CLK_CONF */
#define MONITOR_MSPMON_CLK 1
#endif /* MONITOR_MSPMON_CLK_CONF */

extern const struct monitor_driver mspmon_driver;

#endif /* MSPMON_H_ */

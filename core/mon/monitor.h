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

#ifndef MONITOR_H_
#define MONITOR_H_

#include <stdint.h>

#include "contiki-conf.h"
#include "mon/states.h"

#ifdef MONITOR_DEVICE_CONF
# define MONITOR_DEVICE MONITOR_DEVICE_CONF
# define monitor_record MONITOR_DEVICE.record
# define monitor_info   MONITOR_DEVICE.info
void monitor_init(void);
extern const struct monitor_driver MONITOR_DEVICE;
#else
/* If the monitor is not enabled,
   there is no additional overhead. */
# define monitor_record(...) ((void)0)
# define monitor_info(...)   ((void)0)
# define monitor_init(...)   ((void)0)
#endif

#define monitor_create(context, entity)  monitor_record((context), (entity), MON_ST_CREATE)
#define monitor_destroy(context, entity) monitor_record((context), (entity), MON_ST_DESTROY)

/**
 *  The Contiki Monitor device driver.
 *
 *  This device should record state changes, associate information to
 *  each entity, append time information to each event and organize
 *  them for further analysis.
 */
struct monitor_driver {
  char *name;

  /** \brief Record a state change for an entity inside a state machine.
      \param context The state machine identification.
      \param entity  The uniquely identified entity within the state machine.
      \param state   The new state of the entity.
  */
  void (* record)(int context, int entity, int state);

  /** \brief Provide information for the specified entity.
      \param context The state machine for the entity.
      \param entity  An uniquely identified entity (must already be created).
      \param info    A buffer containing the extra information.
      \param len     The length of the information buffer.
  */
  void (* info)(int context, int entity, const void *info, int len);

  /** \brief Initialize the monitoring device. */
  void (* init)(void);
};

#endif /* MONITOR_H_ */

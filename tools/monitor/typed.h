/* Copyright (c) 2016, David Hauweele <david@hauweele.net>
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

#ifndef _TYPED_H_
#define _TYPED_H_

#include <stdint.h>

#include "parser.h"

enum data_type {
  T_SCOPE_BITMASK, /* scope bitmask (as used in the parser scope structure) */
  T_EVENT_TYPE,    /* one event type */
  T_MON_CONTEXT,   /* context ID from monitor */
  T_MON_ENTITY,    /* entity ID from monitor */
  T_MON_STATE,     /* state ID from monitor */
  T_POSITION,      /* one coordinate for a position */
  T_TIME_MS,       /* time in milliseconds */
  T_TIME_US,       /* time in microseconds */
  T_CYCLES,        /* number of CPU cycles */
  T_DATA_FREED,    /* binary data freed after displaying */
  T_DATA,          /* binary data (not freed) */
  T_ID             /* identifier */
};

/* Events and scopes are comprised of data in the form of
   many different types. Here we provide a way to display
   and manipulate each of those types.

   To do so, the data are wrapped into this generic type
   structure. As a result the data is considerably wider.
   But this should only be used as a final stage of the
   data processing (generally when displaying it). */
struct typed {
  enum data_type type;

  union {
    uint32_t scope_used;
    enum event_element_type event_type;
    unsigned short mon_context;
    struct {
      unsigned short context;
      unsigned short entity;
    } mon_entity;
    struct {
      unsigned short context;
      unsigned short state;
    } mon_state;
    double position;
    double time_ms;
    uint64_t time_us;
    uint64_t cycles;
    struct {
      unsigned int length;
      void *data;
    } data;
    unsigned int id;
  } value;
};

/* Display type using printf. */
void display_typed_human(const struct typed *t);
void display_typed_normal(const struct typed *t);


#endif /* _TYPED_H_ */

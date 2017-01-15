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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "mon-names.h"
#include "dump.h"
#include "typed.h"

static void display_data_normal(const struct typed *t)
{
  const uint8_t *data = (uint8_t *)t->value.data.data;
  unsigned int i;

  for(i = 0 ; i < t->value.data.length ; i++)
    printf("%02X", data[i]);
}

static void display_data_human(const struct typed *t)
{
  fputc('\n', stdout);
  hex_dump(t->value.data.data, t->value.data.length);
}

static void display_scope_bitmask_normal(const struct typed *t)
{
  int prec = 0;
  uint32_t used = t->value.scope_used;

  if(used & SC_UNKNOWN) {
    printf("UNKNOWN");
    prec = 1;
  }
  if(used & SC_SIMULATION) {
    printf("%sSIMULATION", (prec == 1 ? "/" : ""));
    prec = 1;
  }
  if(used & SC_NODE)
    printf("%sNODE", (prec == 1 ? "/" : ""));

  if(used == 0) /* no scope used */
    printf("NONE");
}

static void display_scope_bitmask_human(const struct typed *t)
{
  int prec = 0;
  uint32_t used = t->value.scope_used;

  if(used & SC_UNKNOWN) {
    printf("unknown");
    prec = 1;
  }
  if(used & SC_SIMULATION) {
    printf("%ssimulation", (prec == 1 ? " " : ""));
    prec = 1;
  }
  if(used & SC_NODE) {
    printf("%snode", (prec == 1 ? " " : ""));
  }

  if(used == 0) /* no scope used */
    printf("none");
}

static void display_event_type_normal(const struct typed *t)
{
  switch(t->value.event_type) {
  case EV_T_UNKNOWN:
    printf("UNKNOWN");
    break;
  case EV_T_MON_CREATE:
    printf("MON_CREATE");
    break;
  case EV_T_MON_STATE:
    printf("MON_STATE");
    break;
  case EV_T_MON_DATA:
    printf("MON_DATA");
    break;
  case EV_T_NODE_CREATE:
    printf("NODE_CREATE");
    break;
  case EV_T_NODE_DESTROY:
    printf("NODE_DESTROY");
    break;
  case EV_T_NODE_POSITION:
    printf("NODE_POSITION");
    break;
  default:
    assert(0);
  }
}

static void display_event_type_human(const struct typed *t)
{
  switch(t->value.event_type) {
  case EV_T_UNKNOWN:
    printf("Unknown event");
    break;
  case EV_T_MON_CREATE:
    printf("Monitor creation");
    break;
  case EV_T_MON_STATE:
    printf("State event");
    break;
  case EV_T_MON_DATA:
    printf("Data event");
    break;
  case EV_T_NODE_CREATE:
    printf("Node creation");
    break;
  case EV_T_NODE_DESTROY:
    printf("Node removal");
    break;
  case EV_T_NODE_POSITION:
    printf("Node position");
    break;
  default:
    assert(0);
  }
}

static void display_mon_ids(const struct typed *t)
{
  const char *name;

  switch(t->type) {
  case T_MON_CONTEXT:
    name = get_context_name_or_id(t->value.mon_context);
    break;
  case T_MON_ENTITY:
    name = get_entity_name_or_id(t->value.mon_entity.context,
                                 t->value.mon_entity.entity);
    break;
  case T_MON_STATE:
    name = get_state_name_or_id(t->value.mon_state.context,
                                t->value.mon_state.state);
    break;
  default:
    assert(0);
  }

  fputs(name, stdout);
}

void display_typed_normal(const struct typed *t)
{
  switch(t->type) {
  case T_SCOPE_BITMASK:
    display_scope_bitmask_normal(t);
    break;
  case T_EVENT_TYPE:
    display_event_type_normal(t);
    break;
  case T_MON_CONTEXT:
  case T_MON_ENTITY:
  case T_MON_STATE:
    display_mon_ids(t);
    break;
  case T_POSITION:
  case T_TIME_MS:
    printf("%f", t->value.position);
    break;
  case T_CYCLES:
  case T_TIME_US:
    printf("%lu", t->value.cycles);
    break;
  case T_DATA:
    display_data_normal(t);
    break;
  case T_DATA_FREED:
    display_data_normal(t);
    free(t->value.data.data);
    break;
  case T_ID:
    printf("%d", t->value.id);
    break;
  default:
    assert(0);
  }
}

void display_typed_human(const struct typed *t)
{
  switch(t->type) {
  case T_SCOPE_BITMASK:
    display_scope_bitmask_human(t);
    break;
  case T_EVENT_TYPE:
    display_event_type_human(t);
    break;
  case T_MON_CONTEXT:
  case T_MON_ENTITY:
  case T_MON_STATE:
    display_mon_ids(t);
    break;
  case T_POSITION:
    printf("%f", t->value.position);
    break;
  case T_TIME_MS:
    printf("%f ms", t->value.time_ms);
    break;
  case T_TIME_US:
    printf("%lu us", t->value.time_us);
    break;
  case T_CYCLES:
    printf("%lu cycles", t->value.cycles);
    break;
  case T_DATA:
    display_data_human(t);
    break;
  case T_DATA_FREED:
    display_data_human(t);
    free(t->value.data.data);
    break;
  case T_ID:
    printf("%d", t->value.id);
    break;
  default:
    assert(0);
  }
}

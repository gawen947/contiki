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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "mode.h"
#include "typed.h"

#define MIN_SCOPE_LIST_SIZE 16
#define MIN_EVENT_LIST_SIZE 16

#define MAX_SCOPE_LIST_SIZE 256
#define MAX_EVENT_LIST_SIZE 256

#define HUMAN_DISPLAY_OFFSET 2

/* Instead of separate display functions for each event,
   we assemble all data to be displayed in a display list
   that is later displayed with a common display function.
   This ensures that we display in both normal/human mode
   using a consistent format. It also helps a lot aligning
   the fields in the human display mode. */
struct display_element {
  const char *name;        /* name of the element */
  const char *human_name;  /* name of the element with the human option */

  struct typed data;       /* typed data (see typed.h) */
};

/* The display lists are implemented as two dynamic arrays.
   As the lists are constructed for each event, we try to
   avoid allocation overhead. */
static struct display_list {
  struct display_element *list;

  unsigned int max_size;
  unsigned int size;
  unsigned int idx;
} scope_list, event_list;

static unsigned long event_id;

static void display_list_normal(const struct display_list *l)
{
  const struct display_element *e;
  int first = 1;

  for(e = l->list ; e->name ; e++) {
    if(!first)
      fputc(' ', stdout);

    fputs(e->name, stdout);
    fputc('=', stdout);
    display_typed_normal(&e->data);

    first = 0;
  }
}

/* Find maximum of a display list. We use this for alignment
   purposes when displaying event with the human flag set. */
static unsigned int display_list_max(struct display_list *l)
{
  const struct display_element *e;
  unsigned int size;
  unsigned int max = 0;

  /* max field size */
  for(e = l->list ; e->human_name ; e++) {
    size = strlen(e->human_name);

    if(size > max)
      max = size;
  }

  return max;
}

static void display_list_human(struct display_list *l, unsigned int max)
{
  const struct display_element *e;
  int size;

  /* display each field */
  for(e = l->list ; e->human_name ; e++) {
    /* pad before field name */
    size = HUMAN_DISPLAY_OFFSET;
    while(size--)
      fputc(' ', stdout);

    fputs(e->human_name, stdout);

    /* pad after field name */
    size = strlen(e->human_name);
    for(; size <= max ; size++)
      fputc(' ', stdout);

    /* separator */
    fputs(": ", stdout);

    /* display field data */
    display_typed_human(&e->data);

    fputc('\n', stdout);
  }
}

static void init_list(struct display_list *l, unsigned int size, unsigned int max_size)
{
  l->idx      = 0;
  l->size     = size;
  l->max_size = max_size;

  l->list = malloc(sizeof(struct display_element) * size);

  if(!l->list)
    err(EXIT_FAILURE, "cannot create display lists");
}

static struct display_element * list_add(struct display_list *l)
{
  if(l->idx > l->size) {
    l->size <<= 1;

    /* This shouldn't happen as we have carefully choosen
       the default and max size so that all the data fields
       from any event can fit the display list. */
    if(l->size > l->max_size)
      errx(EXIT_FAILURE, "too much fields in event");

    l->list = realloc(l->list, l->size * sizeof(struct display_element));
  }

  return l->list + l->idx++;
}

/* module initialization */
static void before(const struct context *ctx)
{
  event_id = 0;

  /* initialize dynamic arrays */
  init_list(&scope_list, MIN_SCOPE_LIST_SIZE, MAX_SCOPE_LIST_SIZE);
  init_list(&event_list, MIN_EVENT_LIST_SIZE, MAX_EVENT_LIST_SIZE);
}

/* module destruction */
static void after(const struct context *ctx)
{
  free(scope_list.list);
  free(event_list.list);
}

static void process_scope(const struct scope *scope)
{
  struct display_element *e;

  scope_list.idx = 0; /* reset */

  e  = list_add(&scope_list);
  *e = (struct display_element){ "SCOPES_USED", "Scopes used",
                                 { T_SCOPE_BITMASK,
                                   .value.scope_used = scope->scope } };

  if(scope->scope & SC_SIMULATION) {
    e  = list_add(&scope_list);
    *e = (struct display_element){ "SIM_TIME_US", "Simulation time (micros.)",
                                   { T_TIME_US,
                                     .value.time_us = scope->sim_us } };
  }

  if(scope->scope & SC_NODE) {
    e = list_add(&scope_list);
    *e = (struct display_element){ "NID", "Node ID",
                                   { T_ID,
                                     .value.id = scope->nid } };
    e  = list_add(&scope_list);
    *e = (struct display_element){ "NODE_TIME_MS", "Millis. since boot",
                                   { T_TIME_MS,
                                     .value.time_ms = scope->node_time.node_ms } };
    e  = list_add(&scope_list);
    *e = (struct display_element){ "NODE_CYCLES", "CPU cycles since boot",
                                   { T_CYCLES,
                                     .value.cycles = scope->node_time.cycles } };
  }

  /* close the list */
  e  = list_add(&scope_list);
  *e = (struct display_element){ NULL, NULL, {} };
}

/* Pre-process common for all events. */
static void pre_process_event(const struct scope *scope, enum event_element_type type)
{
  struct display_element *e;

  event_list.idx = 0; /* reset */

  process_scope(scope);

  e  = list_add(&event_list);
  *e = (struct display_element){ "EVENT_TYPE", "Event type",
                                 { T_EVENT_TYPE,
                                   .value.event_type = type } };
}

/* Post-process common for all events. */
static void post_process_event(struct context *ctx)
{
  struct display_element *e;

  /* close the list */
  e  = list_add(&event_list);
  *e = (struct display_element){ NULL, NULL, {} };

  if(ctx->human) {
    unsigned int max_scope = display_list_max(&scope_list);
    unsigned int max_event = display_list_max(&event_list);
    unsigned int max;

    printf("Event: %lu\n", event_id++);

    max = max_scope < max_event ? max_event : max_scope;

    display_list_human(&scope_list, max);
    display_list_human(&event_list, max);

    fputc('\n', stdout);
  } else {
    printf("EVENT=%lu ", event_id++);

    display_list_normal(&scope_list);
    fputc(':', stdout);
    display_list_normal(&event_list);

    fputc('\n', stdout);
  }
}

static int ev_unknown(const struct scope *scope, const void *null, void *data)
{
  pre_process_event(scope, EV_T_UNKNOWN);
  post_process_event((struct context *)data);

  return 0;
}

/* add a display element for time offset in mon_create events */
static void display_add_offset(const struct node_time *offset,
                               const char *normal_name_time,
                               const char *human_name_time,
                               const char *normal_name_cycles,
                               const char *human_name_cycles)
{
  struct display_element *e;

  /* time ms */
  e  = list_add(&event_list);
  *e = (struct display_element){ normal_name_time, human_name_time,
                                 { T_TIME_MS,
                                   .value.time_ms = offset->node_ms } };

  /* cycles */
  e  = list_add(&event_list);
  *e = (struct display_element){ normal_name_cycles, human_name_cycles,
                                 { T_CYCLES,
                                   .value.cycles = offset->cycles } };
}

static int ev_mon_create(const struct scope *scope, const struct ev_mon_create *event, void *data)
{
  pre_process_event(scope, EV_T_MON_CREATE);

  display_add_offset(&event->state_offset,
                     "STATE_OFFSET_TIME", "State offset in millis",
                     "STATE_OFFSET_CYCLES", "State offset in CPU cycles");
  display_add_offset(&event->data_offset,
                     "DATA_OFFSET_TIME", "Data offset in millis",
                     "DATA_OFFSET_CYCLES", "Data offset in CPU cycles");
  display_add_offset(&event->byte_offset,
                     "BYTE_OFFSET_TIME", "Byte offset in millis",
                     "BYTE_OFFSET_CYCLES", "Byte offset in CPU cycles");

  post_process_event((struct context *)data);

  return 0;
}

/* add a display element for context in state/data events */
static void display_add_context(unsigned short context, const char *normal, const char *human)
{
  struct display_element *e;

  e  = list_add(&event_list);
  *e = (struct display_element){ normal, human, { T_MON_CONTEXT, .value.mon_context = context }};
}

static void display_add_state(unsigned short context,
                              unsigned short state,
                              const char *normal, const char *human)
{
  struct display_element *e;

  e  = list_add(&event_list);
  *e = (struct display_element){ normal, human, { T_MON_STATE,
                                                  .value.mon_state = { .context = context,
                                                                       .state   = state } } };
}

static void display_add_entity(unsigned short context,
                               unsigned short entity,
                               const char *normal, const char *human)
{
  struct display_element *e;

  e  = list_add(&event_list);
  *e = (struct display_element){ normal, human, { T_MON_ENTITY,
                                                  .value.mon_entity = { .context  = context,
                                                                        .entity   = entity } } };
}


static int ev_mon_state(const struct scope *scope, const struct ev_mon_state *event, void *data)
{
  pre_process_event(scope, EV_T_MON_STATE);

  display_add_context(event->context, "CTX", "Context");
  display_add_entity(event->context, event->entity, "ENT", "Entity");
  display_add_state(event->context, event->state, "STATE", "State");

  post_process_event((struct context *)data);

  return 0;
}

static int ev_mon_data(const struct scope *scope, const struct ev_mon_data *event, void *data)
{
  struct display_element *e;

  pre_process_event(scope, EV_T_MON_DATA);

  display_add_context(event->context, "CTX", "Context");
  display_add_entity(event->context, event->entity, "ENT", "Entity");

  e  = list_add(&event_list);
  *e = (struct display_element){ "DATA", "Data",
                                 { T_DATA_FREED, /* freed after type displaying */
                                   .value.data = { .length = event->length,
                                                   .data   = event->data } } };

  post_process_event((struct context *)data);

  return 0;
}

static int ev_node_create(const struct scope *scope, const void *null, void *data)
{
  pre_process_event(scope, EV_T_NODE_CREATE);
  post_process_event((struct context *)data);

  return 0;
}

static int ev_node_destroy(const struct scope *scope, const void *null, void *data)
{
  pre_process_event(scope, EV_T_NODE_DESTROY);
  post_process_event((struct context *)data);

  return 0;
}

/* add a display element for a coordinate in position events */
static void display_add_coord(double coord, const char *normal, const char *human)
{
  struct display_element *e;

  e  = list_add(&event_list);
  *e = (struct display_element){ normal, human, { T_POSITION, .value.position = coord }};
}


static int ev_node_position(const struct scope *scope, const struct ev_node_position *event, void *data)
{
  pre_process_event(scope, EV_T_NODE_POSITION);

  display_add_coord(event->x, "COORD_X", "Coord. X");
  display_add_coord(event->y, "COORD_Y", "Coord. Y");
  display_add_coord(event->z, "COORD_Z", "Coord. Z");

  post_process_event((struct context *)data);

  return 0;
}

struct output_mode print_mode = {
  .name        = "print",
  .description = "Print each event on the standard output.",

  .before = before,
  .after  = after,

  .parsed_by_events = {
    .unknown       = ev_unknown,
    .mon_create    = ev_mon_create,
    .mon_state     = ev_mon_state,
    .mon_data      = ev_mon_data,
    .node_create   = ev_node_create,
    .node_destroy  = ev_node_destroy,
    .node_position = ev_node_position
  }
};

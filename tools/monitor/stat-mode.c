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

#include "utils.h"
#include "mode.h"
#include "trace.h"
#include "stat-mode.h"

#define MAX_SEEN 16

static struct {
  /* number of events */
  unsigned long nb_unknown;
  unsigned long nb_mon_create;
  unsigned long nb_mon_state;
  unsigned long nb_mon_data;
  unsigned long nb_node_create;
  unsigned long nb_node_destroy;
  unsigned long nb_node_position;

  /* number of scopes */
  unsigned long nb_sc_unknown;
  unsigned long nb_sc_simulation;
  unsigned long nb_sc_node;

  /* number of nodes */
  uint64_t *seen_nodes;

  /* contexts, entities, and states observed */
  uint64_t *seen_contexts;
  uint64_t *seen_entities;
  uint64_t *seen_states;
} statistics;

struct display_list {
  const char *name;
  const char *human_name;
  const char *unit;

  union {
    unsigned long count;
    uint64_t     *seen;
  } value;
};

#define SEEN(seen_array, id)    seen_array[id / 64] |= 1LLU << (id % 64)
#define IS_SEEN(seen_array, id) seen_array[id / 64] & (1LLU << (id % 64))

static void before(const struct context *ctx)
{
  /* The stat module requires 32k for the seen arrays.
     Using HT we can probably reduce it to <16k but
     that would be slower. */
  statistics.seen_nodes    = (uint64_t *)xmalloc0(0xffff >> 3);
  statistics.seen_contexts = (uint64_t *)xmalloc0(0xffff >> 3);
  statistics.seen_entities = (uint64_t *)xmalloc0(0xffff >> 3);
  statistics.seen_states   = (uint64_t *)xmalloc0(0xffff >> 3);
}

static unsigned long sum_seen(uint64_t *seen) {
  unsigned long count = 0;
  int n = 64;
  int i;

  for(i = 0 ; i <= (0xffff >> 6) ; i++) {
    for(n = 63 ; n >= 0 ; n--)
      count += (seen[i] >> n) & 1;
  }

  return count;
};

static void display_counter_human(const struct display_list *e, const char *unit)
{
  if(!unit)
    printf(": %lu", e->value.count);
  else
    printf(": %lu %s", e->value.count, unit);
}

static void display_seen_human(const struct display_list *e, const char *unit)
{
  int count = 0;
  int i;

  fputc(':', stdout);

  for(i = 0 ; i < 0xffff ; i++) {
    if(IS_SEEN(e->value.seen, i)) {
      printf(" %04x", i);

      if(++count > MAX_SEEN) {
        printf(" ...");
        return;
      }
    }
  }
}

static void display_list_human(const struct display_list list[],
                               void (*display)(const struct display_list *e,
                                               const char *unit))
{
  const struct display_list *l;
  const char *default_unit = NULL, *unit;
  int size;
  int max = 0;
  int first = 1;

  /* Maximum statistics names. */
  for(l = list ; l->human_name ; l++) {
    if(!l->name)
      continue; /* skip titles */
    if(l->value.count == 0)
      continue; /* skip empty values */


    size = strlen(l->human_name);
    if(size > max)
      max = size;
  }

    /* Print statistics */
  for(l = list ; l->human_name ; l++) {
    size = strlen(l->human_name);

    if(!l->name) { /* title */
      if(!first) /* newline before title */
        fputc('\n', stdout);

      default_unit = l->unit;

      fputs(l->human_name, stdout);
      fputc('\n', stdout);

      while(size--)
        fputc('-', stdout);

      fputs("\n\n", stdout);;

      first = 0;
    } else {
      if(l->value.count == 0)
        continue; /* skip empty values */

      fputs(l->human_name, stdout);

      /* pad */
      for(; size <= max ; size++)
        fputc(' ', stdout);

      /* choose default unit when needed */
      unit = l->unit ? l->unit : default_unit;

      display(l, unit);

      fputc('\n', stdout);
    }
  }
}

static void display_counter_normal(const struct display_list *e)
{
  printf(":%lu", e->value.count);
}

static void display_seen_normal(const struct display_list *e)
{
  /* For now the normal/human versions are
     displayed using the same format. */
  display_seen_human(e, NULL);
}

static void display_list_normal(const struct display_list list[],
                                void (*display)(const struct display_list *e))
{
  const struct display_list *l;

  for(l = list ; l->human_name ; l++) {
    if(!l->name) /* skip title */
      continue;

    fputs(l->name, stdout);
    display(l);
    fputc('\n', stdout);
  }
}

static void after(const struct context *ctx)
{
  struct display_list counters[] = {
    { NULL, "Number of events", "times", .value.count = 0 },
    { "ev_unknown", "Unknown", NULL, .value.count = statistics.nb_unknown },
    { "ev_mon_create", "Mon create", NULL, .value.count = statistics.nb_mon_create },
    { "ev_mon_state", "Mon state", NULL, .value.count = statistics.nb_mon_state },
    { "ev_mon_data", "Mon data", NULL, .value.count = statistics.nb_mon_data },
    { "ev_node_create", "Node create", NULL, .value.count = statistics.nb_node_create },
    { "ev_node_destroy", "Node destroy", NULL, .value.count = statistics.nb_node_destroy },
    { "ev_node_position", "Node position", NULL, .value.count = statistics.nb_node_position },

    { NULL, "Number of scopes", "times", .value.count = 0 },
    { "sc_unknown", "Unknown", NULL, .value.count = statistics.nb_sc_unknown },
    { "sc_simulation", "Simulation", NULL, .value.count = statistics.nb_sc_simulation },
    { "sc_node", "Node", NULL, .value.count = statistics.nb_sc_node },

    { NULL, "Elements seen", NULL, .value.count = 0 },
    { "nb_seen_nodes", "Nodes", "distinct nodes", .value.count = sum_seen(statistics.seen_nodes) },
    { "nb_seen_contexts", "Contexts", "distinct contexts", .value.count = sum_seen(statistics.seen_contexts) },
    { "nb_seen_entities", "Entities", "distinct entities", .value.count = sum_seen(statistics.seen_entities) },
    { "nb_seen_states", "States", "distinct states", .value.count = sum_seen(statistics.seen_states) },
    { NULL, NULL, NULL, .value.count = 0 }
  };

  struct display_list seen[] = {
    { NULL, "Seen values", .value.count = 0 },
    { "seen_nodes", "Nodes", .value.seen = statistics.seen_nodes },
    { "seen_contexts", "Contexts", .value.seen = statistics.seen_contexts },
    { "seen_entities", "Entities", .value.seen = statistics.seen_entities },
    { "seen_states", "States", .value.seen = statistics.seen_states },
    { NULL, NULL, .value.count = 0 }
  };

  /* FIXME: Use typed (typed.h) displays. */
  if(ctx->human) { /* human display */
    display_list_human(counters, display_counter_human);
    fputc('\n', stdout);
    display_list_human(seen, display_seen_human);
  } else {
    display_list_normal(counters, display_counter_normal);
    display_list_normal(seen, display_seen_normal);
  }

  free(statistics.seen_nodes);
  free(statistics.seen_contexts);
  free(statistics.seen_entities);
  free(statistics.seen_states);
}

static void process_scope(const struct scope *scope)
{
  if(scope->scope & SC_UNKNOWN)
    statistics.nb_sc_unknown++;
  if(scope->scope & SC_SIMULATION)
    statistics.nb_sc_simulation++;
  if(scope->scope & SC_NODE) {
    statistics.nb_sc_node++;
    SEEN(statistics.seen_nodes, scope->nid);
  }
}

static int ev_unknown(const struct scope *scope, const void *null, void *data)
{
  process_scope(scope);
  statistics.nb_unknown++;

  return 0;
}

static int ev_mon_create(const struct scope *scope, const struct ev_mon_create *event, void *data)
{
  process_scope(scope);
  statistics.nb_mon_create++;

  return 0;
}

static int ev_mon_state(const struct scope *scope, const struct ev_mon_state *event, void *data)
{
  process_scope(scope);
  statistics.nb_mon_state++;

  SEEN(statistics.seen_contexts, event->context);
  SEEN(statistics.seen_entities, event->entity);
  SEEN(statistics.seen_states, event->state);

  return 0;
}

static int ev_mon_data(const struct scope *scope, const struct ev_mon_data *event, void *data)
{
  process_scope(scope);
  statistics.nb_mon_data++;

  free(event->data); /* thanks for the alloc but we don't need it */

  return 0;
}

static int ev_node_create(const struct scope *scope, const void *null, void *data)
{
  process_scope(scope);
  statistics.nb_node_create++;

  return 0;
}

static int ev_node_destroy(const struct scope *scope, const void *null, void *data)
{
  process_scope(scope);
  statistics.nb_node_destroy++;

  return 0;
}

static int ev_node_position(const struct scope *scope, const struct ev_node_position *event, void *data)
{
  process_scope(scope);
  statistics.nb_node_position++;

  return 0;
}

struct output_mode stat_mode = {
  .name        = "stat",
  .description = "Display statistics.",

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

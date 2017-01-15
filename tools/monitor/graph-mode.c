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
#include <assert.h>
#include <float.h>

#include "hash.h"
#include "htable.h"
#include "mode.h"
#include "mon-names.h"
#include "graph-mode.h"

#define CONTEXT_HT_SIZE 16
#define STATE_HT_SIZE   32

#define MAX_LABEL_STR   255


/* Informations used to update label statistics. */
struct label_info {
  struct scope destination_scope;
  struct scope source_last_scope;
};

struct st_list {
  unsigned short state;
  struct label_stats label;

  struct st_list *next;
};

struct mon_ctx {
  unsigned short context;
  unsigned short last_state;

  htable_t state_ht;
};

struct mon_st {
  unsigned short context;
  unsigned short state;
  struct scope last_scope;

  struct st_list *adjacency;
};

/* Scope and state used within ev_mon_state() for state creation. */
struct mon_sc_st {
  const struct scope *scope;
  const struct ev_mon_state *event;
};

static htable_t context_ht;
static char label_str[MAX_LABEL_STR];

static const char * label_sim_us(const struct label_stats *stats);
static const char * label_cycles(const struct label_stats *stats);
static const char * label_node_ms(const struct label_stats *stats);
static const char * label_count(const struct label_stats *stats);

static const struct graph_metric avail_metric[] = {
  { "sim_us", "Simulation time in number of microsecs between each transition", label_sim_us},
  { "cycles", "Time in number of CPU cycles between each transition", label_cycles },
  { "node_ms", "Time in number of millisecs between each transition", label_node_ms },
  { "count", "Number of times this transition has been used", label_count },
  { NULL, NULL, NULL }
};

static const char * label_sim_us(const struct label_stats *stats)
{
  uint64_t min = stats->min_delta_sim_us;
  uint64_t max = stats->max_delta_sim_us;
  double avg = (double)stats->sum_delta_sim_us / stats->count;

  snprintf(label_str, sizeof(label_str), "min=%lu avg=%.3f max=%lu", min, avg, max);

  return label_str;
}

static const char * label_node_ms(const struct label_stats *stats)
{
  double min = stats->min_delta_node_ms;
  double max = stats->max_delta_node_ms;
  double avg = stats->sum_delta_node_ms / stats->count;

  snprintf(label_str, sizeof(label_str), "min=%.3f avg=%.3f max=%.3f", min, avg, max);

  return label_str;
}

static const char * label_cycles(const struct label_stats *stats)
{
  uint64_t min = stats->min_delta_cycles;
  uint64_t max = stats->max_delta_cycles;
  double avg = (double)stats->sum_delta_cycles / stats->count;

  snprintf(label_str, sizeof(label_str), "min=%lu avg=%.3f max=%lu", min, avg, max);

  return label_str;
}

static const char * label_count(const struct label_stats *stats)
{
  snprintf(label_str, sizeof(label_str), "count=%u", stats->count);

  return label_str;
}


const struct graph_metric * select_graph_metric_by_name(const char *name)
{
  const struct graph_metric *metric;

  for(metric = avail_metric ; metric->name ; metric++) {
    if(!strcmp(name, metric->name))
      return metric;
  }

  return NULL;
}

void walk_graph_metrics(void (*visit)(const struct graph_metric *metric, void *data), void *data)
{
  const struct graph_metric *metric;

  for(metric = avail_metric ; metric->name ; metric++)
    visit(metric, data);
}

static struct mon_st * create_state(const void *null, struct mon_sc_st *ev_data)
{
  struct mon_st *st = malloc(sizeof(struct mon_st));

  *st = (struct mon_st){ .context = ev_data->event->context, .state = ev_data->event->state, .adjacency = NULL };

  return st;
}

static void destroy_state(struct mon_st *st)
{
  struct st_list *s = st->adjacency;

  while(s) {
    struct st_list *e = s;
    s = s->next;
    free(e);
  }

  free(st);
}

static struct mon_ctx * create_context(const void *null, struct ev_mon_state *event)
{
  struct mon_ctx *ctx = malloc(sizeof(struct mon_ctx));
  if(!ctx)
    errx(EXIT_FAILURE, "Out of memory");

  ctx->context    = event->context;
  ctx->last_state = -1;
  ctx->state_ht   = ht_create(STATE_HT_SIZE, hash_mon_id, compare_mon_id, (void *)destroy_state);

  return ctx;
}

static void destroy_context(struct mon_ctx *ctx)
{
  ht_destroy(ctx->state_ht);
  free(ctx);
}

static void init_label_statistics(struct label_stats *label)
{
  label->min_delta_sim_us  = UINT64_MAX;
  label->min_delta_cycles  = UINT64_MAX;
  label->min_delta_node_ms = DBL_MAX;
}

static void update_label_statistics(struct label_stats *label, const struct label_info *info)
{
  label->count++;

  /* simulation event */
  if((info->source_last_scope.scope & SC_SIMULATION) && (info->destination_scope.scope & SC_SIMULATION)) {
    uint64_t src_sim_us = info->source_last_scope.sim_us;
    uint64_t dst_sim_us = info->destination_scope.sim_us;
    uint64_t delta = dst_sim_us - src_sim_us;

    label->sum_delta_sim_us += delta;
    if(delta < label->min_delta_sim_us)
      label->min_delta_sim_us = delta;
    if(delta > label->max_delta_sim_us)
      label->max_delta_sim_us = delta;
  }

  /* node event within the same node */
  if(((info->source_last_scope.scope & SC_NODE) && (info->destination_scope.scope & SC_NODE)) &&
     (info->source_last_scope.nid == info->destination_scope.nid)) {
    double   src_node_ms = info->source_last_scope.node_time.node_ms;
    double   dst_node_ms = info->destination_scope.node_time.node_ms;
    uint64_t src_cycles  = info->source_last_scope.node_time.cycles;
    uint64_t dst_cycles  = info->destination_scope.node_time.cycles;

    {
      double delta = dst_node_ms - src_node_ms;

      label->sum_delta_node_ms += delta;
      if(delta < label->min_delta_node_ms)
        label->min_delta_node_ms = delta;
      if(delta > label->max_delta_node_ms)
        label->max_delta_node_ms = delta;
    }

    {
      uint64_t delta = dst_cycles - src_cycles;

      label->sum_delta_cycles += delta;
      if(delta < label->min_delta_cycles)
        label->min_delta_cycles = delta;
      if(delta > label->max_delta_cycles)
        label->max_delta_cycles = delta;
    }
  }
}

static struct st_list * new_st(unsigned short state, const struct label_info *label_info)
{
  struct st_list *s = malloc(sizeof(struct st_list));
  if(!s)
    errx(EXIT_FAILURE, "Out of memory");

  *s = (struct st_list){ .state = state, .label = { 0 }, .next = NULL };

  init_label_statistics(&s->label);
  update_label_statistics(&s->label, label_info);

  return s;
}

/* Register in the specified context a transition from st_a to st_dst. */
static void register_transition(const struct mon_ctx *ctx, unsigned short st_a, unsigned short st_b, struct label_info *label_info)
{
  struct st_list *s;
  struct mon_st *st = ht_search(ctx->state_ht, TO_KEY(st_a), NULL);
  assert(st); /* state should already be registered */

  /* Update source scope. */
  memcpy(&label_info->source_last_scope, &st->last_scope, sizeof(struct scope));

  /* Most states should only have a few transitions.
     This is why we use a list instead of another HT
     for the adjacencies. */
  if(!st->adjacency) {
    st->adjacency = new_st(st_b, label_info);
    return;
  }

  for(s = st->adjacency ; s->next ; s = s->next) {
    if(s->state == st_b) {
      update_label_statistics(&s->label, label_info);
      return; /* Don't register the same transition twice. */
    }
  }

  if(s->state == st_b) {
    update_label_statistics(&s->label, label_info);
    return; /* Don't register the same transition twice. */
  }

  /* tail */
  s->next = new_st(st_b, label_info);
}

static void display_state_transitions(const struct mon_st *st, const struct context *app_ctx)
{
  struct st_list *s;

  /* We have to duplicate the string because get_*_name() use static buffers
     that would be overwritten if called twice in the same printf(). */
  char *orig_state_name = strdup(get_state_name_or_id(st->context, st->state));

  /* Always display the node itself
     in case it has no transitions. */
  printf("\"%s\";\n", orig_state_name);

  for(s = st->adjacency ; s ; s = s->next) {
    printf("\"%s\" -> \"%s\"", orig_state_name,
                               get_state_name_or_id(st->context, s->state));

    if(app_ctx->graph_metric) {
      const struct graph_metric *metric = app_ctx->graph_metric;
      const char *label = metric->label_transition(&s->label);

      if(label)
        printf(" [ label=\"%s\" ]", label);
    }

    putc('\n', stdout);
  }

  free(orig_state_name);
}

static void display_context_graph(const struct mon_ctx *ctx, const struct context *app_ctx)
{
  printf("digraph \"%s\" {\n", get_context_name_or_id(ctx->context));

  ht_walk(ctx->state_ht, (void *)display_state_transitions, (void *)app_ctx);

  printf("}\n");
}

/* module initialization */
static void before(const struct context *ctx)
{
  context_ht = ht_create(CONTEXT_HT_SIZE, hash_mon_id, compare_mon_id, (void *)destroy_context);
}

/* module destruction */
static void after(const struct context *ctx)
{
  /* Display the graphs.
     One for each context. */
  ht_walk(context_ht, (void *)display_context_graph, (void *)ctx);

  ht_destroy(context_ht);
}

static int ignored() { return 0; }

static int ev_mon_state(const struct scope *scope, const struct ev_mon_state *event, void *data)
{
  struct label_info label_info;
  struct mon_sc_st ev_data = { scope, event };
  struct mon_st  *st;
  struct mon_ctx *ctx = ht_lookup(context_ht, TO_KEY(event->context), (void *)create_context, (void *)event);
  if(!ctx)
    errx(EXIT_FAILURE, "Out of memory");

  st = ht_lookup(ctx->state_ht, TO_KEY(event->state), (void *)create_state, &ev_data);
  if(!st)
    errx(EXIT_FAILURE, "Out of memory");

  /* Record scope in label info. */
  memset(&label_info, 0, sizeof(struct label_info));
  memcpy(&label_info.destination_scope, scope, sizeof(struct scope));

  /* If we have already seen an event before in this context,
     then it is a transition. */
  if(ctx->last_state != (unsigned short)-1)
    register_transition(ctx, ctx->last_state, st->state, &label_info);

  ctx->last_state = event->state;
  memcpy(&st->last_scope, scope, sizeof(struct scope));

  return 0;
}


struct output_mode graph_mode = {
  .name        = "graph",
  .description = "Output dot graph of the components automata.",

  .before = before,
  .after  = after,

  .parsed_by_events = {
    .unknown       = ignored,
    .mon_create    = ignored,
    .mon_state     = ev_mon_state,
    .mon_data      = ignored,
    .node_create   = ignored,
    .node_destroy  = ignored,
    .node_position = ignored
  }
};

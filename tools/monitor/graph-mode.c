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

#include "hash.h"
#include "htable.h"
#include "mode.h"
#include "mon-names.h"
#include "graph-mode.h"

#define CONTEXT_HT_SIZE 16
#define STATE_HT_SIZE   32

struct st_list {
  unsigned short state;
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

  struct st_list *adjacency;
};

htable_t context_ht;

static struct mon_st * create_state(const void *null, struct ev_mon_state *event)
{
  struct mon_st *st = malloc(sizeof(struct mon_st));

  *st = (struct mon_st){ .context = event->context, .state = event->state, .adjacency = NULL };

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

static struct st_list * new_st(unsigned short state)
{
  struct st_list *s = malloc(sizeof(struct st_list));
  if(!s)
    errx(EXIT_FAILURE, "Out of memory");

  *s = (struct st_list){ .state = state, .next = NULL };

  return s;
}

/* Register in the specified context a transition from st_a to st_b. */
static void register_transition(const struct mon_ctx *ctx, unsigned short st_a, unsigned short st_b)
{
  struct st_list *s;
  struct mon_st *st = ht_search(ctx->state_ht, TO_KEY(st_a), NULL);
  assert(st); /* state should already be registered */

  /* Most states should only have a few transitions.
     This is why we use a list instead of another HT
     for the adjacencies. */
  if(!st->adjacency) {
    st->adjacency = new_st(st_b);
    return;
  }

  for(s = st->adjacency ; s->next ; s = s->next) {
    if(s->state == st_b)
      return; /* Don't register the same transition twice. */
  }

  if(s->state == st_b)
    return; /* Don't register the same transition twice. */

  /* tail */
  s->next = new_st(st_b);
}

static void display_state_transitions(const struct mon_st *st)
{
  struct st_list *s;

  /* We have to duplicate the string because get_*_name() use static buffers
     that would be overwritten if called twice in the same printf(). */
  char *orig_state_name = strdup(get_state_name_or_id(st->context, st->state));

  /* Always display the node itself
     in case it has no transitions. */
  printf("\"%s\";\n", orig_state_name);

  for(s = st->adjacency ; s ; s = s->next)
    printf("\"%s\" -> \"%s\";\n", orig_state_name,
                                  get_state_name_or_id(st->context, s->state));

  free(orig_state_name);
}

static void display_context_graph(const struct mon_ctx *ctx)
{
  printf("digraph \"%s\" {\n", get_context_name_or_id(ctx->context));

  ht_walk(ctx->state_ht, (void *)display_state_transitions);

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
  ht_walk(context_ht, (void *)display_context_graph);

  ht_destroy(context_ht);
}

static int ignored() { return 0; }

static int ev_mon_state(const struct scope *scope, const struct ev_mon_state *event, void *data)
{
  struct mon_st  *st;
  struct mon_ctx *ctx = ht_lookup(context_ht, TO_KEY(event->context), (void *)create_context, (void *)event);
  if(!ctx)
    errx(EXIT_FAILURE, "Out of memory");

  st = ht_lookup(ctx->state_ht, TO_KEY(event->state), (void *)create_state, (void *)event);
  if(!st)
    errx(EXIT_FAILURE, "Out of memory");

  /* If we have already seen an event before in this context,
     then it is a transition. */
  if(ctx->last_state != (unsigned short)-1)
    register_transition(ctx, ctx->last_state, event->state);

  ctx->last_state = event->state;

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

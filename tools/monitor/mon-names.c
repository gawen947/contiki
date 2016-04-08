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
#include <err.h>

#include "hash.h"
#include "htable.h"
#include "mon-names.h"

/* Here we associate a name to each context, or pair
   context-entity, context-state. These are stored in the
   form of small htables for fast lookups. */

#define CONTEXT_HT_SIZE 16
#define STATE_HT_SIZE   32 /* one for each context */
#define ENTITY_HT_SIZE  32 /* one for each context */
#define COMMON_HT_SIZE  8

struct context_entry {
  unsigned short context;
  const char *name;

  htable_t state_ht;
  htable_t entity_ht;
};

/* We use the same entry type for both states and entities,
   hence the name 'stent' entry. */
struct stent_entry {
  unsigned short ID;
  const char *name;
};

/* All lookup start with the context ID. */
static htable_t context_ht;
static htable_t common_state_ht;
static htable_t common_entity_ht;

/* If needed the resolution can be disabled.
   All functions always return NULL. */
static int enabled = 1;

static void destroy_context_entry(void *entry)
{
  struct context_entry *context_entry = (struct context_entry *)entry;

  /* We only destroy the htables for states and entities
     associated to this context. The name comes from
     a string litteral (read only). */
  ht_destroy(context_entry->state_ht);
  ht_destroy(context_entry->entity_ht);

  /* Now free the entry itself. */
  free(context_entry);
}

static void destroy_stent_entry(void *entry)
{
  /* Again the name comes from a string litteral.
     So we only have to free the entry itself. */
  free(entry);
}

void mon_names_init(void)
{
  context_ht       = ht_create(CONTEXT_HT_SIZE, hash_mon_id, compare_mon_id, destroy_context_entry);
  common_state_ht  = ht_create(COMMON_HT_SIZE, hash_mon_id, compare_mon_id, destroy_stent_entry);
  common_entity_ht = ht_create(COMMON_HT_SIZE, hash_mon_id, compare_mon_id, destroy_stent_entry);
}

void mon_names_destroy(void)
{
  ht_destroy(context_ht);
  ht_destroy(common_state_ht);
  ht_destroy(common_entity_ht);
}

void mon_names_set(int enabled_value)
{
  enabled = enabled_value;
}

void reg_context_name(unsigned short context, const char *name)
{
  struct context_entry *context_entry = malloc(sizeof(struct context_entry));
  if(!context_entry)
    errx(EXIT_FAILURE, "Out of memory");

  *context_entry = (struct context_entry){
    .context   = context,
    .name      = name,
    .state_ht  = ht_create(STATE_HT_SIZE, hash_mon_id, compare_mon_id, destroy_stent_entry),
    .entity_ht = ht_create(ENTITY_HT_SIZE, hash_mon_id, compare_mon_id, destroy_stent_entry)
  };

  if(!ht_search(context_ht, TO_KEY(context), context_entry))
    errx(EXIT_FAILURE, "Out of memory");
}

static void reg_stent_name(htable_t stent_ht, unsigned short ID, const char *name)
{
  struct stent_entry *stent_entry = malloc(sizeof(struct stent_entry));
  if(!stent_entry)
    errx(EXIT_FAILURE, "Out of memory");

  *stent_entry = (struct stent_entry){
    .ID   = ID,
    .name = name
  };

  if(!ht_search(stent_ht, TO_KEY(ID), stent_entry))
    errx(EXIT_FAILURE, "Out of memory");
}

void reg_common_state_name(unsigned short state, const char *name)
{
  reg_stent_name(common_state_ht, state, name);
}

void reg_common_entity_name(unsigned short entity, const char *name)
{
  reg_stent_name(common_entity_ht, entity, name);
}

void reg_state_name(unsigned short context, unsigned short state, const char *name)
{
  const struct context_entry *context_entry = ht_search(context_ht, TO_KEY(context), NULL);

  /* Non existing context entry. This should not happen as we should only
     add states/entities to context that are already registered. */
  if(!context_entry)
    errx(EXIT_FAILURE, "Unregistered context %04X", context);

  reg_stent_name(context_entry->state_ht, state, name);
}

void reg_entity_name(unsigned short context, unsigned short entity, const char *name)
{
  const struct context_entry *context_entry = ht_search(context_ht, TO_KEY(context), NULL);

  /* Non existing context entry. This should not happen as we should only
     add states/entities to context that are already registered. */
  if(!context_entry)
    errx(EXIT_FAILURE, "Unregistered context %04X", context);

  reg_stent_name(context_entry->entity_ht, entity, name);
}

const char * get_context_name(unsigned short context)
{
  if(!enabled)
    return NULL;

  const struct context_entry *context_entry = ht_search(context_ht, TO_KEY(context), NULL);
  if(!context_entry)
    return NULL;

  return context_entry->name;
}

static const char *get_stent_name(htable_t stent_ht, unsigned short ID)
{
  if(!enabled)
    return NULL;

  const struct stent_entry *stent_entry = ht_search(stent_ht, TO_KEY(ID), NULL);
  if(!stent_entry)
    return NULL;

  return stent_entry->name;
}

const char * get_state_name(unsigned short context, unsigned short state)
{
  const struct context_entry *context_entry;
  const struct stent_entry   *stent_entry;

  if(!enabled)
    return NULL;

  /* Check common state table first. */
  stent_entry = ht_search(common_state_ht, TO_KEY(state), NULL);
  if(stent_entry)
    return stent_entry->name;

  /* Get the context table and search its associated state table */
  context_entry = ht_search(context_ht, TO_KEY(context), NULL);
  if(!context_entry)
    return NULL;

  return get_stent_name(context_entry->state_ht, state);
}

const char * get_entity_name(unsigned short context, unsigned short entity)
{
  const struct context_entry *context_entry;
  const struct stent_entry   *stent_entry;

  if(!enabled)
    return NULL;

  /* Check common entity table first. */
  stent_entry = ht_search(common_entity_ht, TO_KEY(entity), NULL);
  if(stent_entry)
    return stent_entry->name;

  /* Get the context table and search its associated entity table */
  context_entry = ht_search(context_ht, TO_KEY(context), NULL);
  if(!context_entry)
    return NULL;

  return get_stent_name(context_entry->entity_ht, entity);
}

const char * get_context_name_or_id(unsigned short context)
{
  static char id_buf[5];
  const char *name = get_context_name(context);

  if(name)
    return name;

  snprintf(id_buf, sizeof(id_buf), "%04x", context);
  return id_buf;
}

const char * get_state_name_or_id(unsigned short context, unsigned short state)
{
  static char id_buf[5];
  const char *name = get_state_name(context, state);

  if(name)
    return name;

  snprintf(id_buf, sizeof(id_buf), "%04x", state);
  return id_buf;
}

const char * get_entity_name_or_id(unsigned short context, unsigned short entity)
{
  static char id_buf[5];
  const char *name = get_entity_name(context, entity);

  if(name)
    return name;

  snprintf(id_buf, sizeof(id_buf), "%04x", entity);
  return id_buf;
}

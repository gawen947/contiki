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

#ifndef _MON_NAMES_H_
#define _MON_NAMES_H_

/* Initialize and free the structures that contains the list of names for monitor IDs. */
void mon_names_init(void);
void mon_names_destroy(void);

/* Enable or disable IDs names resolution.
   If disabled the get functions always
   return NULL. */
void mon_names_set(int enabled);

/* Register a name for a monitor ID. Note that for states and entities, the context
   must already be registered with a proper name, otherwise an error is generated.
   Also note that you should register a name only to avoid memory leaks.

   The common state and entity registering functions are for monitor IDs that are
   common to all context. */
void reg_context_name(unsigned short context, const char *name);
void reg_state_name(unsigned short context, unsigned short state, const char *name);
void reg_entity_name(unsigned short context, unsigned short entity, const char *name);
void reg_common_state_name(unsigned short state, const char *name);
void reg_common_entity_name(unsigned short state, const char *name);

/* Get a previously registered name for a monitor ID. If the ID was not registered
   then NULL is returned instead. */
const char * get_context_name(unsigned short context);
const char * get_state_name(unsigned short context, unsigned short state);
const char * get_entity_name(unsigned short context, unsigned short entity);

/* Same as the previous function but returns a hexadecimal number if the ID was
   not registered. */
const char * get_context_name_or_id(unsigned short context);
const char * get_state_name_or_id(unsigned short context, unsigned short state);
const char * get_entity_name_or_id(unsigned short context, unsigned short state);

#endif /* _MON_NAMES_H_ */

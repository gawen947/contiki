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

#ifndef _PARSER_H_
#define _PARSER_H_

#include <limits.h>
#include <stdint.h>

#include "iobuf.h"

#define PARSER_MAJOR_VERSION 1
#define PARSER_MINOR_VERSION 0

enum trace_error {
  E_SUCCESS = 0,   /* no error */
  E_EOF = INT_MIN, /* EOF reached (no more events) */
  E_SHORT,         /* missing data (falling short) */
  E_IOERR,         /* IO error */
  E_MAGIK,         /* invalid magik numbers */
  E_MAJOR,         /* major version mismatch */
  E_NOMEM,         /* out of memory during parsing */
  E_ICONST,        /* inconsistent parsing (invalid fields length) */
  E_INVAL,         /* invalid format */
  E_LARGE         /* extra data element too large */
};

enum trace_scope {
  SC_UNKNOWN    = 0x1, /* unknown scope for current minor version */
  SC_SIMULATION = 0x2, /* simulation scope */
  SC_NODE       = 0x4  /* node scope */
};

enum event_element_type {
  EV_T_UNKNOWN,        /* Unknown event type. */
  EV_T_MON_CREATE,     /* Initialize the monitor. */
  EV_T_MON_STATE,      /* State event from the monitor. */
  EV_T_MON_DATA,       /* Data event from the monitor. */
  EV_T_NODE_CREATE,    /* Node creation. */
  EV_T_NODE_DESTROY,   /* Node removal. */
  EV_T_NODE_POSITION   /* Node position changes. */
};

enum scope_element_type {
  SC_T_SEPARATOR,  /* Distinguish scopes/event in file format. */
  SC_T_SIMULATION, /* Events that happen within the simulation. */
  SC_T_NODE        /* Events that happen within a specific node. */
};

/* Timestamp in nodes are given as a number of cycles
   and number of milliseconds since boot. */
struct node_time {
  uint64_t cycles;  /* Number of CPU cycles for the node. */
  double   node_ms; /* Milliseconds elapsed since boot of the node. */
};

struct trace {
  /* input stream */
  iofile_t input;

  /* major and minor versions */
  uint32_t major;
  uint32_t minor;
};

/* Information about the scope at which an event occured.
   The scope can be for example the simulation or a node
   within the simulation. The list of all scopes to which
   the event apply can be extraced from the scope bitmask
   using the scope bit flags described above.

   This structure contains the time information for all
   scopes that apply to the event. There is no guarantee
   that unused scope fields are set to zero.

   Note that the current format accepts 32768 scopes but
   this parser only support 32 (because of the bitmask).
   Also technically an event can contain the same scope
   multiple times (for example two nodes scope). This might
   be useful for specifying an event that would apply to
   two nodes at the same time. But this is not possible
   with only the scope structure and not yet supported.
*/
struct scope {
  uint32_t scope; /* Bitmask of the scopes
                     used for this event. */

  /* simulation scope */
  double sim_us; /* Simulation time in microseconds. */

  /* node scope */
  unsigned short nid;          /* node identifier */
  struct node_time node_time;  /* node time */
};

/* Initialize the monitor for a node with offset and byte order. */
struct ev_mon_create {
  struct node_time state_offset;
  struct node_time data_offset;
  struct node_time byte_offset;

  /* Endianness used in the node.
     This is for extra informations that are not handled
     (converted to host endianness) by the parser. */
  unsigned char byte_order; /* 'B' for big endian / 'l' for little endian. */
};

/* State events generated by the monitor inside the firmware. */
struct ev_mon_state {
  unsigned short context;
  unsigned short entity;
  unsigned short state;
};

/* Data events generated by the monitor inside the firmware. */
struct ev_mon_data {
  unsigned short context;
  unsigned short entity;

  unsigned int length;
  void *data;
};

/* Change the position of a node within the simulation. */
struct ev_node_position {
  double x;
  double y;
  double z;
};

/* List of functions for parsing each specific event.
   Note that some events have a pointer called null
   instead of the second event argument. This is
   because this type of event contains no data, hence
   it does have any related data structure. */
struct parsed_by_events {
  int (*unknown)(const struct scope *scope, const void *null, void *data);
  int (*mon_create)(const struct scope *scope, const struct ev_mon_create *event, void *data);
  int (*mon_state)(const struct scope *scope, const struct ev_mon_state *event, void *data);
  int (*mon_data)(const struct scope *scope, const struct ev_mon_data *event, void *data);
  int (*node_create)(const struct scope *scope, const void *null, void *data);
  int (*node_destroy)(const struct scope *scope, const void *null, void *data);
  int (*node_position)(const struct scope *scope, const struct ev_node_position *event, void *data);
};

/* Open the trace from a previously opened file descriptor.
   The passed structure is initialized accordingly with the
   major/minor version and trace information. */
int trace_open(int fd, struct trace *trace);

/* Close the trace. Note that the trace should be closed
   even in the case of an error in order to free the parser's
   internal buffers. */
int trace_close(const struct trace *trace);

/* Start the parsing of the trace. Each event is passed to
   the parsed() function in parameters. This function takes
   a summary of the scope elements, the type of the event,
   and a structure representing the event (passed as a
   generic pointer). It is possible to pass an extra data
   pointer that is passed along each parsed event.

   Note that the event structure that is passed is not kept
   between events except for extra data that are allocated
   individually (and must be freed manually).

   The parsing cancels as soon as the parsed function
   returns a negative value, the error is then reported. */
int trace_parse(const struct trace *trace,
                int (*parsed)(const struct scope *scope,
                              enum event_element_type type,
                              const void *event,
                              void *data),
                void *data);

/* This is an alternative to the parsing function above.
   Instead of a single parsing callback that should switch
   on the event type, it uses a structure that provides a
   callback for each type of event.

   Using this approach it is harder to leave newly added
   events unprocessed by other parts of the program as the
   parser and file format evolve.

   The function also asserts that all callbacks are
   initialized properly. To do so it checks for any null
   pointer within the callbacks structure. Therefore it is
   recommended that the structure is initialized to zero.

   The parsing cancels as soon as any of the parsed function
   returns a negative value, the error is then reported. */
int trace_parse_by_events(const struct trace *trace,
                          const struct parsed_by_events *parsed_by_events,
                          void *data);

/* Return the error message associated to a parsing error. */
const char * trace_parse_error(enum trace_error error);

#endif /* _PARSER_H_ */
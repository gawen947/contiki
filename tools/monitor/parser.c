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
#include <assert.h>

#include "sys-endian.h"
#include "iobuf.h"
#include "parser.h"

#include "dump.h"

/* Size of the buffer used to parse event elements. */
#define EVENT_BUFFER_SIZE 256

/* Maximum size for extra data appended to parsed elements
   because it wouldn't fit the EVENT_BUFFER_SIZE (for
   example with MON_DATA events).

   The memory needed is allocated by the parser and must
   be freed manually by the parser caller.

   If the extra data is larger than the value below, an
   error (E_LARGE) is generated. */
#define MAX_EXTRA_DATA 268435456 /* 256 MB */

/* The EOF can occur only at specific points
   during the parsing. In other cases, it means
   that the file failed too short and it should
   be reported as an error.

   This macro executes the expression and returns
   E_SHORT when EOF is reached. */
#define ERR_ON_EOF(exp)  \
  {                      \
    int r = (exp);       \
    if(r < 0) {          \
      if(r == E_EOF)     \
        r = E_SHORT;     \
      return r;          \
    }                    \
  }

/* Contrary to the preceding macro, here we
   just return EOF as is. In other words,
   simply return the error message as it is. */
#define RET(exp)         \
  {                      \
    int r = (exp);       \
    if(r < 0)            \
      return r;          \
  }

static int xiobuf_read(iofile_t file, void *buf, size_t count)
{
  ssize_t ret = iobuf_read(file, buf, count);

  if(ret < 0)
    return E_IOERR; /* FIXME: catch EINTR should restart the call */
  else if(ret == 0)
    return E_EOF;
  else if((size_t)ret == count)
    return 0;
  else /* > 0 */
    return E_SHORT;
}

static int xiobuf_lseek(iofile_t file, off_t offset) {
  off_t ret = iobuf_lseek(file, offset, SEEK_CUR);

  if(ret < 0)
    return E_IOERR; /* FIXME: catch EINTR should restart the call */
  else
    return 0;
}

int trace_open(int fd, struct trace *trace)
{
  const uint32_t magik[2] = { 0x63746b74, 0x72616365 }; /* magik numbers */

  int n;
  uint32_t header[4];

  /* Open a buffered file from the file descriptor. */
  trace->input = iobuf_dopen(fd);
  if(!trace->input)
    return E_NOMEM;

  /* We only care about the header here.
     But it must not fall short. */
  ERR_ON_EOF(xiobuf_read(trace->input, header, sizeof(header)));

  /* Convert to host endianness. */
  for(n = 3 ; n >= 0 ; n--)
    header[n] = be32toh(header[n]);

  /* Check magik numbers. */
  for(n = 1 ; n >= 0 ; n--) {
    if(header[n] != magik[n])
      return E_MAGIK;
  }

  /* Check major/minor version. */
  trace->major = header[2];
  trace->minor = header[3];
  if(trace->major != PARSER_MAJOR_VERSION)
    return E_MAJOR;

  return 0; /* success */
}

int trace_close(const struct trace *trace)
{
  if(iobuf_close(trace->input) < 0)
    return E_IOERR; /* FIXME: catch EINTR */
  return 0;
}

static int parse_node_time(const struct trace *trace, struct node_time *time)
{
  RET(xiobuf_read(trace->input, &time->cycles, sizeof(uint64_t)));
  RET(xiobuf_read(trace->input, &time->node_ms, sizeof(double)));

  time->cycles  = be64toh(time->cycles);
  time->node_ms = betohD(time->node_ms);

  return 0;
}

static int parse_element_hdr(const struct trace *trace, unsigned short *type, unsigned long *len)
{
  unsigned int len_field;
  uint16_t type_and_len;

  /* Since the len is either 8 or 32bits,
     we ensure that it is initialized
     properly in both cases */
  *len = 0;

  /* Parse the type_and_len (control) field. */
  RET(xiobuf_read(trace->input, &type_and_len, sizeof(type_and_len)));

  if(type_and_len & 1)
    len_field = sizeof(uint32_t);
  else
    len_field = sizeof(uint8_t);
  *type = be16toh(type_and_len >> 1);

  /* The separator has type 0 and no length. */
  if(*type == SC_T_SEPARATOR)
    return 0;

  /* Parse the len */
  ERR_ON_EOF(xiobuf_read(trace->input, len, len_field));

  /* Convert endianness when needed. */
  if(type_and_len & 1)
    *len = be32toh(*len);

  return 0;
}

static int parse_scope(const struct trace *trace, struct scope *scope)
{
  unsigned short type;
  unsigned long  len;

  RET(parse_element_hdr(trace, &type, &len));

  switch(type) {
  case SC_T_SEPARATOR:
    return 1; /* signal separation */
  case SC_T_SIMULATION:
    if(len != sizeof(uint64_t))
      return E_ICONST;

    scope->scope |= SC_SIMULATION;

    ERR_ON_EOF(xiobuf_read(trace->input, &scope->sim_us, sizeof(uint64_t)));

    scope->sim_us = be64toh(scope->sim_us);
    break;
  case SC_T_NODE:
    if(len != (sizeof(uint64_t) + sizeof(double) + sizeof(uint16_t)))
      return E_ICONST;

    scope->scope |= SC_NODE;

    ERR_ON_EOF(parse_node_time(trace, &scope->node_time));
    ERR_ON_EOF(xiobuf_read(trace->input, &scope->nid, sizeof(uint16_t)));

    scope->nid = be16toh(scope->nid);
    break;
  default:
    scope->scope |= SC_UNKNOWN;

    RET(xiobuf_lseek(trace->input, len));
    break;
  }

  return 0; /* continue */
}

static int parse_event_mon_create(const struct trace *trace, char event_buffer[])
{
  struct ev_mon_create *event = (struct ev_mon_create *)event_buffer;

  RET(parse_node_time(trace, &event->state_offset));
  RET(parse_node_time(trace, &event->data_offset));
  RET(parse_node_time(trace, &event->byte_offset));

  RET(xiobuf_read(trace->input, &event->byte_order, sizeof(event->byte_order)));

  return 0;
}

static int parse_event_mon_state(const struct trace *trace, char event_buffer[])
{
  struct ev_mon_state *event = (struct ev_mon_state *)event_buffer;

  RET(xiobuf_read(trace->input, &event->context, sizeof(event->context)));
  RET(xiobuf_read(trace->input, &event->entity, sizeof(event->entity)));
  RET(xiobuf_read(trace->input, &event->state, sizeof(event->state)));

  event->context = be16toh(event->context);
  event->entity  = be16toh(event->entity);
  event->state   = be16toh(event->state);

  return 0;
}

static int parse_event_mon_data(const struct trace *trace, char event_buffer[], unsigned int len)
{
  struct ev_mon_data *event = (struct ev_mon_data *)event_buffer;

  RET(xiobuf_read(trace->input, &event->context, sizeof(event->context)));
  RET(xiobuf_read(trace->input, &event->entity, sizeof(event->entity)));

  event->context = be16toh(event->context);
  event->entity  = be16toh(event->entity);

  /* The actual length of extra data in the buffer. We check
     that it isn't too large and that we can allocate memory
     for it. */
  event->length = len - sizeof(event->context) - sizeof(event->entity);

  if(event->length > MAX_EXTRA_DATA)
    return E_LARGE;

  if(!(event->data = malloc(event->length)))
    return E_NOMEM;

  RET(xiobuf_read(trace->input, event->data, event->length));

  return 0;
}

static int parse_event_node_position(const struct trace *trace, char event_buffer[])
{
  struct ev_node_position *event = (struct ev_node_position *)event_buffer;

  RET(xiobuf_read(trace->input, &event->x, sizeof(event->x)));
  RET(xiobuf_read(trace->input, &event->y, sizeof(event->y)));
  RET(xiobuf_read(trace->input, &event->z, sizeof(event->z)));

  event->x = betohD(event->x);
  event->y = betohD(event->y);
  event->z = betohD(event->z);

  return 0;
}

static const void * parse_event(const struct trace *trace, enum event_element_type *type, enum trace_error *error)
{
  /* Events are created inside a buffer
     that should be large enough for
     the majorty of events.

     Larger events should be allocated or
     transmitted directly. */
  static char event_buffer[EVENT_BUFFER_SIZE];

  unsigned short event_type;
  unsigned long  event_len;

  int ret;

  ret = parse_element_hdr(trace, &event_type, &event_len);
  if(ret < 0) {
    if(ret == E_EOF)
      ret = E_SHORT;
    *error = ret;
    return NULL;
  }

  *type = event_type;

  switch(event_type) {
  case EV_T_MON_CREATE:
    if(event_len != 1 + (sizeof(double) + sizeof(uint64_t)) * 3)
      ret = E_ICONST;
    else
      ret = parse_event_mon_create(trace, event_buffer);
    break;
  case EV_T_MON_STATE:
    if(event_len != sizeof(uint16_t) * 3)
      ret = E_ICONST;
    else
      ret = parse_event_mon_state(trace, event_buffer);
    break;
  case EV_T_MON_DATA:
    if(event_len < sizeof(uint16_t) * 2)
      ret = E_ICONST;
    else
      ret = parse_event_mon_data(trace, event_buffer, event_len);
    break;
  case EV_T_NODE_CREATE:
  case EV_T_NODE_DESTROY:
    if(event_len != 0)
      ret = E_ICONST;
    /* Empty elements. No parsing needed. */
    break;
  case EV_T_NODE_POSITION:
    if(event_len != sizeof(double) * 3)
      ret = E_ICONST;
    ret = parse_event_node_position(trace, event_buffer);
    break;
  default:
    *type = EV_T_UNKNOWN;
    ret = xiobuf_lseek(trace->input, event_len);
    break;
  }

  if(ret < 0) {
    *error = ret;
    return NULL;
  }

  return event_buffer;
}

int trace_parse(const struct trace *trace,
                int (*parsed)(const struct scope *scope,
                              enum event_element_type type,
                              const void *event,
                              void *data),
                void *data)
{
  static struct scope scope;
  enum event_element_type type;
  enum trace_error error;
  const void *event;
  int ret;


  while(1) { /* events loop */
    /* We can hit EOF only on the first scope element
       (that is the start of a new event). Other elements
       must fail with an error when hitting EOF. */
    ret = parse_scope(trace, &scope);
    if(ret < 0) {
      if(ret == E_EOF)
        return 0; /* valid EOF */
      return ret; /* error */
    } else if(ret == 0) {
      /* Continue parsing if there are remaining scope elements.
         This time an EOF is reported as a E_SHORT error. */
      while(!(ret = parse_scope(trace, &scope)));
      if(ret < 0) {
        if(ret == E_EOF)
          ret = E_SHORT;
        return ret;
      }
    }

    /* Parse event and tell the caller. */
    if(!(event = parse_event(trace, &type, &error)))
       return error;

    RET(parsed(&scope, type, event, data));
  }
}

struct by_events_data {
  const struct parsed_by_events *parsed_by_events;
  void *data;
};

static int by_events(const struct scope *scope,
                     enum event_element_type type,
                     const void *event,
                     void *data)
{
  struct by_events_data *by_events_data    = (struct by_events_data *)data;
  const struct parsed_by_events *callbacks = by_events_data->parsed_by_events;
  int (*parsed_for_event)(const struct scope *scope,
                          const void *event,
                          void *data);

  /* Using a parsed_by_events structure, we call the
     correct function matching the event type. */
  switch(type) {
  case EV_T_UNKNOWN:
    parsed_for_event = (void *)callbacks->unknown;
    break;
  case EV_T_MON_CREATE:
    parsed_for_event = (void *)callbacks->mon_create;
    break;
  case EV_T_MON_STATE:
    parsed_for_event = (void *)callbacks->mon_state;
    break;
  case EV_T_MON_DATA:
    parsed_for_event = (void *)callbacks->mon_data;
    break;
  case EV_T_NODE_CREATE:
    parsed_for_event = (void *)callbacks->node_create;
    break;
  case EV_T_NODE_DESTROY:
    parsed_for_event = (void *)callbacks->node_destroy;
    break;
  case EV_T_NODE_POSITION:
    parsed_for_event = (void *)callbacks->node_position;
    break;
  default:
    assert(0); /* unknown event */
  }

  return parsed_for_event(scope, event, by_events_data->data);
}

#ifdef NDEBUG
# define assert_parsed_by_events(...) ((void)0)
#else
static void assert_parsed_by_events(const struct parsed_by_events *parsed_by_events)
{
  /* We check that all pointers of the parsed_by_events
     structure are non null. We try to do this in a generic
     way so that we don't have update to this function each
     time a new event is added to the parser. */
  const void **callback = (const void **)parsed_by_events;
  unsigned int i;

  for(i = 0 ; i < (sizeof(struct parsed_by_events) / sizeof(void *)) ; i++)
    assert(callback[i]);
}
#endif /* NDEBUG */

int trace_parse_by_events(const struct trace *trace,
                          const struct parsed_by_events *parsed_by_events,
                          void *data)
{
  struct by_events_data by_events_data = { parsed_by_events, data };

  /* Check that all callbacks are initialized properly.
     This is a critical error. The program should abort
     immediately if the asertion fails. */
  assert_parsed_by_events(parsed_by_events);

  return trace_parse(trace, by_events, &by_events_data);
}

const char * trace_parse_error(enum trace_error error)
{
  switch(error) {
  case E_SUCCESS:
    return "Success.";
  case E_EOF:
    return "End of file reached.";
  case E_SHORT:
    return "Invalid file format (missing data).";
  case E_IOERR:
    return "IO error.";
  case E_MAGIK:
    return "Invalid magik numbers.";
  case E_MAJOR:
    return "Major version mismatch.";
  case E_NOMEM:
    return "Out of memory.";
  case E_ICONST:
    return "Inconsistent parsing.";
  case E_INVAL:
    return "Invalid file format.";
  case E_LARGE:
    return "Extra data element too large.";
  default:
    return "Unknown error.";
  }
}

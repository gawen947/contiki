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

#ifndef _GRAPH_MODE_H_
#define _GRAPH_MODE_H_

#include "mode.h"

/* Statistics used to construct a label. */
struct label_stats {
  uint64_t sum_delta_cycles;
  uint64_t min_delta_cycles;
  uint64_t max_delta_cycles;

  double sum_delta_node_ms;
  double min_delta_node_ms;
  double max_delta_node_ms;

  uint64_t sum_delta_sim_us;
  uint64_t min_delta_sim_us;
  uint64_t max_delta_sim_us;

  unsigned int count; /* Number of time this transition has been taken. */
};

struct graph_metric {
  const char *name;
  const char *description;

  /* Labelize a transition from one state to another. */
  const char * (*label_transition)(const struct label_stats *stats);
};

struct output_mode graph_mode;

/* Select a graph metric by its name. */
const struct graph_metric * select_graph_metric_by_name(const char *name);

/* List all graph metrics. This is useful for example to display a list of
   all available metrics along with their description. */
void walk_graph_metrics(void (*visit)(const struct graph_metric *metric, void *data), void *data);

#endif /* _GRAPH_MODE_H_ */

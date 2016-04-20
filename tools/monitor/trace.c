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
#include <getopt.h>
#include <string.h>
#include <err.h>

#include "mon-ids.h"
#include "mon-names.h"
#include "print-mode.h"
#include "graph-mode.h"
#include "help.h"
#include "version.h"
#include "mode.h"
#include "trace.h"

#define TARGET "Trace"

/* Init and destroy internal data structures. */
static void init(void)
{
  mon_names_init();
  register_mon_ids();
}

static void destroy(void)
{
  mon_names_destroy();
}

/* Process parse error and abort when needed. */
static void parse_error(int ret)
{
  if(ret < 0)
    errx(EXIT_FAILURE, "Parsing error: %s", trace_parse_error(ret));
}

static int parse(const struct context *ctx,
                 const char *trace_path,
                 const struct output_mode *mode)
{
  struct trace trace;
  int ret, fd;

  fd = open(trace_path, O_RDONLY);
  if(fd < 1)
    err(EXIT_FAILURE, "cannot open trace");

  /* initialize the output module */
  mode->before(ctx);

  /* parse the trace */
  ret = trace_open(fd, &trace);
  parse_error(ret);

  ret = trace_parse_by_events(&trace, &mode->parsed_by_events, (void *)ctx);
  parse_error(ret);

  ret = trace_close(&trace);
  parse_error(ret);

  /* destroy the output module
     (and show results if needed) */
  mode->after(ctx);

  return EXIT_SUCCESS;
}

static void display_mode(const struct output_mode *mode, void *data)
{
  printf("%s\t%s\n", mode->name, mode->description);
}

/* FIXME: Display_(metric|mode) are basically the same.
          We should use only one function. */
static void display_metric(const struct graph_metric *metric, void *data)
{
  printf("%s\t%s\n", metric->name, metric->description);
}

int main(int argc, char *argv[])
{
  const char *name;
  const struct output_mode *mode = &print_mode;
  struct context ctx = { 0 };

  int exit_status = EXIT_FAILURE;

  name = (const char *)strrchr(argv[0], '/');
  name = name ? (name + 1) : argv[0];

  enum long_only_opt {
    OPT_COMMIT = 0x100,
  };

  struct opt_help helps[] = {
    { 'h', "help", "Show this help message" },
    { 'V', "version", "Print version information" },
#ifdef COMMIT
    { 0, "commit", "Display commit information" },
#endif /* COMMIT */
    { 'H', "human", "Display trace or statistics in an human readable format" },
    { 'i', "no-ids", "Disable resolution of monitor IDs names"},
    { 'o', "output", "Select the output mode (use ? or list to display available modes)" },
    { 'G', "graph-metric", "Metric to use for the graph (use ? or list to display available metrics)" },
    { 0, NULL, NULL }
  };

  struct option long_opts[] = {
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'V' },
#ifdef COMMIT
    { "commit", no_argument, NULL, OPT_COMMIT},
#endif /* COMMIT */
    { "human", no_argument, NULL, 'H' },
    { "no-ids", no_argument, NULL, 'i' },
    { "output", required_argument, NULL, 'o' },
    { "graph-metric", required_argument, NULL, 'G' },
    { NULL, 0, NULL, 0 }
  };

  /* init */
  init();

  while(1) {
    int c = getopt_long(argc, argv, "hVHio:G:", long_opts, NULL);

    if(c == -1)
      break;

    switch(c) {
#ifdef COMMIT
    case OPT_COMMIT:
      commit();
      exit_status = EXIT_SUCCESS;
      goto EXIT;
#endif /* COMMIT */
    case 'o':
      if(!strcmp(optarg, "list") || !strcmp(optarg, "?")) {
        walk_modes(display_mode, NULL);

        exit_status = EXIT_SUCCESS;
        goto EXIT;
      }

      mode = select_mode_by_name(optarg);
      if(!mode) {
        /* mode not found */
        fprintf(stderr, "error: Output mode not found.\n"
                        "       Use list or ? to display available modes\n");
        goto EXIT;
      }
      break;
    case 'G':
      if(!strcmp(optarg, "list") || !strcmp(optarg, "?")) {
        walk_graph_metrics(display_metric, NULL);

        exit_status = EXIT_SUCCESS;
        goto EXIT;
      }

      ctx.graph_metric = select_graph_metric_by_name(optarg);
      if(!ctx.graph_metric) {
        /* graph metric not found */
        fprintf(stderr, "error: Graph metric not found.\n"
                        "       use list or ? to display available metrics\n");
        goto EXIT;
      }
      break;
    case 'H':
      ctx.human = 1;
      break;
    case 'i':
      mon_names_set(0);
      break;
    case 'V':
      version(TARGET);
      exit_status = EXIT_SUCCESS;
      goto EXIT;
    case 'h':
      exit_status = EXIT_SUCCESS;
    default:
      help(name, "[OPTIONS] trace-file", helps);
      goto EXIT;
    }
  }

  if((argc - optind) != 1)
    errx(EXIT_FAILURE, "except trace-file");

  exit_status = parse(&ctx, argv[optind], mode);

EXIT:
  destroy();
  return exit_status;
}

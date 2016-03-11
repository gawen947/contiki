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

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "help.h"
#include "version.h"

#define TARGET "Trace"

int main(int argc, char *argv[])
{
  const char *name;

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
    { 'o', "output", "Select the output mode (use ? or list to display available modes)" },
    { 'p', "period", "Only display events within the specified period" },
    { 0, NULL, NULL }
  };

  struct option long_opts[] = {
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'V' },
#ifdef COMMIT
    { "commit", no_argument, NULL, OPT_COMMIT},
#endif /* COMMIT */
    { "human", no_argument, NULL, 'H' },
    { "output", required_argument, NULL, 'o' },
    { "period", required_argument, NULL, 'p' },
    { NULL, 0, NULL, 0 }
  };

  while(1) {
    int c = getopt_long(argc, argv, "hVHo:p:", long_opts, NULL);

    if(c == -1)
      break;

    switch(c) {
#ifdef COMMIT
    case OPT_COMMIT:
      commit();
      exit_status = EXIT_SUCCESS;
      goto EXIT;
#endif /* COMMIT */
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

EXIT:
  return exit_status;
}

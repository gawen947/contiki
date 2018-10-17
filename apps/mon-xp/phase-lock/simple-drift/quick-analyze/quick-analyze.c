/* Copyright (c) 2018, David Hauweele <david@hauweele.net>
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

/* quick string analyze for drift XP
   do not even think about using this for anything else */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <err.h>

#include <gawen/iobuf.h>
#include <gawen/string.h>

#ifdef __linux__
/* memmem in Linux requires _GNU_SOURCE
   but it also declares basename() with
   a GNU specific prototype which conflict
   with libgawen. */
void *memmem(const void *haystack, size_t haystacklen,
             const void *needle, size_t needlelen);
#endif


#define BUF_SIZE (4096)

#define SUFFIX      "ENT=TEST STATE=MCLK"
#define SUFFIX_LEN  (strlen(SUFFIX))
#define SUFFIX_LAST (SUFFIX + SUFFIX_LEN)

#define SIM_FIELD "SIM_TIME_US"
#define CYC_FIELD "NODE_CYCLES"

int main(int argc, char *argv[])
{
  iofile_t in, out;
  char *s, *t, *prefix;
  unsigned int prefix_len;
  int n;

  if(argc != 2)
    errx(EXIT_FAILURE, "require prefix string");
  prefix     = argv[1];
  prefix_len = strlen(argv[1]);

  in  = iobuf_dopen(STDIN_FILENO);
  out = iobuf_dopen(STDOUT_FILENO);
  if(!out || !in)
    err(EXIT_FAILURE, "iobuf_dopen()");

  while(1) {
    char buf[BUF_SIZE];

    n = iobuf_gets(in, buf, BUF_SIZE);
    if(n <= 0) {
      if(n < 0)
        err(EXIT_FAILURE, "read()");
      else
        break;
    } else {
      if(buf[n - 1] == '\n')
        buf[--n] = '\0';
    }

    /* check suffix */
    if(memrevcmp(buf + n, SUFFIX_LAST, SUFFIX_LEN))
      continue;

    /* In the code below, we don't check for null pointers,
       it will segfault but we don't care, we assume
       that data is correctly formated if the check above
       is verified. */

    /* sim field */
    s  = memmem(buf, n, SIM_FIELD, strlen(SIM_FIELD));
    s += strlen(SIM_FIELD) + 1;
    t  = s;
    while(isdigit(*s++));
    iobuf_write(out, prefix, prefix_len);
    iobuf_write(out, t, s - t);

    n = buf + n - s;

    /* cyc fields */
    s  = memmem(s, n, CYC_FIELD, strlen(CYC_FIELD));
    s += strlen(CYC_FIELD);
    t  = s;
    while(isdigit(*++s));
    *s = '\n';
    iobuf_write(out, t + 1, s - t);
  }

  iobuf_close(out);

  return 0;
}

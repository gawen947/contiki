/* Copyright (c) 2013-2016, David Hauweele <david@hauweele.net>
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
#include <ctype.h>

#include "dump.h"

/* display configuration */
#define OFFSET_FMT  "$%04x:"
#define BYTE_FMT    "%02x"
#define PADDING_FMT "__"
#define WORD_SZ     8

#define DWORD_SZ    WORD_SZ << 1

void hex_dump(const unsigned char *data, int size)
{
  int i;
  unsigned int offset = 0;

  for(; size >= DWORD_SZ ; size -= DWORD_SZ) {
    printf(OFFSET_FMT " ", offset);

    for(i = 0 ; i < WORD_SZ ; i++, data++)
      printf(BYTE_FMT " ", *data);
    putchar(' ');
    for(i = 0 ; i < WORD_SZ ; i++, data++)
      printf(BYTE_FMT " ", *data);

    data -= DWORD_SZ;
    putchar('|');
    for(i = 0 ; i < DWORD_SZ ; i++, data++) {
      if(isprint(*data))
        putchar(*data);
      else
        putchar('.');
    }
    printf("|\n");

    offset += DWORD_SZ;
  }

  if(size <= 0)
    return;

  printf(OFFSET_FMT " ", offset);

  for(i = 0 ; i < WORD_SZ ; i++, data++, size--) {
    if(size > 0)
      printf(BYTE_FMT " ", *data);
    else
      printf(PADDING_FMT " ");
  }

  putchar(' ');

  for(i = 0 ; i < WORD_SZ ; i++, data++, size--) {
    if(size > 0)
      printf(BYTE_FMT " ", *data);
    else
      printf(PADDING_FMT " ");
  }

  data -= DWORD_SZ;
  size += DWORD_SZ;

  putchar('|');
  for(i = 0 ; i < DWORD_SZ ; i++, data++, size--) {
    if(size <= 0) {
      putchar('.');
      continue;
    }

    if(isprint(*data))
      putchar(*data);
    else
      putchar('.');
  }
  printf("|\n");
}

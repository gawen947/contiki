#  Copyright (c) 2012-2016, David Hauweele <david@hauweele.net>
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright notice, this
#      list of conditions and the following disclaimer.
#   2. Redistributions in binary form must reproduce the above copyright notice,
#      this list of conditions and the following disclaimer in the documentation
#      and/or other materials provided with the distribution.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
#  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import sys
import signal
import os

# Global options, we try to avoid them when possible.
__debug = False

def set_debug(status):
    global __debug;
    __debug = status

def stderr(msg, prefix = None):
    """
    Display a message to stderr.
    This message is prefixed by the program's name and optionally
    a message prefix such as error, warning, debug, ...
    """

    output = sys.argv[0] + ": "

    # We force the string representation here since
    # we may want to display a representable object
    # to stderr.
    if prefix:
        output = str(prefix) + ": "
    output += str(msg) + "\n"

    sys.stderr.write(output)

def err(exit_code, msg):
    """Display an error message and exit."""

    stderr(msg, "error")
    os.killpg(os.getpgrp(), signal.SIGTERM)
    sys.exit(exit_code)

def verbose(msg):
    """Display a debug message when needed."""
    if __debug:
        stderr(msg, "debug")

# Display warning/information messages
warn  = lambda m : stderr(m, "warning")
info  = lambda m : stderr(m, "info")

def line_to_file(f, string):
    f.write(string + '\n')

def try_float(s):
    """Try to convert a string to float."""

    try:
        f = float(s)
    except ValueError:
        err(1, "cannot convert '%s' to float" % (s,))
    return f

def try_int(s):
    """Try to convert a string to int."""

    try:
        f = int(s)
    except ValueError:
        err(1, "cannot convert '%s' to int" % (s,))
    return f


def try_open(filename, mode):
    try:
        f = open(filename, mode)
    except IOError:
        err(1, "cannot open \"%s\"" % (filename,))
    return f

def incoherent_fields_error(line, fields, file):
    err_mess = "incoherent number of fields"
    verbose("%s (%d) in \"%s\"" % (err_mess, len(fields), file))
    verbose("incriminated line :")
    verbose(" \"%s\"" % (line,))
    err(1, err_mess)

def replace_characters(string, characters, substitute):
    """
    Replace in 'string' any occurence of any character found
    in 'characters' and replace each occurence with 'substitute'.
    """

    return ''.join(map(lambda c : substitute if c in characters else c, string))

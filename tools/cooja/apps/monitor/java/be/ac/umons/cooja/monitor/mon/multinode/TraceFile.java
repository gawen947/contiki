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

package be.ac.umons.cooja.monitor.mon.multinode;

import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteOrder;

import se.sics.mspsim.util.Utils;

/**
 * A trace records all events within a simulation consisting of multiple nodes.
 */
public class TraceFile {
  /* 'ctktrace' */
  public static final int MAGIK1 = 0x63746b74; /* 'ctkt' */
  public static final int MAGIK2 = 0x72616365; /* 'race' */ 
  
  /* We have two versions for the file, MAJOR and MINOR.
   * The file structure is incompatible between MAJOR versions,
   * but forward compatible between MINOR versions. */
  
  /* Increment when new feature or changes hinder
   * parsing (i.e. it is not possible to parse new
   * traces with previous version of the parser). */ 
  public static final int MAJOR_VERSION = 1;
  
  /* Increment when new features or changes do not
   * hinder parsing (i.e. the previous version of
   * the  parser can skip new features and changes). */
  public static final int MINOR_VERSION  = 0;
  
  /* The endianness used for all fields. */
  public static final ByteOrder ENDIAN = ByteOrder.BIG_ENDIAN;
  
  
  private final OutputStream out;
  
  public TraceFile(String filePath) throws IOException {
    out = new BufferedOutputStream(new FileOutputStream(filePath));

    writeMagik();
    writeVersion();
  }
  
  public void write(Event event) throws IOException {
    event.serialize(out);
  }
  
  private void writeMagik() throws IOException {
    out.write(Utils.toBytes(MAGIK1, ByteOrder.BIG_ENDIAN));
    out.write(Utils.toBytes(MAGIK2, ByteOrder.BIG_ENDIAN));
  }
  
  private void writeVersion() throws IOException {
    out.write(Utils.toBytes(MAJOR_VERSION, ByteOrder.BIG_ENDIAN));
    out.write(Utils.toBytes(MINOR_VERSION, ByteOrder.BIG_ENDIAN));
  }
  
  public void destroy() throws IOException {
    out.close();
  }
}

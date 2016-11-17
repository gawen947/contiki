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

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteOrder;

import be.ac.umons.cooja.monitor.mon.MonTimestamp;
import se.sics.mspsim.util.Utils;

/**
 * Initialization of the monitor within a node with offset and byte order.
 */
public class MonCreateEvent implements EventElement {
  MonTimestamp stateOffset;
  MonTimestamp dataOffset;
  MonTimestamp byteOffset;
  ByteOrder byteOrder;
  
  public MonCreateEvent(MonTimestamp stateOffset, MonTimestamp dataOffset, MonTimestamp byteOffset, ByteOrder byteOrder) {
    this.stateOffset = stateOffset;
    this.dataOffset  = dataOffset;
    this.byteOffset  = byteOffset;
    this.byteOrder   = byteOrder;  
  }
  
  @Override
  public void serialize(OutputStream out) throws IOException {
    Utils.writeBytes(out, stateOffset, TraceFile.ENDIAN);
    Utils.writeBytes(out, dataOffset, TraceFile.ENDIAN);
    Utils.writeBytes(out, byteOffset, TraceFile.ENDIAN);
    
    if(byteOrder == ByteOrder.BIG_ENDIAN)
      out.write((byte)'B'); /* BE */
    else
      out.write((byte)'l'); /* le */
  }

  @Override
  public EventElementType getType() {
    return EventElementType.MON_CREATE;
  }

  @Override
  public int getLength() {
    return (MonTimestamp.SIZE * 3 + Byte.SIZE) >> 3;
  }
}

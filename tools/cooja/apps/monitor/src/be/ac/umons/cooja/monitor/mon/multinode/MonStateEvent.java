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

import be.ac.umons.cooja.monitor.Utils;

/**
 * State event generated by the monitor inside the firmware.
 */
public class MonStateEvent implements EventElement {
  private final int context;
  private final int entity;
  private final int state;
  
  public MonStateEvent(int context, int entity, int state) {
    this.context = context;
    this.entity  = entity;
    this.state   = state;
  }
  
  @Override
  public void serialize(OutputStream out) throws IOException {
    Utils.writeBytes(out, (short)context, TraceFile.ENDIAN);
    Utils.writeBytes(out, (short)entity, TraceFile.ENDIAN);
    Utils.writeBytes(out, (short)state, TraceFile.ENDIAN);
  }

  @Override
  public EventElementType getType() {
    return EventElementType.MON_STATE;
  }

  @Override
  public int getLength() {
    return (Short.SIZE * 3) >> 3;
  }
}
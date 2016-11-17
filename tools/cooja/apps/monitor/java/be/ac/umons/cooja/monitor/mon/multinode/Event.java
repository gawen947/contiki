/* Copyright (c) 2015, David Hauweele <david@hauweele.net>
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
import java.util.ArrayList;

import be.ac.umons.cooja.monitor.Utils;

/**
 * All events are created through this class.
 * An event is split into two parts. 
 * 
 * 1) A series of scopes that defines the time/location of the event within a certain context (node/simulation).
 * 2) The event itself.
 * 
 * When you create an event, you first give the event itself to the constructor.
 * Then you add multiple scopes to the event.
 * 
 * After that you pass the event to the trace file instance for writing.
 */
public class Event implements MultinodeEventElement {
  private final EventElement event;
  private final ArrayList<ScopeElement> scopes = new ArrayList<ScopeElement>();

  public Event(EventElement event) {
    this.event = event;
  }
  
  public void addScope(ScopeElement scope) {
    this.scopes.add(scope);
  }
  
  private void writeElementHeader(OutputStream out, short code, int len) throws IOException {
    if(len > 255) {
      Utils.writeBytes(out, (short)((code << 1) | 1), TraceFile.ENDIAN);
      Utils.writeBytes(out, (int)len, TraceFile.ENDIAN);
    } else {
      Utils.writeBytes(out, (short)(code << 1), TraceFile.ENDIAN);
      Utils.writeBytes(out, (byte)len, TraceFile.ENDIAN);
    }
  }
  
  @Override
  public void serialize(OutputStream out) throws IOException {
    /* write scope elements */
    for(ScopeElement scope : scopes) {
      writeElementHeader(out, scope.getType().code, scope.getLength());
      scope.serialize(out);
    }
    
    /* scopes/event separator */
    Utils.writeBytes(out, (short)ScopeElementType.SEPARATOR.code, TraceFile.ENDIAN);
    
    /* write event */
    writeElementHeader(out, event.getType().code, event.getLength());
    event.serialize(out);
  }
}

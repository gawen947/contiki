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

package be.ac.umons.cooja.monitor.mon.backend;

import java.util.Hashtable;
import java.util.Map;
import java.nio.ByteOrder;

import be.ac.umons.cooja.monitor.Utils;
import be.ac.umons.cooja.monitor.mon.MonError;
import be.ac.umons.cooja.monitor.mon.MonTimestamp;

/**
 * Specify where and how the events should be recorded.
 * This works for multiple motes.
 */
public abstract class MonBackend {
  private Map<Short,MonMoteBackend> motes = new Hashtable<Short,MonMoteBackend>();

  private boolean oneMoteInitiated = false;

  /* Used for comparison between all motes.
     For now all motes must share the same
     offsets value and endianess. This is
     because the functions do not take the
     nodeId in argument. So the value has
     to hold for all motes.

     In this case only the first initiated
     mote counts. */
  private ByteOrder    byteOrder;

  private MonTimestamp recordOffset;
  private MonTimestamp infoOffset;
  private MonTimestamp byteOffset;

  protected MonTimestamp getRecordOffset() {
    /* these are errors instead of exception because
       the backend must not access them if the protocol
       has not been initialized properly. */
    if(!oneMoteInitiated)
      throw new MonError("protocol not initiated");
    return recordOffset;
  }

  /** Return the duration of an info message with a zero bytes buffer. */
  protected MonTimestamp getInfoOffset() {
    if(!oneMoteInitiated)
      throw new MonError("protocol not initiated");
    return infoOffset;
  }

  /** Return the processing duration of one byte in the info message buffer. */
  protected MonTimestamp getByteOffset() {
    if(!oneMoteInitiated)
      throw new MonError("protocol not initiated");
    return byteOffset;
  }

  /** Get the byte order in which messages are given to the backend.
      Since the backend does not try to understand info messages, it
      should only store the endianness. It is up to the extracting
      tool to display data in the correct endiannes. */
  protected ByteOrder getEndian() {
    if(!oneMoteInitiated)
      throw new MonError("protocol not initiated");
    return byteOrder;
  }

  /** Return true if the monitor has been initiated. */
  protected boolean isInitiated() {
    return oneMoteInitiated;
  }

  private void checkMoteState(MonMoteBackend mote) {
    if(oneMoteInitiated)
      return;

    if(mote.isInitiated()) {
      this.recordOffset = mote.getRecordOffset();
      this.infoOffset   = mote.getInfoOffset();
      this.byteOffset   = mote.getByteOffset();
      this.byteOrder    = mote.getEndian();
      this.oneMoteInitiated    = true;
      initiated();
    }
  }

  public void state(int context, int entity, int state, MonTimestamp timestamp, long simTime, short nodeID) {
    MonMoteBackend mote = motes.get(nodeID);

    if(mote == null) {
      mote = new MonMoteBackend(this);
      motes.put(nodeID, mote);
    }

    mote.state(context, entity, state, timestamp, simTime, nodeID);
    checkMoteState(mote);
  }

  public void info(int context, int entity, byte[] info, MonTimestamp timestamp, long simTime, short nodeID) {
    MonMoteBackend mote = motes.get(nodeID);

    if(mote == null) {
      mote = new MonMoteBackend(this);
      motes.put(nodeID, mote);
    }

    mote.info(context, entity, info, timestamp, simTime, nodeID);
    checkMoteState(mote);
  }

  /** Record an event into the backend. */
  protected abstract void recordState(int context, int entity, int state,
                                      MonTimestamp timestamp, long simTime, short nodeID);

  /** Record information about an entity into the backend. */
  protected abstract void recordInfo(int context, int entity, byte[] info,
                                     MonTimestamp timestamp, long simTime, short nodeID);

  /** Signal that the monitor protocol has been initiated.
      Offsets and endianness should be accessible. */
  protected abstract void initiated();

  private void error(String message) {
    System.out.printf("(mon) error: %s\n", message);
  }

  public abstract void close();
}

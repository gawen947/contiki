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

import java.nio.ByteOrder;

import be.ac.umons.cooja.monitor.Utils;
import be.ac.umons.cooja.monitor.mon.MonError;
import be.ac.umons.cooja.monitor.mon.MonTimestamp;

/**
 * Specify where and how the events should be recorded.
 * This is for a single mote to handle initialization
 * of the mote's monitor device.
 */
public class MonMoteBackend {
  private final MonBackend backend;

  /* The monitor has to be initialized with
     the source endianness and the duration
     of monitor functions (record/info). */
  private enum MonInitState {
    ENDIAN,          /* check up byte order */
    RECORD_OFFSET,   /* record duration */
    INFO_FETCH,      /* fetch info pointer */
    INFO_U8_OFFSET,  /* info with one byte buffer */
    INFO_U16_OFFSET, /* info with two bytes buffer */
    INITIATED,       /* monitor initiated */
    DISABLED         /* error occured during initialisation */
  }

  /* MON_CT_CONTROL and MON_ENT_CAL values of zero
     ensure that there are always the same in any
     endianness, so the endianness messages is always
     understood. */
  private static final int MON_CT_CONTROL = 0;
  private static final int MON_ENT_CAL    = 0;
  private static final int MON_ST_CHECK   = 0xaabb;

  private MonInitState initState = MonInitState.ENDIAN;
  private ByteOrder    byteOrder;
  private MonTimestamp last;

  private MonTimestamp recordOffset;
  private MonTimestamp infoOffsetU8;
  private MonTimestamp infoOffset;
  private MonTimestamp byteOffset;

  public MonMoteBackend(MonBackend backend) {
    this.backend = backend;
  }

  /** Return the duration of a record (state) message. */
  public MonTimestamp getRecordOffset() {
    /* these are errors instead of exception because
       the backend must not access them if the protocol
       has not been initialized properly. */
    if(initState != MonInitState.INITIATED)
      throw new MonError("protocol not initiated");
    return recordOffset;
  }

  /** Return the duration of an info message with a zero bytes buffer. */
  public MonTimestamp getInfoOffset() {
    if(initState != MonInitState.INITIATED)
      throw new MonError("protocol not initiated");
    return infoOffset;
  }

  /** Return the processing duration of one byte in the info message buffer. */
  public MonTimestamp getByteOffset() {
    if(initState != MonInitState.INITIATED)
      throw new MonError("protocol not initiated");
    return byteOffset;
  }

  /** Get the byte order in which messages are given to the backend.
      Since the backend does not try to understand info messages, it
      should only store the endianness. It is up to the extracting
      tool to display data in the correct endiannes. */
  public ByteOrder getEndian() {
    if(initState != MonInitState.INITIATED)
      throw new MonError("protocol not initiated");
    return byteOrder;
  }

  /** Return true if the monitor has been initiated. */
  public boolean isInitiated() {
    return initState == MonInitState.INITIATED;
  }

  public void state(int context, int entity, int state, MonTimestamp timestamp, long simTime, short nodeID) {
    switch(initState) {
    case ENDIAN:
      /* If the source was already in network order,
         we don't need to change anything. Unlikely
         though because we know its an MSP430. */
      if(state == Utils.htons(MonMoteBackend.MON_ST_CHECK))
        byteOrder = ByteOrder.BIG_ENDIAN;
      else
        byteOrder = ByteOrder.LITTLE_ENDIAN;

      last = timestamp;

      initState  = MonInitState.RECORD_OFFSET;
      break;
    case RECORD_OFFSET:
      /* Messages are sent consecutively. Since we
         recorded the time of the last message, we
         can now compute the duration of one record
         message. */
      recordOffset = last.diff(timestamp);

      initState = MonInitState.INFO_FETCH;
      break;
    case INFO_FETCH:
    case INFO_U8_OFFSET:
    case INFO_U16_OFFSET:
      /* we should not get here, this is an error */
      error("unexpected state");
      initState = MonInitState.DISABLED;
      break;
    case INITIATED:
      /* The protocol has been initiated so we
         just transmit the message to the backend
         implementation. */
      backend.recordState(context, entity, state, timestamp, simTime, nodeID);
      break;
    case DISABLED:
      /* an error occured */
      return;
    }
  }

  public void info(int context, int entity, byte[] info, MonTimestamp timestamp, long simTime, short nodeID) {
    switch(initState) {
    case ENDIAN:
    case RECORD_OFFSET:
      /* we should not get here, this is an error */
      error("unexpected state");
      initState = MonInitState.DISABLED;
      break;
    case INFO_FETCH:
      last = timestamp;

      initState = MonInitState.INFO_U8_OFFSET;
      break;
    case INFO_U8_OFFSET:
      /* We received an info message with a one
         byte info buffer. We can compute its
         duration. */
      infoOffsetU8 = last.diff(timestamp);
      last         = timestamp;

      initState = MonInitState.INFO_U16_OFFSET;
      break;
    case INFO_U16_OFFSET:
      /* Now we received a buffer with a different
         length, so we can compute the offset per
         byte and compute the real info offset. */
      MonTimestamp infoOffsetU16 = last.diff(timestamp);
      byteOffset = infoOffsetU16.diff(infoOffsetU8);
      infoOffset = infoOffsetU8.diff(byteOffset);

      /* free ref */
      last         = null;
      infoOffsetU8 = null;

      initState = MonInitState.INITIATED;
      break;
    case INITIATED:
      /* The protocol has been initiated so we
         just transmit the message to the backend
         implementation. */
      backend.recordInfo(context, entity, info, timestamp, simTime, nodeID);
      break;
    case DISABLED:
      /* an error occured */
      return;
    }
  }

  private void error(String message) {
    System.out.printf("(mon) error: %s\n", message);
  }
}

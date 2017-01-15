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

import org.apache.log4j.Logger;

import be.ac.umons.cooja.monitor.mon.MonTimestamp;
import be.ac.umons.cooja.monitor.Utils;

/**
 * Display events on stdout.
 */
public class StdMon extends MonBackend {
  private static Logger logger = Logger.getLogger(StdMon.class);

  public void initiated() {
    MonTimestamp recordOffset = getRecordOffset();
    MonTimestamp infoOffset   = getInfoOffset();
    MonTimestamp byteOffset   = getByteOffset();

    logger.info("(mon) initiated!");
    System.out.printf("(mon) endianness: %s\n",
                      getEndian() == ByteOrder.LITTLE_ENDIAN ? "LE"
                                                             : "BE");
    System.out.printf("(mon) record offset: %d cycles, %.3fus\n", recordOffset.getCycles(), recordOffset.getMillis() * 1000.);
    System.out.printf("(mon) info offset  : %d cycles, %.3fus\n", infoOffset.getCycles(), infoOffset.getMillis() * 1000.);
    System.out.printf("(mon) byte offset  : %d cycles, %.3fus\n", byteOffset.getCycles(), byteOffset.getMillis() * 1000.);
  }

  public void recordState(int context, int entity, int state,
                          MonTimestamp timestamp, long simTime, short nodeID) {
    /* Since we display directly on stdout we must take care of endianness and offset. */
    context = xtohs(context);
    entity  = xtohs(entity);
    state   = xtohs(state);

    timestamp = reduceRecordOffset(timestamp);

    System.out.printf("(mon) @(node: %d cpu: %d %fms, sim: %fms) RECORD %d %d %d\n",
                      nodeID, timestamp.getCycles(), timestamp.getMillis(), simTime / 1000.,
                      context, entity, state);
  }

  public void recordInfo(int context, int entity, byte[] info,
                         MonTimestamp timestamp, long simTime, short nodeID) {
    /* Since we display directly on stdout we must take care of endianness and offset. */
    context = xtohs(context);
    entity  = xtohs(entity);

    timestamp = reduceInfoOffset(timestamp, info.length);

    System.out.printf("(mon) @(node: %d cpu: %d %fms, sim: %fms) INFO %d %d [",
                      nodeID, timestamp.getCycles(), timestamp.getMillis(), simTime / 1000.,
                      context, entity);

    /* though the info buffer is not converted */
    for(byte b : info)
      System.out.printf("%02x", b);
    System.out.printf("]\n");
  }

  public void close() {
    logger.info("(mon) close!");
  }


  private int xtohs(int value) {
    return Utils.xtohs(value, getEndian());
  }

  private MonTimestamp reduceRecordOffset(MonTimestamp timestamp) {
    return timestamp.reduce(getRecordOffset());
  }

  private MonTimestamp reduceInfoOffset(MonTimestamp timestamp, int bufferLen) {
    timestamp = timestamp.reduce(getInfoOffset());
    return timestamp.reduce(getByteOffset(), bufferLen);
  }
}


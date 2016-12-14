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

package be.ac.umons.cooja.monitor.mon.switchable;

import java.nio.ByteOrder;

import org.apache.log4j.Logger;

import be.ac.umons.cooja.monitor.mon.MonException;
import be.ac.umons.cooja.monitor.mon.MonTimestamp;
import be.ac.umons.cooja.monitor.Utils;

/**
 * Write events on StdOut.
 */
public class StdMonBackend extends SwitchableMonBackend {
  private static Logger logger = Logger.getLogger(StdMonBackend.class);

  /* Use an instance of this class to tell SwitchableMon how to create this backend. */
  static public class Creator implements SwitchableMonBackendCreator {
    @Override
    public SwitchableMonBackend create(MonTimestamp recordOffset, MonTimestamp infoOffset, MonTimestamp byteOffset, ByteOrder byteOrder) throws MonException {
      return new StdMonBackend(recordOffset, infoOffset, byteOffset, byteOrder);
    }
  }

  public StdMonBackend(MonTimestamp recordOffset, MonTimestamp infoOffset, MonTimestamp byteOffset, ByteOrder byteOrder) throws MonException {
    super(recordOffset, infoOffset, byteOffset, byteOrder);

    logger.info("(mon) initiated!");
    System.out.printf("(mon) endianness: %s\n",
                        byteOrder == ByteOrder.LITTLE_ENDIAN ? "LE"
                                                                : "BE");
    System.out.printf("(mon) record offset: %d cycles, %.3fus\n", recordOffset.getCycles(), recordOffset.getMillis() * 1000.);
    System.out.printf("(mon) info offset  : %d cycles, %.3fus\n", infoOffset.getCycles(), infoOffset.getMillis() * 1000.);
    System.out.printf("(mon) byte offset  : %d cycles, %.3fus\n", byteOffset.getCycles(), byteOffset.getMillis() * 1000.);
  }

  @Override
  public void recordState(int context, int entity, int state, MonTimestamp timestamp, double simTime, short nodeID) throws MonException {
    /* Since we display directly on stdout we must take care of endianness and offset. */
    context = xtohs(context);
    entity  = xtohs(entity);
    state   = xtohs(state);

    timestamp = reduceRecordOffset(timestamp);

    System.out.printf("(mon) @(node: %d cpu: %d %fms, sim: %fms) RECORD %d %d %d\n",
                      nodeID, timestamp.getCycles(), timestamp.getMillis(), simTime / 1000.,
                      context, entity, state);
  }

  @Override
  public void recordInfo(int context, int entity, byte[] info, MonTimestamp timestamp, double simTime, short nodeID) throws MonException {
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

  @Override
  public void destroy() throws MonException {
    logger.info("(mon) close!");
  }


  private int xtohs(int value) {
    return Utils.xtohs(value, byteOrder);
  }

  private MonTimestamp reduceRecordOffset(MonTimestamp timestamp) {
    return timestamp.reduce(recordOffset);
  }

  private MonTimestamp reduceInfoOffset(MonTimestamp timestamp, int bufferLen) {
    timestamp = timestamp.reduce(infoOffset);
    return timestamp.reduce(byteOffset, bufferLen);
  }
}

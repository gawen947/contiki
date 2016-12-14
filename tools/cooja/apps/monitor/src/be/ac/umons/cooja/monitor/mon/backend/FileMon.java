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


import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteOrder;

import org.apache.log4j.Logger;

import be.ac.umons.cooja.monitor.mon.MonTimestamp;
import be.ac.umons.cooja.monitor.Utils;

import org.contikios.cooja.Simulation;

/**
 * Store events in a file.
 */
public class FileMon extends MonBackend {
  private static Logger logger = Logger.getLogger(FileMon.class);

  public static final int MAGIK = 0x63746b6d; /* 'ctkm' */

  private OutputStream out = null;
  private String path;

  public FileMon(String path) {
    this.path = path;
  }

  private void disable() {
    out = null;
    logger.info("(mon) log backend disabled!");
  }

  protected void initiated() {
    try {
      out = new BufferedOutputStream(new FileOutputStream(path));
    } catch(IOException e) {
      logger.error("(mon) cannot open '" + path + "' for writing.");
      disable();
    }

    /* Register an output handler to flush the file
     * when the emulator exit. */
    Runtime.getRuntime().addShutdownHook(new ShutdownHandler(this));

    try {
      writeMagik();
      writeControl();
      writeTime(getRecordOffset());
      writeTime(getInfoOffset());
      writeTime(getByteOffset());

      logger.info("(mon) initiated!");
    }
    catch (IOException e) {
      logger.error("(mon) write error!");
      disable();
    }
  }

  private void writeMagik() throws IOException {
    if(out == null)
      return;

    out.write(Utils.toBytes(MAGIK, ByteOrder.BIG_ENDIAN));
  }

  private void writeControl() throws IOException {
    if(out == null)
      return;

    /* control format:
     *   <0> = LITTLE_ENDIAN (1) / BIG_ENDIAN (0)
     */
    int control = 0;

    if(getEndian() == ByteOrder.LITTLE_ENDIAN)
      control |= 1;

    out.write(Utils.toBytes(control, ByteOrder.BIG_ENDIAN));
  }

  private void writeTime(MonTimestamp offset) throws IOException {
    if(out == null)
      return;

    out.write(offset.toBytes(getEndian()));
  }

  public void recordState(int context, int entity, int state,
                          MonTimestamp timestamp, double simTime, short nodeID) {
    try {
      out.write(timestamp.toBytes(getEndian()));
      out.write(Utils.toBytes(simTime, getEndian()));
      out.write(Utils.toBytes(nodeID, getEndian()));
      out.write(Utils.toBytes((short)context, getEndian()));
      out.write(Utils.toBytes((short)entity, getEndian()));
      out.write(Utils.toBytes((short)state, getEndian()));
    } catch (IOException e) {
      logger.error("(mon) write error!");
      disable();
    }
  }

  public void recordInfo(int context, int entity, byte[] info,
                         MonTimestamp timestamp, double simTime, short nodeID) {
    try {
      out.write(timestamp.toBytes(getEndian()));
      out.write(Utils.toBytes(simTime, getEndian()));
      out.write(Utils.toBytes(nodeID, getEndian()));
      out.write(Utils.toBytes((short)context, getEndian()));
      out.write(Utils.toBytes((short)entity, getEndian()));
      out.write(Utils.toBytes((short)0xffff, getEndian())); /* special state to announce info */

      out.write(info);
    } catch (IOException e) {
      logger.error("(mon) write error!");
      disable();
    }
  }

  public void close() {
    try {
      out.close();
      logger.info("(mon) close!");
    } catch (IOException e) {
      logger.error("(mon) close error!");
    }

    disable();
  }

  private static class ShutdownHandler extends Thread {
    private final FileMon mon;

    public ShutdownHandler(FileMon mon) {
      super("FileMon-Shutdown");

      this.mon = mon;
    }

    public void run() {
      mon.close();
    }
  }
}

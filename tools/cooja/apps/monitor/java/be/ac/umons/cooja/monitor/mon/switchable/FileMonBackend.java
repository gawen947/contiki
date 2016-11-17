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

package be.ac.umons.cooja.monitor.mon.switchable;

import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteOrder;

import be.ac.umons.cooja.monitor.mon.MonException;
import be.ac.umons.cooja.monitor.mon.MonTimestamp;
import be.ac.umons.cooja.monitor.mon.backend.FileMon;
import se.sics.mspsim.util.Utils;

/**
 * Record events from into a file (dedicated to a single mote).
 */
public class FileMonBackend extends SwitchableMonBackend {
  /* Use an instance of this class to tell SwitchableMon how to create this backend. */
  static public class Creator implements SwitchableMonBackendCreator {
    private final String filePath;
    
    public Creator(String filePath) {
      this.filePath = filePath;
    }
    
    @Override   
    public SwitchableMonBackend create(MonTimestamp recordOffset, MonTimestamp infoOffset, MonTimestamp byteOffset, ByteOrder byteOrder) throws MonException {
      return new FileMonBackend(recordOffset, infoOffset, byteOffset, byteOrder, filePath);
    }
  }
  
  public static final int MAGIK = FileMon.MAGIK; /* 'ctkm' */
  
  private final OutputStream out;
  private final ByteOrder     endianness;

  public FileMonBackend(MonTimestamp recordOffset, MonTimestamp infoOffset, MonTimestamp byteOffset, ByteOrder byteOrder,
                          String filePath)  throws MonException {
    super(recordOffset, infoOffset, byteOffset, byteOrder);
    
    this.endianness = byteOrder;
    
    try {
      out = new BufferedOutputStream(new FileOutputStream(filePath));
    } catch(IOException e) {
      throw new MonException("cannot open '" + filePath + "' for writing");
    }
    
    try {
      writeMagik();
      writeControl();
      writeTime(recordOffset);
      writeTime(infoOffset);
      writeTime(byteOffset);      
    }
    catch (IOException e) {
      throw new MonException("write error");
    }
    
    System.out.println("(mon) file backend '" + filePath + "' initiated!");
  }
  
  @Override
  public void recordState(int context, int entity, int state, MonTimestamp timestamp) throws MonException {
    try {
      out.write(timestamp.toBytes(endianness));
      out.write(Utils.toBytes((short)context, endianness));
      out.write(Utils.toBytes((short)entity, endianness));
      out.write(Utils.toBytes((short)state, endianness));
    } catch (IOException e) {
      throw new MonException("write error");
    }  
  }

  @Override
  public void recordInfo(int context, int entity, byte[] info,
      MonTimestamp timestamp) throws MonException {
    try {
      out.write(timestamp.toBytes(endianness));
      out.write(Utils.toBytes((short)context, endianness));
      out.write(Utils.toBytes((short)entity, endianness));
      out.write(Utils.toBytes((short)0xffff, endianness)); /* special state to announce info */
      
      out.write(info);
    } catch (IOException e) {
      throw new MonException("write error");
    }
  }

  @Override
  public void destroy() throws MonException {
    try {
      out.close();
      System.out.println("(mon) file backend closes!");
    } catch (IOException e) {
      throw new MonException("close error");
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
    
    if(endianness == ByteOrder.LITTLE_ENDIAN)
      control |= 1;
    
    out.write(Utils.toBytes(control, ByteOrder.BIG_ENDIAN));
  }
  
  private void writeTime(MonTimestamp offset) throws IOException {
    if(out == null)
      return;
    
    out.write(offset.toBytes(endianness));
  }
}

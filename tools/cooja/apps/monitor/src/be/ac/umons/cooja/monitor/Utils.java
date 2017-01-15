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

package be.ac.umons.cooja.monitor;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import be.ac.umons.cooja.monitor.mon.MonTimestamp;

public class Utils {
  /* Useful static for some monitor backends.
   * Note that we cannot use generic here.
   * Or at least I don't know how to do that "easily". */

  /** Convert a integer to an array of byte using a specific endianness. */
  public static byte[] toBytes(int value, ByteOrder byteOrder) {
    ByteBuffer buf = ByteBuffer.allocate(Integer.SIZE >> 3);
    buf.order(byteOrder);
    buf.putInt(value);

    return buf.array();
  }

  /** Convert a short integer to an array of byte using a specific endianness. */
  public static byte[] toBytes(short value, ByteOrder byteOrder) {
    ByteBuffer buf = ByteBuffer.allocate(Short.SIZE >> 3);
    buf.order(byteOrder);
    buf.putShort(value);

    return buf.array();
  }

  /** Convert a long integer to an array of byte using a specific endianness. */
  public static byte[] toBytes(long value, ByteOrder byteOrder) {
    ByteBuffer buf = ByteBuffer.allocate(Long.SIZE >> 3);
    buf.order(byteOrder);
    buf.putLong(value);

    return buf.array();
  }

  /** Convert a double to an array of byte using a specific endianness. */
  public static byte[] toBytes(double value, ByteOrder byteOrder) {
    ByteBuffer buf = ByteBuffer.allocate(Double.SIZE >> 3);
    buf.order(byteOrder);
    buf.putDouble(value);

    return buf.array();
  }

  /** Convert a short to network byte order. */
  public static int htons(int value) {
    /* network order is in big endian */
    if(ByteOrder.nativeOrder() == ByteOrder.BIG_ENDIAN)
      return value;
    else
      return reverseU16(value);
  }

  /** Convert a short from source to host byte order. */
  public static int xtohs(int value, ByteOrder valueByteOrder) {
    if(ByteOrder.nativeOrder() == valueByteOrder)
      return value;
    else
      return reverseU16(value);
  }

  private static int reverseU16(int value) {
    return ((value << 8) | (value >> 8)) & 0xffff;
  }

  /* We could probably use generics here. */
  public static void writeBytes(OutputStream out, byte value, ByteOrder byteOrder) throws IOException {
    out.write(value);
  }

  public static void writeBytes(OutputStream out, short value, ByteOrder byteOrder) throws IOException {
    out.write(Utils.toBytes(value, byteOrder));
  }

  public static void writeBytes(OutputStream out, int value, ByteOrder byteOrder) throws IOException {
    out.write(Utils.toBytes(value, byteOrder));
  }

  public static void writeBytes(OutputStream out, long value, ByteOrder byteOrder) throws IOException {
    out.write(Utils.toBytes(value, byteOrder));
  }

  public static void writeBytes(OutputStream out, double value, ByteOrder byteOrder) throws IOException {
    out.write(Utils.toBytes(value, byteOrder));
  }

  public static void writeBytes(OutputStream out, MonTimestamp nodeTime, ByteOrder byteOrder) throws IOException {
    out.write(nodeTime.toBytes(byteOrder));
  }
}

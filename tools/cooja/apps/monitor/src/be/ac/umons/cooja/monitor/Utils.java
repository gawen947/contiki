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

  public static void writeBytes(OutputStream out, double value, ByteOrder byteOrder) throws IOException {
    out.write(Utils.toBytes(value, byteOrder));
  }

  public static void writeBytes(OutputStream out, MonTimestamp nodeTime, ByteOrder byteOrder) throws IOException {
    out.write(nodeTime.toBytes(byteOrder));
  }
}

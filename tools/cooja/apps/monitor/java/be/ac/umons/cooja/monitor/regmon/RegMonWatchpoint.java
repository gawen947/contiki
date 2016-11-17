package be.ac.umons.cooja.monitor.regmon;

import be.ac.umons.cooja.monitor.mon.MonTimestamp;
import be.ac.umons.cooja.monitor.mon.backend.MonBackend;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.core.Memory.AccessMode;
import se.sics.mspsim.core.Memory.AccessType;
import se.sics.mspsim.core.MemoryMonitor;

public class RegMonWatchpoint implements MemoryMonitor {
  public static final int MONCTX = 0x1C0; /* context */
  public static final int MONENT = 0x1C2; /* entity */
  public static final int MONSTI = 0x1C4; /* state/info */
  public static final int MONCTL = 0x1C6; /* len/control */

  /* CTL register:
     9:   mode (0: event / 1: info)
     8:   record event/info
     7-0: info len
   */

  private int ctx;
  private int ent;
  private int sti;
  private int len;

  private MonBackend backend;
  private MSP430     cpu;
  
  public RegMonWatchpoint(MSP430 cpu, MonBackend backend) {
    this.backend = backend;
    this.cpu     = cpu;
  }
  
  @Override
  public void notifyWriteAfter(int addr, int data, AccessMode mode) {
    switch(addr) {
    case MONCTX:
      ctx = data;
      break;
    case MONENT:
      ent = data;
      break;
    case MONSTI:
      sti = data;
      break;
    case MONCTL:
      len = data & 0xff;

      if((data & 0x100) != 0) {
        if((data & 0x200) != 0)
          recordInfo();
        else
          recordState();
      }
    }   
  }
  
  private void recordState() {
    /* sti is state */
    backend.state(ctx, ent, sti,
                  new MonTimestamp(cpu.cycles, cpu.getTimeMillis()));
  }

  private void recordInfo() {
    byte[] info = new byte[len];

    /* sti is info ptr */
    for(int i = 0 ; i < len ; i++)
      /* cpu memory has a byte granularity.
         great for us. */
      info[i] = (byte)cpu.memory[sti + i];

    backend.info(ctx, ent, info,
                 new MonTimestamp(cpu.cycles, cpu.getTimeMillis()));
  }
  
  @Override
  public void notifyReadAfter(int addr, AccessMode mode, AccessType type) {}

  @Override
  public void notifyReadBefore(int addr, AccessMode mode, AccessType type) {}

  @Override
  public void notifyWriteBefore(int addr, int data, AccessMode mode) {}
}

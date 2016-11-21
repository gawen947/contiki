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

package be.ac.umons.cooja.monitor.device;

import org.contikios.cooja.mspmote.MspMote;
import org.contikios.cooja.Simulation;

import be.ac.umons.cooja.monitor.mon.MonTimestamp;
import be.ac.umons.cooja.monitor.mon.backend.MonBackend;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.core.Memory.AccessMode;
import se.sics.mspsim.core.Memory.AccessType;
import se.sics.mspsim.core.MemoryMonitor;

public class RegMon implements MemoryMonitor {
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

  private final MonBackend backend;
  private final MSP430     cpu;
  private final Simulation simulation;

  public RegMon(MspMote mspMote, MonBackend backend, Simulation simulation) {
    this.backend    = backend;
    this.simulation = simulation;

    cpu = mspMote.getCPU();
    cpu.addWatchPoint(MONCTX, this);
    cpu.addWatchPoint(MONENT, this);
    cpu.addWatchPoint(MONSTI, this);
    cpu.addWatchPoint(MONCTL, this);
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
                  new MonTimestamp(cpu.cycles, cpu.getTimeMillis()), simulation.getSimulationTime());
  }

  private void recordInfo() {
    byte[] info = new byte[len];

    /* sti is info ptr */
    for(int i = 0 ; i < len ; i++)
      /* cpu memory has a byte granularity.
         great for us. */
      info[i] = (byte)cpu.memory[sti + i];

    backend.info(ctx, ent, info,
                 new MonTimestamp(cpu.cycles, cpu.getTimeMillis()), simulation.getSimulationTime());
  }

  @Override
  public void notifyReadAfter(int addr, AccessMode mode, AccessType type) {}

  @Override
  public void notifyReadBefore(int addr, AccessMode mode, AccessType type) {}

  @Override
  public void notifyWriteBefore(int addr, int data, AccessMode mode) {}
}

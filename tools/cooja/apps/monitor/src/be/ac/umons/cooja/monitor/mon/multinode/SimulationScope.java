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

package be.ac.umons.cooja.monitor.mon.multinode;

import java.io.IOException;
import java.io.OutputStream;

import be.ac.umons.cooja.monitor.Utils;

/**
 * This scope is for events that happen within a simulation.
 * That is most if not all events.
 */
public class SimulationScope implements ScopeElement {
  private final long timeUs; /* simulation time in microseconds */

  /**
   * Create a simulation scope element.
   * @param timeUs Simulation time in microseconds.
   */
  public SimulationScope(long timeUs) {
    this.timeUs = timeUs;
  }

  @Override
  public void serialize(OutputStream out) throws IOException {
    Utils.writeBytes(out, timeUs, TraceFile.ENDIAN);
  }

  @Override
  public ScopeElementType getType() {
    return ScopeElementType.SIMULATION;
  }

  @Override
  public int getLength() {
    return Long.SIZE >> 3;
  }
}

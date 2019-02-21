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

import org.apache.log4j.Logger;

import be.ac.umons.cooja.monitor.mon.MonError;
import be.ac.umons.cooja.monitor.mon.MonException;
import be.ac.umons.cooja.monitor.mon.MonTimestamp;
import be.ac.umons.cooja.monitor.mon.MonStats;
import be.ac.umons.cooja.monitor.mon.switchable.SwitchableMonBackend;
import be.ac.umons.cooja.monitor.mon.switchable.SwitchableMonBackendCreator;

/**
 * Switchable monitor backends.
 * Allow online switch from one backend to another.
 */
public abstract class SwitchableMon extends MonBackend {
  private static Logger logger = Logger.getLogger(SwitchableMon.class);

  private MonTimestamp recordOffset = null;
  private MonTimestamp infoOffset   = null;
  private MonTimestamp byteOffset   = null;
  
  private final MonStats stats;

  private SwitchableMonBackend backend = null;

  /* If we select a backend before the monitor has been initialized. */
  private SwitchableMonBackendCreator delayedBackend = null;
  
  /* We can enable/disable the backend.
     Disable means we skip all new events. */
  private boolean enabled = true;

  public SwitchableMon(MonStats stats) {
    this.stats = stats;
  }

  protected void initiated() {
    recordOffset = getRecordOffset();
    infoOffset   = getInfoOffset();
    byteOffset   = getByteOffset();

    /* If backend creation was delayed (because the monitor was
     * not already initiated), we create it now. */
    if(delayedBackend != null)
      createBackend(delayedBackend);
    delayedBackend = null;
  }
  
  public void selectBackend(SwitchableMonBackendCreator backendCreator) {
    if(this.backend != null)
      /* replace a currently existing backend */
      close();

    /* If the monitor has not been initiated already,
     * we delay the creation of the backend. */
    if(isInitiated())
      createBackend(backendCreator);
    else
      delayedBackend = backendCreator;
  }

  public void recordState(int context, int entity, int state, MonTimestamp timestamp, long simTime, short nodeID) {
    try {
      if(backend != null && enabled) {
        backend.recordState(context, entity, state, timestamp, simTime, nodeID);
        stats.incStates(); /* only record non-skipped events */
      }
      else {
        /* no backend selected, skip event and tell subclass */
        skipState(context, entity, state, timestamp, simTime, nodeID);
        stats.incSkipped();
      }
    } catch(MonException e) {
      /* If something bad happened during the backend creation,
       * tell the user and acts like if no backend was selected. */
      backendError(e);
      return;
    }
  }

  public void recordInfo(int context, int entity, byte[] info, MonTimestamp timestamp, long simTime, short nodeID) {
    try {
      if(backend != null && enabled) {
        backend.recordInfo(context, entity, info, timestamp, simTime, nodeID);
        stats.incInfos(); /* only record non-skipped events */ 
      }
      else {
        /* no backend selected, skip event and tell subclass */
        skipInfo(context, entity, info, timestamp, simTime, nodeID);
        stats.incSkipped();
      }
    } catch(MonException e) {
      /* If something bad happened during the backend creation,
       * tell the user and acts like if no backend was selected. */
      backendError(e);
      return;
    }
  }

  public boolean isEnabled() {
    return enabled;
  }

  public void setEnabled(boolean enabled) {
    logger.info("(mon) backend " + (enabled ? "enabled" : "disabled"));
    this.enabled = enabled;
  }

  public void close() {
    if(this.backend != null) {
      /* tell the backend to finalize any pending operation */
      try {
        this.backend.destroy();
      } catch (MonException e) {
        /* If we cannot destroy the backend, at least we can mark it as erroneous and unselected. */
        backendError(e);
      }

      unselect();
    }
  }

  public void flush() {
    if(this.backend != null) {
      /* tell the backend to finalize any pending operation */
      try {
        this.backend.flush();
      } catch (MonException e) {
        /* If we cannot flush the backend, mark it as erroneous. */
        backendError(e);
      }
    }
  }

  private void createBackend(SwitchableMonBackendCreator backendCreator) {
    /* create the backend instance now */
    try {
      this.backend = backendCreator.create(recordOffset, infoOffset, byteOffset, getEndian());

      /* Tell subclasses about the newly created backend instance.
       * This subclass implement behaviors to react to events that
       * are triggered when no backend is currently selected. */
      initSkip(backend);
    } catch (MonException e) {
      /* If something bad happened during the backend creation,
       * tell the user and acts like if no backend was selected. */
      backendError(e);
      return;
    }
  }

  private void unselect() {
    if(this.backend != null) {
      /* tell subclass that the currently selected backend was just destroyed */
      destroySkip(this.backend);

      /* marks that no backend are currently selected */
      this.backend = null;

      logger.warn("(mon) backend unselected!");
    }
  }

  private void backendError(MonException e) {
    unselect();

    /* This is ugly! I know...
     * If we have an error while recording events,
     * we want the complete simulation to stop.
     * Otherwise we would have to throw the exception
     * upwards (up to the user interface).
     *
     * But this is still research code. So it's OK for now.
     * We better abort an erroneous simulation than risking
     * to base our observations on invalid results.
     *
     * At least I put a fixme.
     *
     * FIXME: propagate exception up to the UI. */
    throw new MonError("monitor backend error");
  }

  /* subclasses must override this to provide their own implementation for skipped events. */
  protected abstract void skipState(int context, int entity, int state, MonTimestamp timestamp, long simTime, short nodeID) throws MonException;
  protected abstract void skipInfo(int context, int entity, byte[] info, MonTimestamp timestamp, long simTime, short nodeID) throws MonException;
  protected abstract void initSkip(SwitchableMonBackend backend);
  protected abstract void destroySkip(SwitchableMonBackend backend);
}

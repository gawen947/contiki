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

import java.io.File;

import javax.swing.JFileChooser;

import org.apache.log4j.Logger;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.PluginType;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.VisPlugin;
import org.contikios.cooja.mspmote.MspMote;

import be.ac.umons.cooja.monitor.mon.backend.ErrorSkipMon;
import be.ac.umons.cooja.monitor.mon.backend.SwitchableMon;
import be.ac.umons.cooja.monitor.mon.switchable.TraceMonBackend;
import be.ac.umons.cooja.monitor.memmon.MemMon;

/* TODO: 
 *  - Use simulation.getSimulationTime() in simulation scope (instead of 0).
 *  - Check for MSPMote when starting the plugin.
 *  - Add GUI to enable/disable monitor AND select a new backend.
 */

@ClassDescription("Monitor")
@PluginType(PluginType.SIM_PLUGIN)
public class Monitor extends VisPlugin {
  private static final long serialVersionUID = 5359332460231108667L;

  private static Logger logger = Logger.getLogger(Monitor.class);
  
  private SwitchableMon backend;
  private Simulation    simulation;
  private MemMon[]       monDevices;
    
  public Monitor(Simulation simulation, final Cooja gui) {
    super("Monitor", gui, false);
    
    logger.info("Loading monitor plugin...");
    
    this.simulation = simulation;
    
    /* Ensure that we always have a backend for the output trace.
     * If an event is generated and no real backend is configured,
     * ErrorSkipMon will generate an exception. */
    backend = new ErrorSkipMon(); 
  }
  
  public void startPlugin() {
    super.startPlugin();
    
    logger.info("Starting monitor plugin...");
    
    /* Select the backend first (or at least the default file). */
    selectBackend();
    
    /* Add the MemMon device to all compatible motes. */
    monDevices = new MemMon[simulation.getMotesCount()];
    for(int i = 0 ; i < simulation.getMotesCount() ; i++) {
      /* FIXME: Isn't it dangerous ? We don't know if all nodes are MSP ones. */
      MspMote mspMote = (MspMote)simulation.getMote(i);
      monDevices[i] = new MemMon(mspMote, backend);
    }
  }
  
  public void closePlugin() {
    backend.close(); /* flush backend buffers */
  }
  
  private void selectBackend() {
    File backendFile = selectTraceFile();
    backend.selectBackend(new TraceMonBackend.Creator(backendFile));
    logger.info("Monitor backend selected '" + backendFile.getAbsolutePath() + "'");
  }
  
  private File selectTraceFile() {
    JFileChooser fileChooser = new JFileChooser();
    File suggest = new File(Cooja.getExternalToolsSetting("MONITOR_LAST", "monitor.trace"));
    fileChooser.setSelectedFile(suggest);
    fileChooser.setDialogTitle("Select monitor output trace file");
    
    int reply = fileChooser.showOpenDialog(Cooja.getTopParentContainer());
    if(reply == JFileChooser.APPROVE_OPTION) {
      File selectedFile = fileChooser.getSelectedFile();
      Cooja.setExternalToolsSetting("MONITOR_LAST", selectedFile.getAbsolutePath());
      
      return selectedFile;
    }
    else
      throw new RuntimeException("No monitor output trace file");
  }
}

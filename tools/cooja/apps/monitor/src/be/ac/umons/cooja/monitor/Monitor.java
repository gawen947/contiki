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

import java.util.Hashtable;

import java.io.File;

import java.awt.Component;
import java.awt.Dimension;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JSeparator;
import javax.swing.JFileChooser;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JButton;
import javax.swing.SwingConstants;

import org.apache.log4j.Logger;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.Mote;
import org.contikios.cooja.PluginType;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.VisPlugin;
import org.contikios.cooja.mspmote.MspMote;
import org.contikios.cooja.SimEventCentral.MoteCountListener;

import be.ac.umons.cooja.monitor.mon.backend.ErrorSkipMon;
import be.ac.umons.cooja.monitor.mon.backend.SwitchableMon;
import be.ac.umons.cooja.monitor.mon.MonStats;
import be.ac.umons.cooja.monitor.mon.switchable.TraceMonBackend;
import be.ac.umons.cooja.monitor.device.MemMon;
import be.ac.umons.cooja.monitor.device.MonDevice;

/* TODO:
 *  ! - Add GUI to enable/disable monitor AND select a new backend.
 *  ! - Add Interface to send info from backend to GUI.
 *  ! - Change warning/info messages from to Logger in monitor core classes.
 *  ! - getConfigXML();
 */

@ClassDescription("Monitor")
@PluginType(PluginType.SIM_PLUGIN)
public class Monitor extends VisPlugin {
  private static final long serialVersionUID = 5359332460231108667L;
  private static final String VERSION = "v1.2";

  private static final int GUI_SPACING = 5;

  private static Logger logger = Logger.getLogger(Monitor.class);

  private final Simulation simulation;
  private final MonStats stats;
  private final SwitchableMon backend;
  private final Hashtable<MonDevice, Integer> monDevices = new Hashtable<MonDevice, Integer>();

  private final JCheckBox enable        = new JCheckBox("Enable monitor", true);
  private final JButton   selectBackend = new JButton("Change output file...");
  private final JLabel    pluginVersion = new JLabel("<html><b>" + VERSION + "</b></html>", SwingConstants.CENTER);
  private final JLabel    numEvents     = new JLabel();
  private final JLabel    numStates     = new JLabel();
  private final JLabel    numInfos      = new JLabel();
  private final JLabel    numSkipped    = new JLabel();
  private final JLabel    numNodes      = new JLabel();

  private MoteCountListener moteCountListener;

  public Monitor(Simulation simulation, final Cooja gui) {
    super("Monitor", gui, false);

    logger.info("Loading monitor plugin...");

    this.simulation = simulation;

    /* Ensure that we always have a backend for the output trace.
     * If an event is generated and no real backend is configured,
     * ErrorSkipMon will generate an exception. */
    stats   = new MonitorGUI(numEvents, numStates, numInfos, numSkipped, numNodes);
    backend = new ErrorSkipMon(stats);

    /* Create GUI. */
    JPanel mainPane = new JPanel();
    mainPane.setLayout(new BoxLayout(mainPane, BoxLayout.PAGE_AXIS));

    mainPane.add(Box.createRigidArea(new Dimension(GUI_SPACING, 0)));
    mainPane.add(pluginVersion);
    mainPane.add(Box.createRigidArea(new Dimension(0, GUI_SPACING)));
    mainPane.add(enable);
    mainPane.add(selectBackend);
    mainPane.add(Box.createRigidArea(new Dimension(0, GUI_SPACING)));
    mainPane.add(numEvents);
    mainPane.add(numStates);
    mainPane.add(numInfos);
    mainPane.add(numSkipped);
    mainPane.add(numNodes);
    mainPane.add(Box.createRigidArea(new Dimension(0, 2*GUI_SPACING)));

    add(mainPane);
    pack();

    /* automatically add/delete motes */
    simulation.getEventCentral().addMoteCountListener(moteCountListener = new MoteCountListener() {
      public void moteWasAdded(Mote mote) {
        addMote(mote);
      }
      public void moteWasRemoved(Mote mote) {
        removeMote(mote);
      }
    });

    /* add all already existing motes */
    for(Mote m : simulation.getMotes())
      addMote(m);
  }

  public void addMote(Mote mote) {
    MspMote mspMote = (MspMote)mote;

    int nodeID = mspMote.getID();
    if(nodeID > Short.MAX_VALUE) {
      logger.error("Node ID (" + nodeID + ") too large");
      return; /* monitor can only recognize up to 65k nodes */
    }

    logger.info("Add monitor to mote " + nodeID);
    monDevices.put(new MemMon(mspMote, backend, simulation, (short)nodeID),
                   nodeID);

    stats.incNodes();
  }

  /* Remove a mote. This is different from disabling it.
     We only remove a mote when it is removed from the simulation.
     On the other hand disabling the monitor for a specific mote
     means that we ignore the events coming from that mote. */
  public void removeMote(Mote mote) {
    MspMote mspMote = (MspMote)mote;
    int nodeID = mspMote.getID();

    logger.info("Removing monitor from mote " + nodeID);
    monDevices.remove(nodeID);

    stats.decNodes();
  }

  public void startPlugin() {
    super.startPlugin();

    logger.info("Starting monitor plugin...");

    /* Select the backend first (or at least the default file). */
    selectBackend();
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
      return new File("monitor.trace");
  }
}

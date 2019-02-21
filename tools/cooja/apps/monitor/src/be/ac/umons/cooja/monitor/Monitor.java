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
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import java.io.File;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

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
import org.jdom.Element;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.Mote;
import org.contikios.cooja.PluginType;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.VisPlugin;
import org.contikios.cooja.mspmote.MspMote;
import org.contikios.cooja.SimEventCentral.MoteCountListener;

import be.ac.umons.cooja.monitor.mon.backend.IgnoreSkipMon;
import be.ac.umons.cooja.monitor.mon.backend.SwitchableMon;
import be.ac.umons.cooja.monitor.mon.MonStats;
import be.ac.umons.cooja.monitor.mon.switchable.TraceMonBackend;
import be.ac.umons.cooja.monitor.device.MemMon;
import be.ac.umons.cooja.monitor.device.MonDevice;

@ClassDescription("Monitor")
@PluginType(PluginType.SIM_PLUGIN)
public class Monitor extends VisPlugin {
  private static final long serialVersionUID = 5359332460231108667L;
  private static final String VERSION = "v1.3.25";

  private static final int GUI_SPACING = 5;

  private static Logger logger = Logger.getLogger(Monitor.class);

  private final Simulation simulation;
  private final MonStats stats;
  private final SwitchableMon backend;
  private final Hashtable<MonDevice, Integer> monDevices = new Hashtable<MonDevice, Integer>();

  private final JCheckBox guiEnable;
  private final JButton   guiSelectBackend;
  private final JLabel    guiPluginVersion;
  private final JLabel    guiNumEvents;
  private final JLabel    guiNumStates;
  private final JLabel    guiNumInfos;
  private final JLabel    guiNumSkipped;
  private final JLabel    guiNumNodes;
  private final JButton   guiFlush;

  private boolean pluginEnabled = true;

  private MoteCountListener moteCountListener;
  private File              outputFile = new File("monitor.trace");

  public Monitor(Simulation simulation, final Cooja gui) {
    super("Monitor", gui, false);

    logger.info("Loading monitor plugin...");

    this.simulation = simulation;

    if(gui.isVisualized()) {
      guiEnable        = new JCheckBox("Enable monitor", true);
      guiSelectBackend = new JButton("Change output file...");
      guiPluginVersion = new JLabel("<html><b>" + VERSION + "</b></html>", SwingConstants.CENTER);
      guiNumEvents     = new JLabel();
      guiNumStates     = new JLabel();
      guiNumInfos      = new JLabel();
      guiNumSkipped    = new JLabel();
      guiNumNodes      = new JLabel();
      guiFlush         = new JButton("Flush trace file");

      /* Ensure that we always have a backend for the output trace.
       * If an event is generated and no real backend is configured,
       * ErrorSkipMon will generate an exception. */
      stats   = new MonitorStatsGUI(guiNumEvents,
                                    guiNumStates,
                                    guiNumInfos,
                                    guiNumSkipped,
                                    guiNumNodes);

      /* Create GUI. */
      JPanel mainPane = new JPanel();
      mainPane.setLayout(new BoxLayout(mainPane, BoxLayout.PAGE_AXIS));

      mainPane.add(guiPluginVersion);
      mainPane.add(Box.createRigidArea(new Dimension(0, GUI_SPACING)));
      mainPane.add(guiEnable);
      mainPane.add(guiSelectBackend);
      mainPane.add(Box.createRigidArea(new Dimension(0, GUI_SPACING)));
      mainPane.add(guiNumEvents);
      mainPane.add(guiNumStates);
      mainPane.add(guiNumInfos);
      mainPane.add(guiNumSkipped);
      mainPane.add(guiNumNodes);
      mainPane.add(Box.createRigidArea(new Dimension(0, 2*GUI_SPACING)));

      add(mainPane);
      pack();

      guiSelectBackend.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent e) {
            outputFile = chooseOutputFile();
            setBackend(outputFile);
          }
        });
      guiEnable.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent e) {
            setPluginEnabled(!isPluginEnabled());
            backend.setEnabled(isPluginEnabled());
          }
        });
      guiFlush.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent e) {
            backend.flush();
          }
      });
    } else {
      stats = new MonitorStats();

      guiEnable        = null;
      guiSelectBackend = null;
      guiPluginVersion = null;
      guiNumEvents     = null;
      guiNumStates     = null;
      guiNumInfos      = null;
      guiNumSkipped    = null;
      guiNumNodes      = null;
      guiFlush         = null;
    }

    backend = new IgnoreSkipMon(stats);

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

    setBackend(outputFile);
  }

  public void closePlugin() {
    backend.close(); /* flush backend buffers */
  }

  /* Configure the new backend. */
  private void setBackend(File file) {
    backend.selectBackend(new TraceMonBackend.Creator(file));
    if(guiSelectBackend != null)
      guiSelectBackend.setToolTipText(outputFile.getAbsolutePath());
    logger.info("Monitor backend selected '" + outputFile.getAbsolutePath() + "'");
  }

  private File chooseOutputFile() {
    JFileChooser fileChooser = new JFileChooser();
    File suggest = new File(Cooja.getExternalToolsSetting("MONITOR_LAST", outputFile != null ? outputFile.getAbsolutePath() : "monitor.trace"));
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

  private boolean isPluginEnabled() {
    return pluginEnabled;
  }

  private void setPluginEnabled(boolean value) {
    if(guiEnable != null)
      guiEnable.setSelected(value);
    pluginEnabled = value;
  }

  @Override
  public Collection<Element> getConfigXML() {
    List<Element> config = new ArrayList<>();
    Element eOutput, eEnabled;

    eOutput  = new Element("output");
    eEnabled = new Element("enabled");

    eOutput.setText(outputFile.getAbsolutePath());
    eEnabled.setText(isPluginEnabled() ? "true" : "false");

    config.add(eOutput);
    config.add(eEnabled);

    stats.getConfigXML(config);

    return config;
  }

  @Override
  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for(Element element : configXML) {
      switch(element.getName()) {
      case "output":
        logger.info("Select backend from config...");
        outputFile = new File(element.getValue());
        /* will be configured when plugin start */
        break;
      case "enabled":
        switch(element.getValue()) {
        case "true":
          backend.setEnabled(true);
          setPluginEnabled(true);
          break;
        case "false":
          backend.setEnabled(false);
          setPluginEnabled(false);
          break;
        }
        break;
      }
    }

    stats.setConfigXML(configXML);

    return true;
  }
}

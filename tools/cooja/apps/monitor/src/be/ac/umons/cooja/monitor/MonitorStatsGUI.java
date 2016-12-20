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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import javax.swing.JLabel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import be.ac.umons.cooja.monitor.mon.MonStats;

/**
   Report the statistics on a GUI.
 */
public class MonitorStatsGUI extends MonitorStats {
  private static Logger logger = Logger.getLogger(MonitorStatsGUI.class);

  /* GUI */
  private static final String NBR_EVENTS_S  = "Total events: ";
  private static final String NBR_STATES_S  = "States events: ";
  private static final String NBR_INFOS_S   = "Infos events: ";
  private static final String NBR_SKIPPED_S = "Skipped events: ";
  private static final String NBR_NODES_S   = "Nodes: ";

  private final JLabel numEventsLabel;
  private final JLabel numStatesLabel;
  private final JLabel numInfosLabel;
  private final JLabel numSkippedLabel;
  private final JLabel numNodesLabel;

  public MonitorStatsGUI(JLabel numEvents,
                         JLabel numStates,
                         JLabel numInfos,
                         JLabel numSkipped,
                         JLabel numNodes) {
    super();

    this.numEventsLabel  = numEvents;
    this.numStatesLabel  = numStates;
    this.numInfosLabel   = numInfos;
    this.numSkippedLabel = numSkipped;
    this.numNodesLabel   = numNodes;

    this.numEventsLabel.setText(NBR_EVENTS_S + "0");
    this.numStatesLabel.setText(NBR_STATES_S + "0");
    this.numInfosLabel.setText(NBR_INFOS_S + "0");
    this.numSkippedLabel.setText(NBR_SKIPPED_S + "0");
    this.numNodesLabel.setText(NBR_NODES_S + "0");
  }

  @Override
  public void incStates() {
    super.incStates();

    numStatesLabel.setText(NBR_STATES_S + Integer.toString(numStates));
    numEventsLabel.setText(NBR_EVENTS_S + Integer.toString(numEvents));
  }

  @Override
  public void incInfos() {
    super.incInfos();

    numInfosLabel.setText(NBR_INFOS_S + Integer.toString(numInfos));
    numEventsLabel.setText(NBR_EVENTS_S + Integer.toString(numEvents));
  }

  @Override
  public void incSkipped() {
    super.incSkipped();

    numSkippedLabel.setText(NBR_SKIPPED_S + Integer.toString(numSkipped));
    numEventsLabel.setText(NBR_EVENTS_S + Integer.toString(numEvents));
  }

  @Override
  public void incNodes() {
    super.incNodes();

    numNodesLabel.setText(NBR_NODES_S + Integer.toString(numNodes));
  }

  @Override
  public void decNodes() {
    super.decNodes();

    numNodesLabel.setText(NBR_NODES_S + Integer.toString(numNodes));
  }

  @Override
  public void setConfigXML(Collection<Element> configXML) {
    super.setConfigXML(configXML);

    numStatesLabel.setText(NBR_STATES_S + Integer.toString(numStates));
    numInfosLabel.setText(NBR_INFOS_S + Integer.toString(numInfos));
    numSkippedLabel.setText(NBR_SKIPPED_S + Integer.toString(numSkipped));
    numEventsLabel.setText(NBR_EVENTS_S + Integer.toString(numEvents));
  }
}

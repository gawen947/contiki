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

import org.apache.log4j.Logger;
import org.jdom.Element;

import be.ac.umons.cooja.monitor.mon.MonStats;

/**
   Record statistics.
 */
public class MonitorStats implements MonStats {
  private static Logger logger = Logger.getLogger(MonitorStats.class);

  protected int numEvents  = 0;
  protected int numStates  = 0;
  protected int numInfos   = 0;
  protected int numSkipped = 0;
  protected int numNodes   = 0;

  @Override
  public void incStates() {
    numStates++;
    numEvents++;
  }

  @Override
  public void incInfos() {
    numInfos++;
    numEvents++;
  }

  @Override
  public void incSkipped() {
    numSkipped++;
    numEvents++;
  }

  @Override
  public void incNodes() {
    numNodes++;
  }

  @Override
  public void decNodes() {
    numNodes--;
  }

  @Override
  public void getConfigXML(List<Element> config) {
    Element eStates, eInfos, eSkipped;

    eStates  = new Element("states");
    eInfos   = new Element("infos");
    eSkipped = new Element("skipped");

    eStates.setText(Integer.toString(numStates));
    eInfos.setText(Integer.toString(numInfos));
    eSkipped.setText(Integer.toString(numSkipped));

    config.add(eStates);
    config.add(eInfos);
    config.add(eSkipped);
  }

  @Override
  public void setConfigXML(Collection<Element> configXML) {
    for(Element element : configXML) {
      switch(element.getName()) {
      case "states":
        try {
          numStates = Integer.parseInt(element.getValue());
        } catch (NumberFormatException e) {
          logger.error("invalid states element in configuration");
        }
        break;
      case "infos":
        try {
          numInfos = Integer.parseInt(element.getValue());
        } catch (NumberFormatException e) {
          logger.error("invalid infos element in configuration");
        }
        break;
      case "skipped":
        try {
          numSkipped = Integer.parseInt(element.getValue());
        } catch (NumberFormatException e) {
          logger.error("invalid skippeds element in configuration");
        }
        break;
      }
    }

    numEvents = numStates + numInfos + numSkipped;
  }
}

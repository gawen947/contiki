#!/bin/sh
#  Copyright (c) 2016, David Hauweele <david@hauweele.net>
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright notice, this
#      list of conditions and the following disclaimer.
#   2. Redistributions in binary form must reproduce the above copyright notice,
#      this list of conditions and the following disclaimer in the documentation
#      and/or other materials provided with the distribution.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
#  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Exit on error
set -e

# Shall we clean  the targets initially?
PRE_CLEAN=true

# Shall we clean the targets each time?
CLEAN_BUILD=false

# The default target. You should probably not change that.
TARGET=sky

# Path to the tools directory.
TOOLS="../../../tools/"

# Fancies BSDs peculiarities.
case "$(uname -s)" in
FreeBSD)
  GNUMAKE=gmake
  ;;
*)
  GNUMAKE=make
  ;;
esac

if [ $# != 5 ]
then
  echo "usage: $0 MIN_CYCLES STEP MAX_CYCLES STEP SEND_COUNT TIME"
  echo "  MIN_CYCLES Mininmum number of working cycles."
  echo "  STEP       Step in number of working cyclces between each run of the simulation."
  echo "  MAX_CYCLES Maximum number of working cycles."
  echo "  SEND_COUNT Number of time the probe messages should be sent on the air."
  echo "  TIME       Duration of the simulation. Must be choosen accordingly to SEND_COUNT."

  exit 1
fi

min_cycles="$1"
step="$2"
max_cycles="$3"
send_count="$4"
time="$5"

rm -f command.info
echo "Min : $min_cycles" > command.info
echo "Step: $step" >> command.info
echo "Max : $max_cycles" >> command.info
echo "Cnt : $send_count" >> command.info
echo "Time: $time" >> command.info

time=$(rpnc "$time" 1000 .)

temp_csc=$(mktemp)

clean() {
  # Makefile doesn't seems to clean correctly.
  # Don't know why so here's a little hack.
  firmware_path="$1"
  bin="$2"

  rm -rfv "$firmware_path"/obj_"$TARGET"
  rm -vf "$firmware_path"/"$bin"
}

rebuild() {
  firmware_path="$1"

  echo "Rebuild $1..."

  o_pwd=$(pwd)

  cd "$firmware_path"

  # Touch some source files that
  # depends on environment variables.
  shift
  touch $*

  $GNUMAKE TARGET=$TARGET

  echo
  cd "$o_pwd"
}

# Legend
rm -f isr2poll_stats.data control_stats.data
echo "# 0: Workload (cyc.)" > isr2poll_stats.data
echo "# 1: Nb. ISR" >> isr2poll_stats.data
echo "# 2: Nb. POLL" >> isr2poll_stats.data
echo "# 3: Average" >> isr2poll_stats.data
echo "# 4: StdDev" >> isr2poll_stats.data
echo "# 5: 5th ptile" >> isr2poll_stats.data
echo "# 6: 95th ptile" >> isr2poll_stats.data
echo "# 7: Min" >> isr2poll_stats.data
echo "# 8: Max" >> isr2poll_stats.data
cp isr2poll_stats.data control_stats.data

if $PRE_CLEAN
then
  clean balanced balanced.sky
  clean ping     ping.sky
fi

export SEND_COUNT="$send_count"
rebuild ping ping.c

# Prepare the simulation file
cat "simulation.template" | sed "s/##TIMEOUT##/$time/g" > "$temp_csc"

for cycles in $(seq "$min_cycles" "$step" "$max_cycles")
do
  # Clean old results. Ensure that we don't compute on a preceding XP.
  rm -f mspsim.trace mspsim.txt isr2poll.data isr2poll.stats control.data control.stats

  # Some info about the current run.
  echo "-------------------------"
  echo "Run $cycles / $max_cycles"
  if [ "$min_cycles" != "$max_cycles" ]
  then
    printf "Progress %3.2f%%\n" $(rpnc "$cycles" "$min_cycles" - "$max_cycles" "$min_cycles" - / 100 .)
  fi
  echo

  # Clean if needed.
  if $CLEAN_BUILD
  then
    clean balanced balanced.sky
  fi

  # Compile the balanced application with new workload.
  export CYCLES="$cycles"
  rebuild balanced balanced.c

  # Start the simulation.
  # This should generate the mspsim.trace
  echo "Starting simulation..."
  java -mx512m -jar "$TOOLS"/cooja/dist/cooja.jar -nogui="$temp_csc"
  echo

  # Parse the trace. The resulting file can be very large (~100MB)
  echo -n "Parsing resulting trace... "
  "$TOOLS"/monitor/trace mspsim.trace > mspsim.txt
  echo "done!"

  echo "Analysing trace: "

  echo -n "  Count events... "
  nb_isr=$(cat mspsim.txt | grep ISR | wc -l | tr -d ' ')
  nb_poll=$(cat mspsim.txt | grep POLL | wc -l | tr -d ' ')
  echo "done!"

  echo -n "  Generate intervals... "
  cat mspsim.txt | "$TOOLS"/monitor/scripts/isr2poll RADIO/CC2420/ISR RADIO/CC2420/OFF >   isr2poll.data
  cat mspsim.txt | "$TOOLS"/monitor/scripts/isr2poll CONTROL/TEST/0001 CONTROL/TEST/0002 > control.data
  echo "done!"

  echo -n "  Compute statistics... "
  cat isr2poll.data | "$TOOLS"/monitor/scripts/bins 4 > isr2poll.stats
  cat control.data  | "$TOOLS"/monitor/scripts/bins 4 > control.stats
  echo "done!"

  extract_stat() {
    cat "$1" | grep "^$2:" | cut -d':' -f2
  }
  echo -n "  Extract stats... "
  i2p_avg=$(extract_stat isr2poll.stats average)
  i2p_std=$(extract_stat isr2poll.stats stddev)
  i2p_5th=$(extract_stat isr2poll.stats 5th)
  i2p_95th=$(extract_stat isr2poll.stats 95th)
  i2p_min=$(extract_stat isr2poll.stats min)
  i2p_max=$(extract_stat isr2poll.stats max)

  ctr_avg=$(extract_stat control.stats average)
  ctr_std=$(extract_stat control.stats stddev)
  ctr_5th=$(extract_stat control.stats 5th)
  ctr_95th=$(extract_stat control.stats 95th)
  ctr_min=$(extract_stat control.stats min)
  ctr_max=$(extract_stat control.stats max)
  echo "done!"

  echo "$cycles $nb_isr $nb_poll $i2p_avg $i2p_std $i2p_5th $i2p_95th $i2p_min $i2p_max" >> isr2poll_stats.data
  echo "$cycles $nb_isr $nb_poll $ctr_avg $ctr_std $ctr_5th $ctr_95th $ctr_min $ctr_max" >> control_stats.data
done

rm -f "$temp_csc"
rm -f mspsim.trace mspsim.txt isr2poll.data isr2poll.stats control.data control.stats
rm -f COOJA.log COOJA.testlog

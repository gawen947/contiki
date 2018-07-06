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

# The default target. You should probably not change that.
TARGET=sky

# Path to the tools directory.
TOOLS="../../../../tools/"

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
  echo "usage: $0 TIME INITIAL_SEED NBR_RUN DRIFT_MOTE_1 DRIFT_MOTE_2..."
  echo "  TIME          Duration of the simulation (in seconds)."
  echo "  INITIAL_SEED  Initial seed to use for the simulation (the seed for the first run)."
  echo "  NBR_RUN       Number of run per seed and drift value."
  echo "  DRIFT_MOTE_1  Deviation value for mote 1."
  echo "  DRIFT_MOTE_2  Deviation value for mote 2."

  exit 1
fi

cmd_time="$1"
cmd_seed="$2"
cmd_runs="$3"
cmd_drift1="$4"
cmd_drift2="$5"

mkdir -p "results"
rm -rf "results/"*

echo "Time:    $cmd_time"   >  results/command.info
echo "Seed:    $cmd_seed"   >> results/command.info
echo "Runs:    $cmd_runs"   >> results/command.info
echo "Drift 1: $cmd_drift1" >> results/command.info
echo "Drift 2: $cmd_drift2" >> results/command.info

# Save config
cp minitiki/config.h results

results_leds="results/LEDs"
results_alpha="results/alpha.data"
results_reg="results/linear.data"
results_drifts="results/drift/"
results_sleeps="results/sleep/"
trace="$(pwd)/output.trace"
temp_csc="$(pwd)/run.csc"

clean() {
  # Makefile doesn't seems to clean correctly.
  # Don't know why so here's a little hack.
  firmware_path="$1"
  bin="$2"

  rm -rfv "$firmware_path"/*.o
  rm -rfv "$firmware_path"/*.d
  rm -vf  "$firmware_path"/"$bin"
}

rebuild() {
  firmware_path="$1"

  echo "Rebuild $1..."

  o_pwd=$(pwd)

  cd "$firmware_path"

  # Touch some source files that
  # depends on environment variables.
  shift
  if [ $# -gt 0 ]
  then
    touch $*
  fi

  #$GNUMAKE TARGET=$TARGET
  $GNUMAKE

  echo
  cd "$o_pwd"
}

clean_all() {
  clean minitiki minitiki.sky
}

do_xp_run() {
  run="$1"
  time="$2"
  seed="$3"

  time=$(rpnc "$time" 1000 .)

  # Prepare the simulation file
  cat "simulation-led.template" | \
    sed "s/##TIMEOUT##/$time/g" | \
    sed "s/##SEED##/$seed/g" | \
    sed "s/##DEVIATION_1##/$cmd_drift1/g" | \
    sed "s/##DEVIATION_2##/$cmd_drift2/g" | \
    sed "s;##MONITOR##;$trace;g" > "$temp_csc"

  cp "$temp_csc" "template-used.csc"
  echo "Starting run=$run, seed=$seed!"

  # Clean old results. Ensure that we don't compute on a preceding XP.
  rm -f "$trace"

  # Start the simulation.
  # This should generate the mspsim.trace
  echo "Starting simulation..."
  time java -mx512m -jar "$TOOLS"/cooja/dist/cooja.jar -nogui="$temp_csc" > cooja.log 2>&1
  echo

  # Count scheduled events
  case "$(uname -s)" in
    FreeBSD)
      usr_time=$(cat cooja.log | grep -Eo "[[:digit:]]+\.[[:digit:]]+ user" | sed 's/ user//g')
      sys_time=$(cat cooja.log | grep -Eo "[[:digit:]]+\.[[:digit:]]+ sys" | sed 's/ sys//g')
      real_time=$(cat cooja.log | grep -Eo "[[:digit:]]+\.[[:digit:]]+ real" | sed 's/ real//g')
      ;;
    *)
      usr_time=$(cat cooja.log | grep -Eo "[[:digit:]]+\.[[:digit:]]+user" | sed 's/user//g')
      sys_time=$(cat cooja.log | grep -Eo "[[:digit:]]+\.[[:digit:]]+system" | sed 's/system//g')
      real_time=$(cat cooja.log | grep -Eo "[[:digit:]]+\.[[:digit:]]+elapsed" | sed 's/elapsed//g')
      ;;
  esac

  rm cooja.log
  echo "RUN=$run TIME=$time $usr_time $sys_time $real_time" >> time.log

  # Parse the trace. The resulting file can be very large (~100MB)
  echo -n "Parsing resulting trace... "
  "$TOOLS"/monitor/trace "$trace" > trace.txt
  echo "done!"

  echo -n "Analysing trace... "
  cat trace.txt | python dual-led.py common-time | tee "${results_leds}_run=${i}_seed=$run_seed"

  rm trace.txt

  echo "done!"
}

prng() {
  # you wanted a linear congruential pseudo random number generator in shell?
  # now you got it!
  A=1103515245
  C=12345
  M=4294967296

  rpnc "$A" "$1" . "$C" +  "$M" mod
}

do_runs() {
  clean_all
  rebuild minitiki
  rm -f time.log cooja.log

  run_seed="$cmd_seed"
  for i in $(seq 1 "$cmd_runs")
  do
    do_xp_run "$i" "$cmd_time" "$run_seed"

    # compute new seed
    # we don't use increment because that would be too easy...
    run_seed=$(prng "$run_seed")
  done
}

average_time() {
  drift=$1

  cat time.log | grep "DRIFT=$drift" > time-avg.log
  avg_usr_time=$(cat time-avg.log | cut -d' ' -f 4 | python avg-stddev.py)
  avg_sys_time=$(cat time-avg.log | cut -d' ' -f 5 | python avg-stddev.py)
  avg_real_time=$(cat time-avg.log | cut -d' ' -f 6 | python avg-stddev.py)

  echo "$avg_usr_time $avg_sys_time $avg_real_time"
}

do_runs

rm -f "$temp_csc" "$trace" "$reg_data" trace.txt
rm -f mspsim.txt
rm -f COOJA.log COOJA.testlog

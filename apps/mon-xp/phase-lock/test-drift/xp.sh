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

if [ $# -lt 4 ]
then
  echo "usage: $0 TIME INITIAL_SEED NBR_RUN DRIFT_TO_TEST..."
  echo "  TIME          Duration of the simulation (in seconds)."
  echo "  INITIAL_SEED  Initial seed to use for the simulation (the seed for the first run)."
  echo "  NBR_RUN       Number of run per seed and drift value."
  echo "  DRIFT_TO_TEST Drift/deviation values to test in the experiment."

  exit 1
fi

cmd_time="$1"
cmd_seed="$2"
cmd_runs="$3"

shift; shift; shift;
cmd_drifts="$*"

mkdir -p "results"
rm -rf "results/"*

echo "Time:   $cmd_time"   >  results/command.info
echo "Seed:   $cmd_seed"   >> results/command.info
echo "Runs:   $cmd_runs"   >> results/command.info
echo "Drifts: $cmd_drifts" >> results/command.info
if [ -n "$TIMER" ]
then
  echo "Timer value: $TIMER" >> results/command.info
else
  echo "Timer value: default (32768)" >> results/command.info
fi


results_alpha="results/alpha.data"
results_reg="results/linear.data"
results_drifts="results/drift/"
results_sleeps="results/sleep/"
trace="$(pwd)/output.trace"
temp_csc="$(pwd)/run.csc"

# Output result keys
echo "# Time:           $cmd_time" >  "$results_reg"
echo "# Initial seed:   $cmd_seed" >> "$results_reg"
echo "# Number of runs: $cmd_runs" >> "$results_reg"
echo "# Drifts tested:  $*"        >> "$results_reg"
echo "#" >> $results_reg
echo "# DRIFT SLOP(CPU-cycles per microseconds) INTERCEPT R_VALUE P_VALUE STDERR OBSERVED/REQUESTED-drift-ratio" \
     " USR-TIME(avg) USR-TIME(stddev) SYS-TIME(avg) SYS-TIME(stddev) REAL-TIME(avg) REAL-TIME(stddev)" >> "$results_reg"
mkdir -p "$results_drifts"
mkdir -p "$results_sleeps"
echo "# drift(%) run alpha number_exec_frames" > "$results_alpha"

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

  $GNUMAKE TARGET=$TARGET

  echo
  cd "$o_pwd"
}

clean_all() {
  clean app drift.sky
}

do_xp_run() {
  run="$1"
  time="$2"
  seed="$3"
  drift="$4"

  time=$(rpnc "$time" 1000 .)

  # Prepare the simulation file
  cat "simulation.template" | \
    sed "s/##TIMEOUT##/$time/g" | \
    sed "s/##SEED##/$seed/g" | \
    sed "s/##DEVIATION##/$drift/g" | \
    sed "s;##MONITOR##;$trace;g" > "$temp_csc"

  echo "Starting run=$run, drift=$drift, seed=$seed!"

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

  # Extract ALPHA_FACTOR
  alpha_factor=$(cat cooja.log | grep "ALPHA_FACTOR" | cut -d'=' -f 2)
  exec_frame=$(cat cooja.log | grep "NUMBER_EXEC_FRAMES" | cut -d'=' -f 2)

  # Distribution of sleep (jump) periods
  cat cooja.log | grep "SLEEP_DISTRIB" > "$results_sleeps/sleep-distrib_d:${drift}_run:${run}.data"

  rm cooja.log
  echo "DRIFT=$drift RUN=$run TIME=$time $usr_time $sys_time $real_time" >> time.log

  echo "$drift $run $alpha_factor $exec_frame" >> "$results_alpha"

  # Parse the trace. The resulting file can be very large (~100MB)
  echo -n "Parsing resulting trace... "
  "$TOOLS"/monitor/trace "$trace" > trace.txt
  echo "done!"

  echo -n "Analysing trace... "

  results_drift="$results_drifts/drift_d:$drift.data"
  echo "# DRIFT $drift" > "$results_drift"
  echo "# RUN° RUN-SEED SIM-TIME CPU-CYCLES" >> "$results_drift"

  # quick-analyze display $sim_time $cpu_cycles for each line in the trace
  # with classical analyzis a XP with 1s sleep on 86400s for dev 1.0 is:
  # 414.36 real       180.39 user       492.32 sys
  # with the new quick-analyzis:
  # 9.67 real        13.32 user         0.87 sys
  cat trace.txt | quick-analyze/quick-analyze "$run $seed " >> "$results_drift"
  #cat trace.txt | grep "ENT=TEST STATE=0001" | while read line
  #do
  #  cpu_cycles=$(echo "$line" | cut -d' ' -f6 | cut -d'=' -f2 | sed 's/:.*//g')
  #  sim_time=$(echo "$line" | cut -d' ' -f3 | cut -d'=' -f2)
  #
  #  echo "$drift $run $seed $sim_time $cpu_cycles" >> "$results"
  #done
  echo "done!"
  echo
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
  rebuild app
  rm -f time.log cooja.log

  run_seed="$cmd_seed"
  for i in $(seq 1 "$cmd_runs")
  do
    for drift in $cmd_drifts
    do
      do_xp_run "$i" "$cmd_time" "$run_seed" "$drift"

      # compute new seed
      # we don't use increment because that would be too easy...
      run_seed=$(prng "$run_seed")
    done
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

echo "Now extracting measured drift:"

# Compute linear regression for each drift
reg_data="linear.data"
for drift in $cmd_drifts
do
  echo -n "Doing drift ${drift}... "
  linear_reg=$(cat "$results_drifts/drift_d:$drift.data" | grep -v "^#" | awk '{ print $3, $4 }' | python linear-reg.py | tail -n 1)
  reg_value=$(echo "$linear_reg" | cut -d' ' -f1)
  ratio=$(rpnc "$reg_value" "3.904173" / 100 .)

  avg_time=$(average_time "$drift")

  echo $drift $linear_reg $ratio $avg_time >> "$results_reg"
  echo "done!"
done

# We delete them because they take up a LOT of space (~4GB for TIMER=32 and SIM_DURATION=3600s)
rm -rf "$results_drifts"

rm -f "$temp_csc" "$trace" "$reg_data" trace.txt
rm -f mspsim.txt
rm -f COOJA.log COOJA.testlog

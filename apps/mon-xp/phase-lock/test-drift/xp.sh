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



rm -f command.info
echo "Time:   $cmd_time"   >  command.info
echo "Seed:   $cmd_seed"   >> command.info
echo "Runs:   $cmd_runs"   >> command.info
echo "Drifts: $cmd_drifts" >> command.info

results_reg="results/linear.data"
results="results/output.data"
trace="$(pwd)/output.trace"
temp_csc="$(pwd)/run.csc"

# Output result keys
echo "# Time:           $cmd_time" >  "$results"
echo "# Initial seed:   $cmd_seed" >> "$results"
echo "# Number of runs: $cmd_runs" >> "$results"
echo "# Drifts tested:  $*"        >> "$results"
echo "#" >> $results
cp "$results" "$results_reg"
echo "# DRIFT RUN° RUN-SPECIFIC-SEED SIM-TIME CPU-CYCLES" >> "$results"
echo "# DRIFT SLOP(CPU-cycles per microseconds) INTERCEPT R_VALUE P_VALUE STDERR OBSERVED/REQUESTED-drift-ratio" >> "$results_reg"

clean() {
  # Makefile doesn't seems to clean correctly.
  # Don't know why so here's a little hack.
  firmware_path="$1"
  bin="$2"

  rm -rfv "$firmware_path"/obj_"$TARGET"
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
  java -mx512m -jar "$TOOLS"/cooja/dist/cooja.jar -nogui="$temp_csc"
  echo

  # Parse the trace. The resulting file can be very large (~100MB)
  echo -n "Parsing resulting trace... "
  "$TOOLS"/monitor/trace "$trace" > trace.txt
  echo "done!"

  echo -n "Analysing trace... "
  cat trace.txt | grep "ENT=TEST STATE=0001" | while read line
  do
    cpu_cycles=$(echo "$line" | cut -d' ' -f6 | cut -d'=' -f2 | sed 's/:.*//g')
    sim_time=$(echo "$line" | cut -d' ' -f3 | cut -d'=' -f2)

    echo "$drift $run $seed $sim_time $cpu_cycles" >> "$results"
  done
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

do_runs

echo "Now extracting measured drift:"

# Compute linear regression for each drift
reg_data="linear.data"
for drift in $cmd_drifts
do
  echo -n "Doing drift ${drift}... "
  cat "$results" | awk "{ if(\$1 == $drift) print \$4,\$5}" > "$reg_data"
  linear_reg=$(python linear-reg.py "$reg_data" | tail -n 1)
  reg_value=$(echo "$linear_reg" | cut -d' ' -f1)
  ratio=$(rpnc "$reg_value" "3.904173" / 100 .)
  echo $drift $linear_reg $ratio >> "$results_reg"
  echo "done!"
done

rm -f "$temp_csc" "$trace" "$reg_data" trace.txt
rm -f mspsim.txt
rm -f COOJA.log COOJA.testlog

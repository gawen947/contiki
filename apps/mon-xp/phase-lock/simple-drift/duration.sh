TIME=300
SAMPLES=100
DEV=1.00
TIMER=32

export TIMER

echo "# Duration for:" > duration.data
echo "# dev        = $DEV%" >> duration.data
echo "# samples    = $SAMPLES runs" >> duration.data
echo "# simtime    = $TIME seconds" >> duration.data
echo "# timer      = $IMER cyc" >> duration.data
echo "# drift algo = " >> duration.data
echo "# all run with seed=1234" >> duration.data
echo "#" >> duration.data
echo "# real-time(s) ratio"

for i in $(seq 1 "$SAMPLES")
do
  # Just to be sure
  rm -f cooja.log

  # Start XP and extract real-simtime and ratio
  sh xp.sh "$TIME" 1234 1 "$DEV"
  line=$(cat cooja.log | grep "main loop stopped, system time:")
  realtime=$(echo "$line" | grep -Eo "Duration: [[:digit:]]+ ms" | grep -Eo "[[:digit:]]+")
  realtime=$(rpnc "$realtime" 1000 /)
  ratio=$(echo "$line" | grep -Eo "[0-9.]+$")

  echo "$realtime $ratio" >> duration.data
done


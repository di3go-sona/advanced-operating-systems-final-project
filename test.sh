#! /bin/bash
REPEATS=${REPEATS:-100}
echo "Testing with 4 sender/receiver threads and $REPEATS repeats"


./test send "--repeat=$REPEATS" --delay=0 --group=2  &
./test send "--repeat=$REPEATS" --delay=0 --group=2  &
./test send "--repeat=$REPEATS" --delay=0 --group=2  &
./test send "--repeat=$REPEATS" --delay=0 --group=2  &

./test recv "--repeat=$REPEATS" --delay=0 --group=2  &
./test recv "--repeat=$REPEATS" --delay=0 --group=2  &
./test recv "--repeat=$REPEATS" --delay=0 --group=2  &
./test recv "--repeat=$REPEATS" --delay=0 --group=2  &


for job in `jobs -p`
do
    wait $job 
done




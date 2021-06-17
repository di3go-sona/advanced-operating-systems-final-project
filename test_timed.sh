
./test send --repeat=100 --delay=0 --group=2  &
./test send --repeat=100 --delay=0 --group=2  &
./test send --repeat=100 --delay=0 --group=2  &
./test send --repeat=100 --delay=0 --group=2  &

./test recv --repeat=100 --delay=0 --group=2  &
./test recv --repeat=100 --delay=0 --group=2  &
./test recv --repeat=100 --delay=0 --group=2  &
./test recv --repeat=100 --delay=0 --group=2  &


for job in `jobs -p`
do
    wait $job 
done

./test send --repeat=1000 --delay=0 --group=2  &
./test send --repeat=1000 --delay=0 --group=2  &
./test send --repeat=1000 --delay=0 --group=2  &
./test send --repeat=1000 --delay=0 --group=2  &

./test recv --repeat=1000 --delay=0 --group=2  &
./test recv --repeat=1000 --delay=0 --group=2  &
./test recv --repeat=1000 --delay=0 --group=2  &
./test recv --repeat=1000 --delay=0 --group=2  &


for job in `jobs -p`
do
echo $job
    wait $job 
done
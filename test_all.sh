./test send --repeat=100 --delay=10 --group=2 &
./test recv --repeat=100 --delay=10 --group=2 &
./test set_delay 10 --repeat=10 --delay=1000 --group=2 &
./test set_delay 0 --repeat=10 --delay=1000 --group=2 &
./test revoke --repeat=10 --delay=1000 --group=2 &
./test flush --repeat=10 --delay=1000 --group=2 &


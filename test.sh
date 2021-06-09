./test send --repeat=1000 --delay=10 &
./test recv --repeat=1000 --delay=10 &
./test set_delay 10 --repeat=10 --delay=1000 &
./test revoke --repeat=10 --delay=1000 &
./test flush --repeat=10 --delay=1000 &


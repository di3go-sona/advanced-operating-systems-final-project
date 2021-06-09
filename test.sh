./test send --repeat=1000 &
./test recv --repeat=1000 &
./test set_delay 10 --repeat=1000 --delay=1000 &
./test send 0 --repeat=1000 --delay=1000 &
./test revoke --repeat=100 --delay=1000 &
./test flush --repeat=100 --delay=1000 &


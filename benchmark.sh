#! /bin/bash
REPEATS=10 time ./test.sh
REPEATS=1000 time ./test.sh
REPEATS=10000 time ./test.sh
REPEATS=50000 time ./test.sh

# Testing with 4 sender/receiver threads and 10 repeats each
# 0.01 user 
# 0.00 system 
# 0.01 elapsed

# Testing with 4 sender/receiver threads and 1000 repeats each
# 0.04 user 
# 0.40 system 
# 0.33 elapsed

# Testing with 4 sender/receiver threads and 10000 repeats each
# 0.47 user 
# 2.83 system 
# 3.00 elapsed

# Testing with 4 sender/receiver threads and 10000 repeats each
# 1.89 user 
# 18.30 system 
# 18.68 elapsed

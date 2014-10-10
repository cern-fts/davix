#!/bin/bash
#
# Generates an input file for davix-bench, containing a fixed number of line, with 2 random numbers on each line.
#

echo "Running script..."

for i in {1..100};
    do n=$(shuf -i 1-10000 -n 1); m=$(shuf -i 1-$n -n 1);
    echo "$n $m">>../build/test/bench/inputfile.txt;
done

#for i in {1..10000};
#	do n=$(( (RANDOM % 10000) +1)); m=$(( (RANDOM % n) +1));
#	echo "$n $m">>inputfile.txt;
#done

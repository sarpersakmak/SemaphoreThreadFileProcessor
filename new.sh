#!/bin/bash
rm -rf myDir
mkdir myDir
cd myDir
for i in $(seq 1 $((31+RANDOM%20))); do
    for j in $(seq 1 $((RANDOM%100000))); do
        echo $RANDOM >> file$i.txt
    done
done
cd ..
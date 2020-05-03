#!/usr/bin/env bash
#
#	Bash script to run all GMT full animations
#

echo "Loop over all full animations and run each job"

for i in movie??; do
    echo "Running full animation ${i}"
    cd $i
    bash $i.sh
    cd ..
done

echo "Completed all full animations"

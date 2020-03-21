#!/usr/bin/env bash
#
#	Bash script to run all GMT examples
#

# Set GMT_END_SHOW to off to disable automatic display of the plots
#export GMT_END_SHOW=off

echo "Loop over all examples and run each job"

# choose awk
if command -v gawk >/dev/null 2>&1 ; then
    export AWK=gawk
elif command -v nawk >/dev/null 2>&1 ; then
    export AWK=nawk
else
    export AWK=awk
fi

for i in ex*; do
    echo "Running example ${i}"
    cd $i
    bash $i.sh
    cd ..
done

echo "Completed all examples"

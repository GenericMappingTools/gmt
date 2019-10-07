#!/usr/bin/env bash
#
#	Bash script to run all GMT examples
#
echo "Loop over all examples and run each job"

# choose awk
if type gawk >/dev/null 2>&1 ; then
  export AWK=gawk
elif type nawk >/dev/null 2>&1 ; then
  export AWK=nawk
else
  export AWK=awk
fi

for n in $(seq -w 1 50); do
    echo "Running example $n"
    cd ex$n
    sh example_$n.sh
    cd ..
done

echo "Completed all examples"

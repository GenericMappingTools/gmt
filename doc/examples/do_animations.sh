#!/usr/bin/env bash
#
#	Bash script to run all GMT animations
#

echo "Loop over all animations and run each job"

# choose awk
if type gawk >/dev/null 2>&1 ; then
    export AWK=gawk
elif type nawk >/dev/null 2>&1 ; then
    export AWK=nawk
else
    export AWK=awk
fi

for i in anim??; do
    echo "Running animation ${i}"
    cd $i
    bash $i.sh animate
    cd ..
done

echo "Completed all animations"

#!/bin/sh
#	$Id$
# Testing gmtmath for the DIFF, MEAN & -S Ops

awk 'BEGIN{printf "1 10\n2 10\n3 10\n4 10\n"}' | gmtmath STDIN DIFF MEAN -S -o1 = out
echo 0 > in
diff out in --strip-trailing-cr > fail

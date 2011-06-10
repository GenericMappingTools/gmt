#!/bin/sh
#	$Id: gmtmath_op.sh,v 1.3 2011-06-10 23:15:36 jluis Exp $
# Testing gmtmath for the DIFF, MEAN & -S Ops

. ../functions.sh
header "Test gmtmath for DIFF "

awk 'BEGIN{printf "1 10\n2 10\n3 10\n4 10\n"}' | gmtmath STDIN DIFF MEAN -S -o1 = out
echo 0 > in
diff out in --strip-trailing-cr > fail

passfail gmtmath_op

rm -f in out

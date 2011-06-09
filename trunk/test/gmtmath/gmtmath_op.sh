#!/bin/sh
#	$Id: gmtmath_op.sh,v 1.2 2011-06-09 18:02:10 remko Exp $
# Testing gmtmath for the DIFF, MEAN & -S Ops

. ../functions.sh
header "Test gmtmath for DIFF "

awk 'BEGIN{printf "1 10\n2 10\n3 10\n4 10\n"}' | gmtmath STDIN DIFF MEAN -S -o1 = out
echo 0 > in
diff out in > fail

passfail gmtmath_op

#!/bin/sh
#	$Id: gmtmath_op.sh,v 1.1 2011-06-05 18:36:35 jluis Exp $
# Testing gmtmath for the DIFF, MEAN & -S Ops

. ../functions.sh
header "Test gmtmath for DIFF "

echo 1 | awk '{printf "1 10\n2 10\n3 10\n4 10\n"}' | gmtmath STDIN DIFF MEAN -S -o1 = fail 

passfail gmtmath_op

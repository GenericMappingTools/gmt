#!/bin/bash
#	$Id$

# Testing gmt math for the DIFF, MEAN & -S Ops
$AWK 'BEGIN{printf "1 10\n2 10\n3 10\n4 10\n"}' | gmt math STDIN DIFF MEAN -S -o1 = out
echo 0 > in
# Testing gmt math for the BITANDoperators
gmt math -Q 7 3 BITAND = >> out
echo 3 >> in
# Testing gmt math for the BITLEFT operators
gmt math -Q 8 3 BITLEFT = >> out
echo 64 >> in
# Testing gmt math for the BITOR operators
gmt math -Q 7 3 BITOR = >> out
echo 7 >> in
# Testing gmt math for the BITTEST operators
gmt math -Q 15 3 BITTEST = >> out
echo 1 >> in
# Testing gmt math for the BITXOR operators
gmt math -Q 7 3 BITXOR = >> out
echo 4 >> in
# Testing gmt math for the BITRIGHT operators
gmt math -Q 64 3 BITRIGHT = >> out
echo 8 >> in
diff out in --strip-trailing-cr > fail

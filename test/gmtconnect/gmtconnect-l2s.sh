#!/usr/bin/env bash
#
# Test gmtconnect longopts translation.

m=gmtconnect
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C
--l2stranstest -D
--l2stranstest -L
--l2stranstest -Q
--l2stranstest -T -T0.05+s1
EOF

# module-specific longopts
gmt $m $l2s --closed >> $b
gmt $m $l2s --dump >> $b
gmt $m $l2s --links >> $b
gmt $m $l2s --lists >> $b
gmt $m $l2s --tolerance --tolerance=0.05+second:1 >> $b

diff $a $b --strip-trailing-cr > fail

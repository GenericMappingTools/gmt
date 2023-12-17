#!/usr/bin/env bash
#
# Test fitcircle longopts translation.

m=fitcircle
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Ffmnsc -Fs
--l2stranstest -L1 -L2
--l2stranstest -L3 -L
--l2stranstest -S-23 -S
EOF

# module-specific longopts
gmt $m $l2s --coordinates=flat_mean,mean,north,south,small --coords=south >> $b
gmt $m $l2s --solution=absolutes --norm=squares >> $b
gmt $m $l2s --solution=both --solution >> $b
gmt $m $l2s --small=-23 --small_circle >> $b

diff $a $b --strip-trailing-cr > fail

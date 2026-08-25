#!/usr/bin/env bash
#
# Test gmtvector longopts translation.

m=gmtvector
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Am98 -Am
--l2stranstest -Ci -Co
--l2stranstest -E
--l2stranstest -N
--l2stranstest -S -S
--l2stranstest -Ta -Tb
--l2stranstest -Td -TD
--l2stranstest -Tp10 -Ts
--l2stranstest -Tr90 -TR
--l2stranstest -Tt20 -Tx
EOF

# module-specific longopts
gmt $m $l2s --primary_vector=mean:98 --primary_vec=mean >> $b
gmt $m $l2s --cartesian=in --cartesian=out >> $b
gmt $m $l2s --geod2geoc >> $b
gmt $m $l2s --normalize >> $b
gmt $m $l2s --secondary_vector --secondary_vec >> $b
gmt $m $l2s --transform=average --transform=bisector >> $b
gmt $m $l2s --transform=dotproduct --transform=angle >> $b
gmt $m $l2s --transform=gcircle:10 --transform=sum >> $b
gmt $m $l2s --transform=rotate:90 --transform=rotate2 >> $b
gmt $m $l2s --transform=translate:20 --transform=crossproduct >> $b

diff $a $b --strip-trailing-cr > fail

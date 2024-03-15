#!/usr/bin/env bash
#
# Test grdselect longopts translation.

m=grdselect
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Ai+il -Au+ih
--l2stranstest -Cfile
--l2stranstest -D12 -D10/2
--l2stranstest -Eb
--l2stranstest -Fmy/file+i -Fyour/file+o
--l2stranstest -G
--l2stranstest -ICDFLNRWZr
--l2stranstest -ILC
--l2stranstest -Lwhich/file
--l2stranstest -M
--l2stranstest -Nl10 -Nh100
--l2stranstest -W100/250
--l2stranstest -Z100/250
EOF

# module-specific longopts
gmt $m $l2s --area=intersection+increment:l --area=union+inc:h >> $b
gmt $m $l2s --pointfile=file >> $b
gmt $m $l2s --increment=12 --inc=10/2 >> $b
gmt $m $l2s --tabs=polygon >> $b
gmt $m $l2s --polygonfile=my/file+in --polygonfile=your/file+out >> $b
gmt $m $l2s --force_remote >> $b
gmt $m $l2s --invert=points,increment,polygons,lines,nans,region,range,zrange,registration >> $b
gmt $m $l2s --reverse=lines,points >> $b
gmt $m $l2s --linefile=which/file >> $b
gmt $m $l2s --margins >> $b
gmt $m $l2s --nans=lower:10 --nans=higher:100 >> $b
gmt $m $l2s --range=100/250 >> $b
gmt $m $l2s --zrange=100/250 >> $b

diff $a $b --strip-trailing-cr > fail

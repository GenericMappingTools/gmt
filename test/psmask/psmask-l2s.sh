#!/usr/bin/env bash
#
# Test psmask longopts translation.

m=psmask
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C
--l2stranstest -D/some/file
--l2stranstest -Fl -Fl
--l2stranstest -Gblack
--l2stranstest -I4 -I2/3 -I1/2/3
--l2stranstest -L+i -L+o
--l2stranstest -N -N
--l2stranstest -Q
--l2stranstest -S10
--l2stranstest -T
EOF

# module-specific longopts
gmt $m $l2s --endclip >> $b
gmt $m $l2s --dump=/some/file >> $b
gmt $m $l2s --forceclip=left --oriented=right >> $b
gmt $m $l2s --fill=black >> $b
gmt $m $l2s --inc=4 --increment=2/3 --spacing=1/2/3 >> $b
gmt $m $l2s --nodegrid+inside --nodegrid+outside >> $b
gmt $m $l2s --invert --inverse >> $b
gmt $m $l2s --cut >> $b
gmt $m $l2s --search_radius=10 >> $b
gmt $m $l2s --tiles >> $b

diff $a $b --strip-trailing-cr > fail

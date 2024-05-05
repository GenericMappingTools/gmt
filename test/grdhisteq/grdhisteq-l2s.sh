#!/usr/bin/env bash
#
# Test grdhisteq longopts translation.

m=grdhisteq
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C64
--l2stranstest -Dfile
--l2stranstest -Gfile.grd=nf+d2+n-99
--l2stranstest -Gother_file.grd=nf+o6+s1.5
--l2stranstest -N
--l2stranstest -Q
EOF

# module-specific longopts
gmt $m $l2s --ncells=64 >> $b
gmt $m $l2s --dump=file >> $b
gmt $m $l2s --outgrid=file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=other_file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --gaussian >> $b
gmt $m $l2s --quadratic >> $b

diff $a $b --strip-trailing-cr > fail

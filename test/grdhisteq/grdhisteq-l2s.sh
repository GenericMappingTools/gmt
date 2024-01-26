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
--l2stranstest -D/some/file
--l2stranstest -G/some/file.grd=nf+d2+n-99
--l2stranstest -G/other/file.grd=nf+o6+s1.5
--l2stranstest -N
--l2stranstest -Q
EOF

# module-specific longopts
gmt $m $l2s --ncells=64 >> $b
gmt $m $l2s --dump=/some/file >> $b
gmt $m $l2s --outgrid=/some/file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=/other/file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --gaussian >> $b
gmt $m $l2s --quadratic >> $b

diff $a $b --strip-trailing-cr > fail

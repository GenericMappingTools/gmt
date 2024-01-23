#!/usr/bin/env bash
#
# Test grdfill longopts translation.

m=grdfill
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Ac666 -Ag/my/grid.grd
--l2stranstest -An3 -As0.8
--l2stranstest -G/some/file.grd=nf+d2+n-99
--l2stranstest -G/other/file.grd=nf+o6+s1.5
--l2stranstest -Lp
--l2stranstest -N -N
EOF

# module-specific longopts
gmt $m $l2s --mode=constant:666 --mode=grid:/my/grid.grd >> $b
gmt $m $l2s --mode=neighbor:3 --mode=spline:0.8 >> $b
gmt $m $l2s --outgrid=/some/file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=/other/file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --list=polygons >> $b
gmt $m $l2s --hole_value --hole >> $b

diff $a $b --strip-trailing-cr > fail

#!/usr/bin/env bash
#
# Test triangulate longopts translation.

m=triangulate
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A
--l2stranstest -C/some/slopefile
--l2stranstest -Dx -Dy
--l2stranstest -E
--l2stranstest -G/some/file.grd=nf+d2+n-99
--l2stranstest -G/other/file.grd=nf+o6+s1.5
--l2stranstest -I5+e/10 -I2 -I1/2+n
--l2stranstest -L/my/index+b
--l2stranstest -M -M
--l2stranstest -N
--l2stranstest -Qn
--l2stranstest -S10 -S+za
--l2stranstest -T
--l2stranstest -Z
EOF

# module-specific longopts
gmt $m $l2s --area >> $b
gmt $m $l2s --slope_grid=/some/slopefile >> $b
gmt $m $l2s --derivatives=x --derivatives=y >> $b
gmt $m $l2s --empty >> $b
gmt $m $l2s --outgrid=/some/file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=/other/file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --increment=5+exact/10 --spacing=2 --inc=1/2+number >> $b
gmt $m $l2s --index=/my/index+binary >> $b
gmt $m $l2s --network --network >> $b
gmt $m $l2s --ids >> $b
gmt $m $l2s --voronoi=polygon >> $b
gmt $m $l2s --triangles=10 --triangles+zvalues:a >> $b
gmt $m $l2s --edges >> $b
gmt $m $l2s --xyz >> $b

diff $a $b --strip-trailing-cr > fail

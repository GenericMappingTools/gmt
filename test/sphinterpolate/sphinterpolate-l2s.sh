#!/usr/bin/env bash
#
# Test sphinterpolate longopts translation.

m=sphinterpolate
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -D99
--l2stranstest -G/some/file.grd=nf+d2+n-99
--l2stranstest -G/other/file.grd=nf+o6+s1.5
--l2stranstest -I5+e/10 -I2 -I1/2+n
--l2stranstest -Qg -Ql
--l2stranstest -Qp -Qs
--l2stranstest -T -T
--l2stranstest -Z
EOF

# module-specific longopts
gmt $m $l2s --skipdup=99 >> $b
gmt $m $l2s --outgrid=/some/file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=/other/file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --increment=5+exact/10 --spacing=2 --inc=1/2+number >> $b
gmt $m $l2s --tension=global --tension=local >> $b
gmt $m $l2s --tension=piecewise --tension=smooth >> $b
gmt $m $l2s --vartension --var_tension >> $b
gmt $m $l2s --scale >> $b

diff $a $b --strip-trailing-cr > fail

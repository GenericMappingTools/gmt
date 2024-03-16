#!/usr/bin/env bash
#
# Test grdsample longopts translation.

m=grdsample
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Gfile.grd=nf+d2+n-99
--l2stranstest -Gother_file.grd=nf+o6+s1.5
--l2stranstest -I12.5+e/10+n
--l2stranstest -I12.5+e -I12+n/10.2+e
--l2stranstest -T -T
EOF

# module-specific longopts
gmt $m $l2s --outgrid=file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=other_file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --increment=12.5+exact/10+number >> $b
gmt $m $l2s --inc=12.5+exact --spacing=12+number/10.2+exact >> $b
gmt $m $l2s --toggle_registration --toggle >> $b

diff $a $b --strip-trailing-cr > fail

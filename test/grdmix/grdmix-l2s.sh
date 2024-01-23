#!/usr/bin/env bash
#
# Test grdmix longopts translation.

m=grdmix
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A
--l2stranstest -C
--l2stranstest -D
--l2stranstest -G/some/file.grd=nf+d2+n-99
--l2stranstest -G/other/file.grd=nf+o6+s1.5
--l2stranstest -I
--l2stranstest -M
--l2stranstest -Ni2 -No
--l2stranstest -Q
--l2stranstest -W0.8
EOF

# module-specific longopts
gmt $m $l2s --alpha >> $b
gmt $m $l2s --construct >> $b
gmt $m $l2s --deconstruct >> $b
gmt $m $l2s --outgrid=/some/file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=/other/file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --intensity >> $b
gmt $m $l2s --monochrome >> $b
gmt $m $l2s --normalize=in:2 --normalize=out >> $b
gmt $m $l2s --opaque >> $b
gmt $m $l2s --weights=0.8 >> $b

diff $a $b --strip-trailing-cr > fail

#!/usr/bin/env bash
#
# Test grdblend longopts translation.

m=grdblend
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Cf -Cl+n
--l2stranstest -Co -Cu+p
--l2stranstest -G/My/Big/File.grd
--l2stranstest -I0.1m+e/100+n
--l2stranstest -Q -Q
--l2stranstest -W -Wz
--l2stranstest -Z
EOF

# module-specific longopts
gmt $m $l2s --clobber=first --clobber=low+negative >> $b
gmt $m $l2s --clobber=last --clobber=high+positive >> $b
gmt $m $l2s --outgrid=/My/Big/File.grd >> $b
gmt $m $l2s --increment=0.1m+exact/100+number >> $b
gmt $m $l2s --no_header --headless >> $b
gmt $m $l2s --no_blend --weights=wzsum >> $b
gmt $m $l2s --scale >> $b

diff $a $b --strip-trailing-cr > fail

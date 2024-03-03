#!/usr/bin/env bash
#
# Test grdinfo longopts translation.

m=grdinfo
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Cn -Ct
--l2stranstest -D20/10+i
--l2stranstest -Ex+l -Ex+L
--l2stranstest -Ey+u -Ey+U
--l2stranstest -F
--l2stranstest -G
--l2stranstest -I10/20 -Ib
--l2stranstest -Ii -Io
--l2stranstest -Ir
--l2stranstest -L0 -L1
--l2stranstest -L2 -Lp -La
--l2stranstest -Mc -Mf
--l2stranstest -T10+a15+s
EOF

# module-specific longopts
gmt $m $l2s --oneliner=numeric --oneliner=name_at_end >> $b
gmt $m $l2s --tiles=20/10+ignore_empty >> $b
gmt $m $l2s --extreme=x+min --extreme=x+minpos >> $b
gmt $m $l2s --extrema=y+max --extrema=y+maxneg >> $b
gmt $m $l2s --geographic >> $b
gmt $m $l2s --download >> $b
gmt $m $l2s --minmax_region=10/20 --minmax_region=polygon >> $b
gmt $m $l2s --minmax_region=imgexact --minmax_region=oblique >> $b
gmt $m $l2s --minmax_region=wesn >> $b
gmt $m $l2s --force_scan=scandata --force_scan=medianL1 >> $b
gmt $m $l2s --force_scan=meanplus --force_scan=modeLMS --force_scan=all >> $b
gmt $m $l2s --minmax_pos=conditional --minmax_pos=force >> $b
gmt $m $l2s --minmax=10+alpha:15+symmetric >> $b

diff $a $b --strip-trailing-cr > fail

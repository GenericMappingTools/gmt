#!/usr/bin/env bash
#
# Test grdtrend longopts translation.

m=grdtrend
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Dno/diff.nc
--l2stranstest -N3+r -N2+x -N2+y
--l2stranstest -Tnew/trend.nc
--l2stranstest -Wtoo/many/weights.nc+s
EOF

# module-specific longopts
gmt $m $l2s --diff=no/diff.nc >> $b
gmt $m $l2s --model=3+robust --model=2+x --model=2+y >> $b
gmt $m $l2s --trend=new/trend.nc >> $b
gmt $m $l2s --weights=too/many/weights.nc+sigma >> $b

diff $a $b --strip-trailing-cr > fail

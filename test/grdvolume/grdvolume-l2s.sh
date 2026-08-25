#!/usr/bin/env bash
#
# Test grdvolume longopts translation.

m=grdvolume
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C25 -Cr1000/5000
--l2stranstest -D
--l2stranstest -L100 -L200
--l2stranstest -Se
--l2stranstest -Tc -Th
--l2stranstest -Z20/1.5
EOF

# module-specific longopts
gmt $m $l2s --contour=25 --contour=range:1000/5000 >> $b
gmt $m $l2s --slice >> $b
gmt $m $l2s --base=100 --base_level=200 >> $b
gmt $m $l2s --unit=e >> $b
gmt $m $l2s --maximize=curvature --maximize=height >> $b
gmt $m $l2s --scale=20/1.5 >> $b

diff $a $b --strip-trailing-cr > fail

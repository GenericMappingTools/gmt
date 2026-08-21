#!/usr/bin/env bash
#
# Test gmtsplit longopts translation.

m=gmtsplit
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A1/2 -A3/4
--l2stranstest -C3
--l2stranstest -D3 -D4
--l2stranstest -F2/3
--l2stranstest -N%09d -N%06d
--l2stranstest -Qxyzdh
--l2stranstest -Qhd
--l2stranstest -S -S
EOF

# module-specific longopts
gmt $m $l2s --azimuth_tolerance=1/2 --azim_tol=3/4 >> $b
gmt $m $l2s --course_change=3 >> $b
gmt $m $l2s --min_distance=3 --min_dist=4 >> $b
gmt $m $l2s --filter=2/3 >> $b
gmt $m $l2s --multifile='%09d' --multi='%06d' >> $b
gmt $m $l2s --outputs=x,y,z,distance,heading >> $b
gmt $m $l2s --fields=hdg,dist >> $b
gmt $m $l2s --extended --dist_head >> $b

diff $a $b --strip-trailing-cr > fail

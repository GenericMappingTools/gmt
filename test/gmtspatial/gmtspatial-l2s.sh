#!/usr/bin/env bash
#
# Test gmtspatial longopts translation.

m=gmtspatial
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Aa1m -A
--l2stranstest -C
--l2stranstest -D+a+c0.02+C0.01+d1
--l2stranstest -D+f/a/file+p+s2
--l2stranstest -E+p -E+p
--l2stranstest -E+n -E+n
--l2stranstest -Fl
--l2stranstest -Ie -Ii
--l2stranstest -L2/1e-8/1e-8
--l2stranstest -N/p/file+a+i+p6
--l2stranstest -N/p/file+r+z
--l2stranstest -Qk+c1/2+h+l -Qm+p
--l2stranstest -Qm+sa
--l2stranstest -Sb2 -Sh -Sh
--l2stranstest -Si -Sj
--l2stranstest -Ss -Su
--l2stranstest -T
--l2stranstest -W5m+f -W2m+l
EOF

# module-specific longopts
gmt $m $l2s --nn=min_dist:1m --nearest_neighbor >> $b
gmt $m $l2s --clip >> $b
gmt $m $l2s --duplicates+amax+cmax:0.02+cmin:0.01+dmax:1 >> $b
gmt $m $l2s --duplicates+file:/a/file+perpendicular+factor:2 >> $b
gmt $m $l2s --handedness+positive --handedness+counterclockwise >> $b
gmt $m $l2s --handedness+negative --handedness+clockwise >> $b
gmt $m $l2s --force_polygons=lines >> $b
gmt $m $l2s --intersections=external --intersections=internal >> $b
gmt $m $l2s --no_tile_lines=2/1e-8/1e-8 >> $b
gmt $m $l2s --in_polygons=/p/file+all+individual+start:6 >> $b
gmt $m $l2s --in_polygons=/p/file+report+addcolumn >> $b
gmt $m $l2s --centroid=k+range:1/2+header+lines --area=m+close >> $b
gmt $m $l2s --length=m+sort:a >> $b
gmt $m $l2s --spatial=buffer:2 --spatial=hole --spatial=holes >> $b
gmt $m $l2s --spatial=intersection --spatial=join >> $b
gmt $m $l2s --spatial=dateline --spatial=union >> $b
gmt $m $l2s --truncate >> $b
gmt $m $l2s --extend=5m+first --extend=2m+last >> $b

diff $a $b --strip-trailing-cr > fail

#!/usr/bin/env bash
#
# Test gmtselect longopts translation.

m=gmtselect
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A10/1/2+ags+l+p20
--l2stranstest -A100+aiS+r
--l2stranstest -C/some/sortof/file+d6 -C20/10
--l2stranstest -Df -Dh -Di
--l2stranstest -Dl+f -Dc
--l2stranstest -Efn
--l2stranstest -F/some/file
--l2stranstest -G/some/mask.grd
--l2stranstest -Icfglrsz
--l2stranstest -Ilc
--l2stranstest -L/some/file+d3+p
--l2stranstest -Nk/s/k/s/k -Nk/s
--l2stranstest -Z10/20+a+c6 -Z10/20+hk+i
EOF

# module-specific longopts
gmt $m $l2s --min_area=10/1/2+antarctica:gs+lakes+percent:20 >> $b
gmt $m $l2s --area=100+antarctica:iS+river_lakes >> $b
gmt $m $l2s --distance=/some/sortof/file+distance:6 --dist2pt=20/10 >> $b
gmt $m $l2s --resolution=full --resolution=high --resolution=intermediate >> $b
gmt $m $l2s --resolution=low+lower --resolution=crude >> $b
gmt $m $l2s --boundary=fn >> $b
gmt $m $l2s --polygon=/some/file >> $b
gmt $m $l2s --gridmask=/some/mask.grd >> $b
gmt $m $l2s --reverse=circle,polygons,zero,line,rectangle,gridmask,zrange >> $b
gmt $m $l2s --invert=line,circle >> $b
gmt $m $l2s --dist2line=/some/file+distance:3+project >> $b
gmt $m $l2s --mask=k/s/k/s/k --maskvalues=k/s >> $b
gmt $m $l2s --in_range=10/20+any+column:6 --in_range=10/20+header:k+invert >> $b

diff $a $b --strip-trailing-cr > fail

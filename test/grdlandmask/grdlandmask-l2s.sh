#!/usr/bin/env bash
#
# Test grdlandmask longopts translation.

m=grdlandmask
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A1000/2/4+ag+l
--l2stranstest -A100+ai+p75
--l2stranstest -A75+r
--l2stranstest -Df+f -Dh
--l2stranstest -Di -Dl
--l2stranstest -Dc -Da
--l2stranstest -E1/2/3/4 -E6
--l2stranstest -G/some/file.grd=nf+d2+n-99
--l2stranstest -G/other/file.grd=nf+o6+s1.5
--l2stranstest -I5+e/10+n
--l2stranstest -N1/0/1/0/1
EOF

# module-specific longopts
gmt $m $l2s --min_area=1000/2/4+antarctica:g+regular_lakes >> $b
gmt $m $l2s --area=100+antarctica:i+min_polygon:75 >> $b
gmt $m $l2s --area_thresh=75+river_lakes >> $b
gmt $m $l2s --resolution=full+lower --resolution=high >> $b
gmt $m $l2s --resolution=intermediate --resolution=low >> $b
gmt $m $l2s --resolution=crude --resolution=auto >> $b
gmt $m $l2s --bordervalues=1/2/3/4 --border=6 >> $b
gmt $m $l2s --outgrid=/some/file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=/other/file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --increment=5+exact/10+number >> $b
gmt $m $l2s --maskvalues=1/0/1/0/1 >> $b

diff $a $b --strip-trailing-cr > fail

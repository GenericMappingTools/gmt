#!/usr/bin/env bash
#
# Test grdmath longopts translation.

m=grdmath
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A1000/2/4+ag+l
--l2stranstest -A100+ai+p75
--l2stranstest -A75+r
--l2stranstest -Cfile.cpt -Canother_file.cpt
--l2stranstest -Df+f -Dh
--l2stranstest -Di -Dl
--l2stranstest -Dc -Da
--l2stranstest -I5+e/10+n
--l2stranstest -M
--l2stranstest -N
--l2stranstest -S -S
EOF

# module-specific longopts
gmt $m $l2s --min_area=1000/2/4+antarctica:g+regular_lakes >> $b
gmt $m $l2s --area=100+antarctica:i+min_polygon:75 >> $b
gmt $m $l2s --area_thresh=75+river_lakes >> $b
gmt $m $l2s --cpt=file.cpt --cmap=another_file.cpt >> $b
gmt $m $l2s --resolution=full+lower --resolution=high >> $b
gmt $m $l2s --resolution=intermediate --resolution=low >> $b
gmt $m $l2s --resolution=crude --resolution=auto >> $b
gmt $m $l2s --increment=5+exact/10+number >> $b
gmt $m $l2s --flatearth >> $b
gmt $m $l2s --lax >> $b
gmt $m $l2s --single --stack_reduce >> $b

diff $a $b --strip-trailing-cr > fail

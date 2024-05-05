#!/usr/bin/env bash
#
# Test nearneighbor longopts translation.

m=nearneighbor
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -E
--l2stranstest -G/somefile=nf+d10+n-999+o6
--l2stranstest -G/otherfile+oa
--l2stranstest -G/otherfile+s100
--l2stranstest -I3m+e/100+n -I1/2 -I2+e
--l2stranstest -N12+m4 -Nn -N4+m2
--l2stranstest -S25
--l2stranstest -W
EOF

# module-specific longopts
gmt $m $l2s --empty >> $b
gmt $m $l2s --outgrid=/somefile=nf+divide:10+nan:-999+offset:6 >> $b
gmt $m $l2s --outgrid=/otherfile+offset:a >> $b
gmt $m $l2s --outgrid=/otherfile+scale:100 >> $b
gmt $m $l2s --increment=3m+exact/100+number --inc=1/2 --spacing=2+exact >> $b
gmt $m $l2s --sectors=12+min:4 --nearest=gdal --sectors=4+min_sectors:2 >> $b
gmt $m $l2s --search_radius=25 >> $b
gmt $m $l2s --weights >> $b

diff $a $b --strip-trailing-cr > fail

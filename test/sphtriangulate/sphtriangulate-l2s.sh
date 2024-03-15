#!/usr/bin/env bash
#
# Test sphtriangulate longopts translation.

m=sphtriangulate
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A
--l2stranstest -C -C
--l2stranstest -D
--l2stranstest -Ld
--l2stranstest -Nmy/nodes
--l2stranstest -Qd -Qv
--l2stranstest -T
EOF

# module-specific longopts
gmt $m $l2s --area >> $b
gmt $m $l2s --save_mem --single_form >> $b
gmt $m $l2s --skipdup >> $b
gmt $m $l2s --unit=d >> $b
gmt $m $l2s --nodes=my/nodes >> $b
gmt $m $l2s --output=delaunay --output=voronoi >> $b
gmt $m $l2s --arcs >> $b

diff $a $b --strip-trailing-cr > fail

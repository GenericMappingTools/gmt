#!/usr/bin/env bash
#
# Test sphdistance longopts translation.

m=sphdistance
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C -C
--l2stranstest -D
--l2stranstest -Ed1 -En -Ez
--l2stranstest -Gfile.grd=nf+d2+n-99
--l2stranstest -Gother_file.grd=nf+o6+s1.5
--l2stranstest -I5+e/10 -I2 -I1/2+n
--l2stranstest -Ld -Le
--l2stranstest -Nmy/nodes -Nyour/nodes
--l2stranstest -Qfile.txt
EOF

# module-specific longopts
gmt $m $l2s --save_mem --single_form >> $b
gmt $m $l2s --skipdup >> $b
gmt $m $l2s --quantity=distance:1 --quantity=polygon --quantity=zvalue >> $b
gmt $m $l2s --outgrid=file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=other_file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --increment=5+exact/10 --spacing=2 --inc=1/2+number >> $b
gmt $m $l2s --unit=d --dist_unit=e >> $b
gmt $m $l2s --nodes=my/nodes --node_table=your/nodes >> $b
gmt $m $l2s --voronoi=file.txt >> $b

diff $a $b --strip-trailing-cr > fail

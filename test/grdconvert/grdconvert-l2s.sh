#!/usr/bin/env bash
#
# Test grdconvert longopts translation.

m=grdconvert
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Cb -Cc
--l2stranstest -Cn -Cp
--l2stranstest -Gfile.grd=nf+d2+n-99
--l2stranstest -Gother_file.grd=nf+o6+s1.5
--l2stranstest -N
--l2stranstest -Z+o3 -Z+s2
EOF

# module-specific longopts
gmt $m $l2s --cmdhist=both --command_history=current >> $b
gmt $m $l2s --cmdhist=none --command_history=previous >> $b
gmt $m $l2s --outgrid=file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=other_file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --no_header >> $b
gmt $m $l2s --modify+offset:3 --scale+scale:2 >> $b

diff $a $b --strip-trailing-cr > fail

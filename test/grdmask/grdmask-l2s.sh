#!/usr/bin/env bash
#
# Test grdmask longopts translation.

m=grdmask
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Am -Ap
--l2stranstest -Ax -Ay
--l2stranstest -Cf -Cl -Co -Cu
--l2stranstest -Gfile.grd=nf+d2+n-99
--l2stranstest -Gother_file.grd=nf+o6+s1.5
--l2stranstest -I5+e/10+n
--l2stranstest -Nz1/2/NaN -NZ3/4/5
--l2stranstest -Np5 -NP100
--l2stranstest -S100/5/5 -S50/2/2
EOF

# module-specific longopts
gmt $m $l2s --straightlines=meridian --steps=parallel >> $b
gmt $m $l2s --straightlines=x --straightlines=y >> $b
gmt $m $l2s --clobber=first --clobber=low --clobber=last --clobber=high >> $b
gmt $m $l2s --outgrid=file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=other_file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --increment=5+exact/10+number >> $b
gmt $m $l2s --out_edge_in=zin:1/2/NaN --out_edge_in=zinedge:3/4/5 >> $b
gmt $m $l2s --out_edge_in=id:5 --out_edge_in=idedge:100 >> $b
gmt $m $l2s --radius=100/5/5 --search_radius=50/2/2 >> $b

diff $a $b --strip-trailing-cr > fail

#!/usr/bin/env bash
#
# Test psscale longopts translation.

m=psscale
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Cfile -Cother/file
--l2stranstest -D10/20/2/3+w200/100+eb6+h
--l2stranstest -Dg1/-0.5+v+n+r
--l2stranstest -DjBR+jLT+maclu
--l2stranstest -DJBR+o1/2
--l2stranstest -Dn0/0 -Dx5/5
--l2stranstest -F+c10+gred+i4p/black
--l2stranstest -F+p2p+r1p+s1/2/gray
--l2stranstest -G100/200
--l2stranstest -I0.2/0.6 -I -I -I
--l2stranstest -Li4 -LI -LI
--l2stranstest -M
--l2stranstest -N300 -Np
--l2stranstest -Q
--l2stranstest -S+a20+c+r+s
--l2stranstest -S+n+xmylabel+ysomeunit
--l2stranstest -W10
--l2stranstest -Zbar/widths/file -Zz/file
EOF

# module-specific longopts
gmt $m $l2s --cpt=file --cmap=other/file >> $b
gmt $m $l2s --position=10/20/2/3+size:200/100+triangles:b6+horizontal >> $b
gmt $m $l2s --position=mapcoords:1/-0.5+vertical+nan+reverse >> $b
gmt $m $l2s --position=inside:BR+anchor:LT+move_annot:aclu >> $b
gmt $m $l2s --position=outside:BR+offset:1/2 >> $b
gmt $m $l2s --position=boxcoords:0/0 --position=plotcoords:5/5 >> $b
gmt $m $l2s --box+clearance:10+fill:red+inner:4p/black >> $b
gmt $m $l2s --box+pen:2p+radius:1p+shade:1/2/gray >> $b
gmt $m $l2s --truncate=100/200 >> $b
gmt $m $l2s --illumination=0.2/0.6 --shading --shade --intensity >> $b
gmt $m $l2s --equalsize=range:4 --equal_size=intensity --equalsize=shade >> $b
gmt $m $l2s --monochrome >> $b
gmt $m $l2s --dpi=300 --dpi=discrete >> $b
gmt $m $l2s --log >> $b
gmt $m $l2s --appearance+angle:20+custom+minmax+nolines >> $b
gmt $m $l2s --appearance+numeric+barlabel:mylabel+barunit:someunit >> $b
gmt $m $l2s --scale=10 >> $b
gmt $m $l2s --barwidths=bar/widths/file --zfile=z/file >> $b

diff $a $b --strip-trailing-cr > fail

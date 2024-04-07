#!/usr/bin/env bash
#
# Test psbasemap longopts translation.

m=psbasemap
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A
--l2stranstest -D10/20/2/3+r+ui
--l2stranstest -Dg1/-0.5+w7
--l2stranstest -DjBR+jLT+sfile
--l2stranstest -DJBR+o1/2+t
--l2stranstest -Dn0/0 -Dx5/5
--l2stranstest -Fd+c0.1/0.2+gOldLace
--l2stranstest -Fl+i0.1c/3p,red,.-
--l2stranstest -Ft+p1p,yellow,4_8_5_8:2p -Ft+r3p
--l2stranstest -Ft+s2p/6p/gray
--l2stranstest -L10/20/2/3+w7+al+c10/20
--l2stranstest -Lg1/-0.5+f+jLT
--l2stranstest -L"jBR+lMy Label+u"
--l2stranstest -LJBR+o1/2+v
--l2stranstest -Ln0/0 -Lx5/5
EOF

# module-specific longopts
gmt $m $l2s --polygon >> $b
gmt $m $l2s --inset=10/20/2/3+corners+units:i >> $b
gmt $m $l2s --inset_box=mapcoords:1/-0.5+width:7 >> $b
gmt $m $l2s --inset=inside:BR+janchor:LT+outfile:file >> $b
gmt $m $l2s --inset=outside:BR+offset:1/2+translate >> $b
gmt $m $l2s --inset=boxcoords:0/0 --inset=plotcoords:5/5 >> $b
gmt $m $l2s --box=inset+clearance:0.1/0.2+fill:OldLace >> $b
gmt $m $l2s --box=scale+inner:0.1c/3p,red,.- >> $b
gmt $m $l2s --box=rose+pen:1p,yellow,4_8_5_8:2p --box=rose+radius:3p >> $b
gmt $m $l2s --box=rose+shade:2p/6p/gray >> $b
gmt $m $l2s --mapscale=10/20/2/3+length:7+align:l+loc:10/20 >> $b
gmt $m $l2s --map_scale=mapcoords:1/-0.5+fancy+janchor:LT >> $b
gmt $m $l2s --mapscale=inside:BR+label:"My Label"+units >> $b
gmt $m $l2s --mapscale=outside:BR+anchoroffset:1/2+vertical >> $b
gmt $m $l2s --mapscale=boxcoords:0/0 --mapscale=plotcoords:5/5 >> $b

diff $a $b --strip-trailing-cr > fail

#!/usr/bin/env bash
#
# Test pscoast longopts translation.

m=pscoast
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A1500/2/3+ai -A150+l+p20
--l2stranstest -A2500+r+p20
--l2stranstest -C#00ff00+l -CSkyBlue+r
--l2stranstest -Df+f -Dh -Di
--l2stranstest -Dl -Dc -Da
--l2stranstest -G#00fafa
--l2stranstest -I0/0.3p,red
--l2stranstest -Jz20 -JZ8
--l2stranstest -Lg1/-0.5+w7n -Lj6+al
--l2stranstest -LJ4/2+c3/4+f -LJ40+jLT
--l2stranstest -Ln0/0+lMyLbl -Ln10/1+o1/2
--l2stranstest -Lx5/5+u -Lx2+v
--l2stranstest -M
--l2stranstest -Q
--l2stranstest -S25-0.86-0.82
EOF

# module-specific longopts
gmt $m $l2s --area=1500/2/3+antarctica:i --area=150+lakes+percentexcl:20 >> $b
gmt $m $l2s --area=2500+riverlakes+percentexcl:20 >> $b
gmt $m $l2s --lakes=#00ff00+lakes --riverfill=SkyBlue+riverlakes >> $b
gmt $m $l2s --resolution=full+lowfallback --resolution=high --resolution=intermediate >> $b
gmt $m $l2s --resolution=low --resolution=crude --resolution=auto >> $b
#gmt $m $l2s --panel=scale+clearance:0.1/0.2+fill:OldLace --panel=scale+inner:0.1c/3p,red,.- >> $b
#gmt $m $l2s --panel=rose+pen:1p,yellow,4_8_5_8:2p --panel=rose+radius:3p >> $b
#gmt $m $l2s --panel=rose+shade:2p/6p/gray >> $b
gmt $m $l2s --land=#00fafa >> $b
gmt $m $l2s --rivers=0/0.3p,red >> $b
gmt $m $l2s --zaxis=scale:20 --zaxis=width:8 >> $b
gmt $m $l2s --mapscale=mapcoords:1/-0.5+length:7n --mapscale=inside:6+align:l >> $b
gmt $m $l2s --mapscale=outside:4/2+loc:3/4+fancy --mapscale=outside:40+janchor:LT >> $b
gmt $m $l2s --mapscale=boxcoords:0/0+label:MyLbl --mapscale=boxcoords:10/1+anchoroffset:1/2 >> $b
gmt $m $l2s --mapscale=plotcoords:5/5+units --mapscale=plotcoords:2+vertical >> $b
gmt $m $l2s --dump >> $b
#gmt $m $l2s --borders=national:3p,red --borders=state:1p,yellow >> $b
#gmt $m $l2s --borders=marine:blue --borders=all >> $b
gmt $m $l2s --markclipend >> $b
gmt $m $l2s --water=25-0.86-0.82 >> $b

diff $a $b --strip-trailing-cr > fail

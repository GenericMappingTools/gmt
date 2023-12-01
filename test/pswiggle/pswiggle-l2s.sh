#!/usr/bin/env bash
#
# Test pswiggle longopts translation.

m=pswiggle
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A30
--l2stranstest -C6
--l2stranstest -Dg-10/-10+w12 -DjRM+jCB
--l2stranstest -DJCB+al -Dn0.2/0.5+o5/10
--l2stranstest -Dx3/3+lYourLbl
--l2stranstest -Gred+n -Ggreen+p
--l2stranstest -I35
--l2stranstest -T0.4p,blue -T0.6p,black
--l2stranstest -W0.2p,yellow -W0.8p,orangeblack
--l2stranstest -Z4.9
EOF

# module-specific longopts
gmt $m $l2s --azimuth=30 >> $b
gmt $m $l2s --center=6 >> $b
gmt $m $l2s --scalebar=mapcoords:-10/-10+length:12 --scalebar=inside:RM+janchor:CB >> $b
gmt $m $l2s --scalebar=outside:CB+side:l --scalebar=boxcoords:0.2/0.5+anchoroffset:5/10 >> $b
gmt $m $l2s --scalebar=plotcoords:3/3+label:YourLbl >> $b
#gmt $m $l2s --panel+clearance:0.1/0.2+fill:OldLace --panel+inner:0.1c/3p,red,.- >> $b
#gmt $m $l2s --panel+pen:1p,yellow,4_8_5_8:2p --panel+radius:3p >> $b
#gmt $m $l2s --panel+shade:2p/6p/gray >> $b
gmt $m $l2s --fill=red+negative --fill=green+positive >> $b
gmt $m $l2s --fixedazim=35 >> $b
gmt $m $l2s --trackpen=0.4p,blue --track=0.6p,black >> $b
gmt $m $l2s --outlinepen=0.2p,yellow --pen=0.8p,orangeblack >> $b
gmt $m $l2s --ampscale=4.9 >> $b

diff $a $b --strip-trailing-cr > fail

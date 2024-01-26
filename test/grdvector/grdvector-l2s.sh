#!/usr/bin/env bash
#
# Test grdvector longopts translation.

m=grdvector
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A
--l2stranstest -C/some/file.cpt -C/some/file.cpt
--l2stranstest -Gblue
--l2stranstest -I20/30 -Ix3/4 -I4/5
--l2stranstest -N
--l2stranstest -Q+a20+bA+c -Q+c+e+gred
--l2stranstest -Q+h1.5+je -Q+mfc+n3/1
--l2stranstest -Q+o0/89+p2p -Q+q+s+t1/2
--l2stranstest -Q+vi7 -Q+z2
--l2stranstest -Si0.8+c10/20
--l2stranstest -Sl6+s2
--l2stranstest -T
--l2stranstest -W2p,red+c
--l2stranstest -Z
EOF

# module-specific longopts
gmt $m $l2s --polar >> $b
gmt $m $l2s --cpt=/some/file.cpt --cmap=/some/file.cpt >> $b
gmt $m $l2s --fill=blue >> $b
gmt $m $l2s --increment=20/30 --inc=multiples:3/4 --spacing=4/5 >> $b
gmt $m $l2s --noclip >> $b
gmt $m $l2s --vector+apex:20+begin:A+cpt --vector+cmap+end+fill:red >> $b
gmt $m $l2s --vector+shape:1.5+justify:e --vector+midpoint:fc+norm:3/1 >> $b
gmt $m $l2s --vector+oblique:0/89+pen:2p --vector+angles+xycoords+trim:1/2 >> $b
gmt $m $l2s --vector+polar_scale:i7 --vector+polar_convert:2 >> $b
gmt $m $l2s --vec_scale=invert:0.8+location:10/20 >> $b
gmt $m $l2s --vec_scale=length:6+size:2 >> $b
gmt $m $l2s --sign_scale >> $b
gmt $m $l2s --pen=2p,red+color >> $b
gmt $m $l2s --azimuth >> $b

diff $a $b --strip-trailing-cr > fail

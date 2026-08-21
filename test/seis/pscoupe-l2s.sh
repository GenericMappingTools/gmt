#!/usr/bin/env bash
#
# Test pscoupe longopts translation.

m=pscoupe
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Aa5/10/20/25+cn+d3
--l2stranstest -Ab5/10/6/100+r12+w6
--l2stranstest -Ac1/2/3/4+z6
--l2stranstest -Ad1/2/30/1000+f
--l2stranstest -Sa1000+a10+fHelvetica+jTR+l+o2/3
--l2stranstest -Sc2000+s2.5
--l2stranstest -Sm -Sd -Sz
--l2stranstest -Sp+m
--l2stranstest -Sx -Sy -St
--l2stranstest -Cmycpt
--l2stranstest -Fsc2 -Fa2/hi
--l2stranstest -Fered -Fggreen
--l2stranstest -Fp3p -Ft1p
--l2stranstest -Frorange
--l2stranstest -H -H5
--l2stranstest -I0.5
--l2stranstest -L -L3p,red
--l2stranstest -N
--l2stranstest -Q
--l2stranstest -T0/green
--l2stranstest -W1p,blue,solid
EOF

# module-specific longopts
gmt $m $l2s --crosssection=geopoints:5/10/20/25+region:n+dip:3 >> $b
gmt $m $l2s --crosssection=geostrikelen:5/10/6/100+domain:12+width:6 >> $b
gmt $m $l2s --crosssection=xypoints:1/2/3/4+depth:6 >> $b
gmt $m $l2s --crosssection=xystrikelen:1/2/30/1000+frame >> $b
gmt $m $l2s --format=aki:1000+angle:10+font:Helvetica+justify:TR+moment+offset:2/3 >> $b
gmt $m $l2s --format=cmt:2000+mreference:2.5 >> $b
gmt $m $l2s --format=smtfull --format=smtdouble --format=smtdev >> $b
gmt $m $l2s --format=partial+samesize >> $b
gmt $m $l2s --format=axisfull --format=axisdouble --format=axisdev >> $b
gmt $m $l2s --cpt=mycpt >> $b
gmt $m $l2s --mode=symbol:c2 --mode=ptaxes:2/hi >> $b
gmt $m $l2s --mode=taxisfill:red --mode=paxisfill:green >> $b
gmt $m $l2s --mode=paxispen:3p --mode=taxispen:1p >> $b
gmt $m $l2s --mode=box:orange >> $b
gmt $m $l2s --scale --scale=5 >> $b
gmt $m $l2s --intensity=0.5 >> $b
gmt $m $l2s --outlinepen --outlinepen=3p,red >> $b
gmt $m $l2s --noclip >> $b
gmt $m $l2s --noinfofiles >> $b
gmt $m $l2s --nodal=0/green >> $b
gmt $m $l2s --pen=1p,blue,solid >> $b

diff $a $b --strip-trailing-cr > fail

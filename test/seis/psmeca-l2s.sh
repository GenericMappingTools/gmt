#!/usr/bin/env bash
#
# Test psmeca longopts translation.

m=psmeca
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Sa2+a20+fHelvetica
--l2stranstest -Sc+jLT
--l2stranstest -Sm6+l
--l2stranstest -Sd+m
--l2stranstest -Sz+o1/2
--l2stranstest -Sp+s5
--l2stranstest -Sx2+jLT
--l2stranstest -Sy2+jLT
--l2stranstest -St2+jLT
--l2stranstest -A+pfaint+s6
--l2stranstest -Csome.cpt
--l2stranstest -D20/100
--l2stranstest -Fa12/dh -Fered
--l2stranstest -Fgblue -Fo
--l2stranstest -Fpfaint -Frred
--l2stranstest -Ftthin,red -Fz0.1c
--l2stranstest -H5
--l2stranstest -I0.5
--l2stranstest -L2p,green
--l2stranstest -N
--l2stranstest -T0/1p
--l2stranstest -W0.1c,orange
EOF

# module-specific longopts
gmt $m $l2s --format=aki:2+angle:20+font:Helvetica >> $b
gmt $m $l2s --format=cmt+justify:LT >> $b
gmt $m $l2s --format=smtfull:6+moment >> $b
gmt $m $l2s --format=smtdouble+samesize >> $b
gmt $m $l2s --format=smtdev+offset:1/2 >> $b
gmt $m $l2s --format=partial+mreference:5 >> $b
gmt $m $l2s --format=axisfull:2+justify:LT >> $b
gmt $m $l2s --format=axisdouble:2+justify:LT >> $b
gmt $m $l2s --format=axisdev:2+justify:LT >> $b
gmt $m $l2s --focaloffset+pen:faint+size:6 >> $b
gmt $m $l2s --cpt=some.cpt >> $b
gmt $m $l2s --depth=20/100 >> $b
gmt $m $l2s --mode=ptaxes:12/dh --mode=taxisfill:red >> $b
gmt $m $l2s --mode=paxisfill:blue --mode=nodepth >> $b
gmt $m $l2s --mode=paxispen:faint --mode=box:red >> $b
gmt $m $l2s --mode=taxispen:thin,red --mode=overlaypen:0.1c >> $b
gmt $m $l2s --scale=5 >> $b
gmt $m $l2s --intensity=0.5 >> $b
gmt $m $l2s --outlinepen=2p,green >> $b
gmt $m $l2s --noclip >> $b
gmt $m $l2s --nodal=0/1p >> $b
gmt $m $l2s --pen=0.1c,orange >> $b

diff $a $b --strip-trailing-cr > fail

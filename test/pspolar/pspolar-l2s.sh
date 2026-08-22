#!/usr/bin/env bash
#
# Test pspolar longopts translation.

m=pspolar
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -D10/20
--l2stranstest -M0.5i -M1c+m10
--l2stranstest -Sa1 -Sc2 -Sd3
--l2stranstest -Sh4 -Si5 -Sp6
--l2stranstest -Ss7 -St8 -Sx9
--l2stranstest -N
--l2stranstest -Qered -Qfgreen -Qgblue
--l2stranstest -Qhred -Qsgreen -Qtblue
--l2stranstest -T+a5+fHelvetica+jTL+o1/2
--l2stranstest -W1p,blue,solid
EOF

# module-specific longopts
gmt $m $l2s --center=10/20 >> $b
gmt $m $l2s --size=0.5i --size=1c+magnitude:10 >> $b
gmt $m $l2s --symbol=star:1 --symbol=circle:2 --symbol=diamond:3 >> $b
gmt $m $l2s --symbol=hexagon:4 --symbol=invtriangle:5 --symbol=point:6 >> $b
gmt $m $l2s --symbol=square:7 --symbol=triangle:8 --symbol=cross:9 >> $b
gmt $m $l2s --noclip >> $b
gmt $m $l2s --mode=extensive:red --mode=focal:green --mode=compressional:blue >> $b
gmt $m $l2s --mode=hypo71:red --mode=spolarity:green --mode=station:blue >> $b
gmt $m $l2s --station+angle:5+font:Helvetica+justify:TL+offset:1/2 >> $b
gmt $m $l2s --pen=1p,blue,solid >> $b

diff $a $b --strip-trailing-cr > fail

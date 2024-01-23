#!/usr/bin/env bash
#
# Test grd2cpt longopts translation.

m=grd2cpt
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A50+a
--l2stranstest -C/some/file.cpt -C/some/file.cpt
--l2stranstest -Di -Do
--l2stranstest -E4+c+f
--l2stranstest -Fc -Fg -Fh+kD
--l2stranstest -Fr+cmylbl -FR -Fx
--l2stranstest -G5/10
--l2stranstest -H
--l2stranstest -Icz
--l2stranstest -L -L
--l2stranstest -M -M
--l2stranstest -N
--l2stranstest -Qi -Qo
--l2stranstest -Sl -Su
--l2stranstest -Sm -Sh
--l2stranstest -T0/10/1 -T2 -T3
--l2stranstest -Ww
--l2stranstest -Z
EOF

# module-specific longopts
gmt $m $l2s --alpha=50+all >> $b
gmt $m $l2s --cpt=/some/file.cpt --cmap=/some/file.cpt >> $b
gmt $m $l2s --bg=in --background=out >> $b
gmt $m $l2s --nlevels=4+cumulative+file >> $b
gmt $m $l2s --format=cmyk --color_model=gray --format=hsv+key:D >> $b
gmt $m $l2s --format=rgb+categorical:mylbl --format=name --format=hex >> $b
gmt $m $l2s --truncate=5/10 >> $b
gmt $m $l2s --savecpt >> $b
gmt $m $l2s --reverse=colors,zvalues >> $b
gmt $m $l2s --datarange --limit >> $b
gmt $m $l2s --overrule --overrule_bg >> $b
gmt $m $l2s --no_bg >> $b
gmt $m $l2s --log=in --log=out >> $b
gmt $m $l2s --symmetric=absmin --symmetric=absmax >> $b
gmt $m $l2s --symmetric=minabsrange --symmetric=maxabsrange >> $b
gmt $m $l2s --steps=0/10/1 --series=2 --range=3 >> $b
gmt $m $l2s --categorical=wrap >> $b
gmt $m $l2s --continuous >> $b

diff $a $b --strip-trailing-cr > fail

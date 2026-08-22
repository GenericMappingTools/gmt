#!/usr/bin/env bash
#
# Test makecpt longopts translation.

m=makecpt
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A50+a
--l2stranstest -Cfile.cpt -Cfile.cpt
--l2stranstest -Di -Do
--l2stranstest -E4
--l2stranstest -Fc -Fg -Fh+kD
--l2stranstest -Fr+cmylbl -FR -Fx
--l2stranstest -G5/10
--l2stranstest -H
--l2stranstest -Icz
--l2stranstest -M -M
--l2stranstest -N
--l2stranstest -Q
--l2stranstest -Sa -Sm -Sp
--l2stranstest -Sp -Sq -Sr
--l2stranstest -S20+d
--l2stranstest -T0/10/1+b -T20/30/5+l
--l2stranstest -T300/1000/100+i -T0/10/1+n
--l2stranstest -Ww
--l2stranstest -Z
EOF

# module-specific longopts
gmt $m $l2s --alpha=50+all >> $b
gmt $m $l2s --cpt=file.cpt --cmap=file.cpt >> $b
gmt $m $l2s --bg=in --background=out >> $b
gmt $m $l2s --nlevels=4 >> $b
gmt $m $l2s --format=cmyk --color_model=gray --format=hsv+key:D >> $b
gmt $m $l2s --format=rgb+categorical:mylbl --format=name --format=hex >> $b
gmt $m $l2s --truncate=5/10 >> $b
gmt $m $l2s --savecpt >> $b
gmt $m $l2s --reverse=colors,zvalues >> $b
gmt $m $l2s --overrule --overrule_bg >> $b
gmt $m $l2s --no_bg >> $b
gmt $m $l2s --log >> $b
gmt $m $l2s --range_mode=average --auto=median --range_mode=lms >> $b
gmt $m $l2s --range_mode=mode --range_mode=quartiles --range_mode=minmax >> $b
gmt $m $l2s --range_mode=20+discrete >> $b
gmt $m $l2s --steps=0/10/1+log2 --series=20/30/5+log10 >> $b
gmt $m $l2s --range=300/1000/100+inverse --steps=0/10/1+number >> $b
gmt $m $l2s --categorical=wrap >> $b
gmt $m $l2s --continuous >> $b

diff $a $b --strip-trailing-cr > fail

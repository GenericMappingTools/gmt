#!/usr/bin/env bash
#
# Test blockmedian longopts translation.

m=blockmedian
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Azslq25q75hw
--l2stranstest -C
--l2stranstest -E -Eb
--l2stranstest -Er+l -Es+h
--l2stranstest -G/some/file.grd
--l2stranstest -Q
--l2stranstest -T0.8
--l2stranstest -Wi -Wo+s
EOF

# module-specific longopts
gmt $m $l2s --fields=zslq25q75hw >> $b
gmt $m $l2s --center >> $b
gmt $m $l2s --extend --extend=boxwhisker >> $b
gmt $m $l2s --extend=record+lower --extend=source+higher >> $b
gmt $m $l2s --outgrid=/some/file.grd >> $b
gmt $m $l2s --quick >> $b
gmt $m $l2s --quantile=0.8 >> $b
gmt $m $l2s --weights=in --weights=out+sigma >> $b

diff $a $b --strip-trailing-cr > fail

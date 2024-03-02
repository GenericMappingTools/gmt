#!/usr/bin/env bash
#
# Test trend2d longopts translation.

m=trend2d
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C1.2e03 -C2
--l2stranstest -Fxymrw
--l2stranstest -Fp -Fp
--l2stranstest -I0.5 -I0.8
--l2stranstest -N4+r -N5
--l2stranstest -W+s -W+w
EOF

# module-specific longopts
gmt $m $l2s --condition=1.2e03 --condition_number=2 >> $b
gmt $m $l2s --output=x,y,model,residual,weight >> $b
gmt $m $l2s --output=params --output=parameters >> $b
gmt $m $l2s --confidence=0.5 --conf_level=0.8 >> $b
gmt $m $l2s --nterms=4+robust --model=5 >> $b
gmt $m $l2s --weights+uncertainties --weights+weights >> $b

diff $a $b --strip-trailing-cr > fail

#!/usr/bin/env bash
#
# Test trend1d longopts translation.

m=trend1d
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C1.2e03 -C2
--l2stranstest -Fxymrw -Fp
--l2stranstest -FP -Fc
--l2stranstest -I0.5 -I0.8
--l2stranstest -Np5,f3,C6+l1+o2+r
--l2stranstest -W+s
EOF

# module-specific longopts
gmt $m $l2s --condition=1.2e03 --condition_number=2 >> $b
gmt $m $l2s --output=x,y,model,residual,weight --output=polynomial >> $b
gmt $m $l2s --output=npolynomial --output=chebyshev >> $b
gmt $m $l2s --confidence=0.5 --conf_level=0.8 >> $b
gmt $m $l2s --model=p5,f3,C6+length:1+origin:2+robust >> $b
gmt $m $l2s --weights+uncertainties >> $b

diff $a $b --strip-trailing-cr > fail

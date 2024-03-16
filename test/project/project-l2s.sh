#!/usr/bin/env bash
#
# Test project longopts translation.

m=project
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A25
--l2stranstest -C1/2 -C3/4
--l2stranstest -E5/6
--l2stranstest -Fpqrs -Frsxy -Fzpq
--l2stranstest -G5k+c -G20/2+h -G100+n
--l2stranstest -L100/1000 -Lw
--l2stranstest -N
--l2stranstest -Q
--l2stranstest -S
--l2stranstest -T180/89
--l2stranstest -W20/30
--l2stranstest -Z100k/70/0+e
EOF

# module-specific longopts
gmt $m $l2s --azimuth=25 >> $b
gmt $m $l2s --center=1/2 --origin=3/4 >> $b
gmt $m $l2s --endpoint=5/6 >> $b
gmt $m $l2s --outformat=pqrs --outvars=rsxy --convention=zpq >> $b
gmt $m $l2s --step=5k+colat --generate=20/2+header --step=100+npoints >> $b
gmt $m $l2s --length=100/1000 --length=limit >> $b
gmt $m $l2s --flat_earth >> $b
gmt $m $l2s --km >> $b
gmt $m $l2s --sort >> $b
gmt $m $l2s --pole=180/89 >> $b
gmt $m $l2s --width=20/30 >> $b
gmt $m $l2s --ellipse=100k/70/0+equal >> $b

diff $a $b --strip-trailing-cr > fail

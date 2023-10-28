#!/usr/bin/env bash
#
# Test psclip longopts translation.

m=psclip
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Am -Ap
--l2stranstest -Ax -Ay
--l2stranstest -Ar -At
--l2stranstest -C -C5
--l2stranstest -N
--l2stranstest -T
--l2stranstest -W0.1c,120-1-1
EOF

# module-specific longopts
gmt $m $l2s --straightlines=meridian --steps=parallel >> $b
gmt $m $l2s --straightlines=x --straightlines=y >> $b
gmt $m $l2s --straightlines=r --straightlines=theta >> $b
gmt $m $l2s --endclip --endclip=5 >> $b
gmt $m $l2s --invert >> $b
gmt $m $l2s --clipregion >> $b
gmt $m $l2s --pen=0.1c,120-1-1 >> $b

diff $a $b --strip-trailing-cr > fail

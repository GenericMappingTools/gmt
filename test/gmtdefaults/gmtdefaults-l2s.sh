#!/usr/bin/env bash
#
# Test gmtdefaults longopts translation.

m=gmtdefaults
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -D -Du -Du
--l2stranstest -Ds -Ds
EOF

# module-specific longopts
gmt $m $l2s --defaults --defaults=us --defaults=US >> $b
gmt $m $l2s --defaults=si --defaults=SI >> $b

diff $a $b --strip-trailing-cr > fail

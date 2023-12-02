#!/usr/bin/env bash
#
# Test gmtsimplify longopts translation.

m=gmtsimplify
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -T
EOF

# module-specific longopts
gmt $m $l2s --tolerance >> $b

diff $a $b --strip-trailing-cr > fail

#!/usr/bin/env bash
#
# Test begin longopts translation.

m=begin
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C
EOF

# module-specific longopts
gmt $m $l2s --clean >> $b

diff $a $b --strip-trailing-cr > fail

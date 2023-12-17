#!/usr/bin/env bash
#
# Test gmtwhich longopts translation.

m=gmtwhich
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A
--l2stranstest -C
--l2stranstest -D -D
--l2stranstest -Ga -Gc
--l2stranstest -Gl -Gu
EOF

# module-specific longopts
gmt $m $l2s --readable >> $b
gmt $m $l2s --confirm >> $b
gmt $m $l2s --directories --report_dir >> $b
gmt $m $l2s --download=user --download=cache >> $b
gmt $m $l2s --download=current --download=data >> $b

diff $a $b --strip-trailing-cr > fail

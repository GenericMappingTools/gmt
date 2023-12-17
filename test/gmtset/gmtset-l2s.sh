#!/usr/bin/env bash
#
# Test gmtset longopts translation.

m=gmtset
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C
--l2stranstest -Du -Ds
--l2stranstest -Du -Ds
--l2stranstest -G/some/file -G/other/file
EOF

# module-specific longopts
gmt $m $l2s --convert >> $b
gmt $m $l2s --defaults=us --defaults=si >> $b
gmt $m $l2s --defaults=US --defaults=SI >> $b
gmt $m $l2s --gmtconf=/some/file --defaultsfile=/other/file >> $b

diff $a $b --strip-trailing-cr > fail

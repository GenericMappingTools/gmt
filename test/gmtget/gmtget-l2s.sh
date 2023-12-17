#!/usr/bin/env bash
#
# Test gmtget longopts translation.

m=gmtget
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Ddata=earth -Dcache
--l2stranstest -G/some/gmt.conf
--l2stranstest -I1m
--l2stranstest -L
--l2stranstest -N
--l2stranstest -Q -Q
EOF

# module-specific longopts
gmt $m $l2s --data=data=earth --dataset=cache >> $b
gmt $m $l2s --gmtconf=/some/gmt.conf >> $b
gmt $m $l2s --increment=1m >> $b
gmt $m $l2s --lines >> $b
gmt $m $l2s --no_convert >> $b
gmt $m $l2s --list --no_download >> $b

diff $a $b --strip-trailing-cr > fail
